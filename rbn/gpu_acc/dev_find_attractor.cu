// Compiled with CUDA C++

#include "dev_find_attractor.hpp"

#include <iostream>
#include <thrust/functional.h>
#include <thrust/logical.h>
#include <thrust/fill.h>
#include <thrust/device_vector.h>
#include <thrust/swap.h>

#include "boolean_functions.hpp"

namespace gpu_acc {

namespace {

struct dev_rm {
	~dev_rm() {
		cudaDeviceReset();
	}
};

dev_rm& dev_init() {
	static dev_rm drm;
	return drm;
}


__device__ bool dev_eq_to_ref;

__global__
void update_state(int nodes_count, const int* xs, int* ys, node_behavior* behavior,
					boolean_functions fs, int* ref)//, bool* eq_to_ref)
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	if(idx < nodes_count) {
		int x = xs[idx];
		int y = fs(idx, xs);
		behavior[idx].changes += (x != y);
		behavior[idx].sum += y;
		ys[idx] = y;
		if(y != ref[idx]) {
			dev_eq_to_ref = false;
		}
	}
}
/*
__device__ unsigned int threads_working;

__device__
void sync_system() {
	while(threads_working != 0);
}

__global__
void find_attractor_kernel(int nodes_count, const int* xs, int* ys, node_behavior* behavior,
							boolean_functions fs, int* reference_state) {
	threads_working = 0;
	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	if(idx == 0) {
		//
	}
	sync_system();

	for(int i = 0; i < 100000; ++i) {
		update_state(idx, nodes_count, xs, ys, behavior[idx], fs, reference_state[idx]);
		sync_system();
	}
}*/

} // namespace

int dev_find_attractor(network& net) {
	dev_init();
	unsigned int T[] = {100, 1000, 10000, 1000000};
	const int max = sizeof(T) / sizeof(unsigned int) - 1;
	unsigned int i, k;
	size_t nodes_count = net.state().size();
	size_t threads_count = 32;
	size_t blocks_count = nodes_count / threads_count + 1;
	if(nodes_count % threads_count == 0) {
		--blocks_count;
	}
	
	thrust::device_vector<int> state0 = net.state(), xs = net.state(), ys = net.state();
	thrust::device_vector<node_behavior> dev_behavior = net.behavior();
	thrust::device_vector<int> functions_storage = net.functions();
	boolean_functions bfs(net.function_size(), net.arguments_size(), functions_storage);
	
	bool host_eq_to_ref;
	for(i = 1, k = 0; i < T[max]; ++i){
		int* xs_ptr = thrust::raw_pointer_cast(xs.data());
		int* ys_ptr = thrust::raw_pointer_cast(ys.data());
		node_behavior* b_ptr = thrust::raw_pointer_cast(dev_behavior.data());
		int* ref_ptr = thrust::raw_pointer_cast(state0.data());
		//int* ref_eq_ptr = thrust::raw_pointer_cast(equal_to_reference_state.data());
		
		host_eq_to_ref = false;
		cudaMemcpyToSymbol("dev_eq_to_ref", &host_eq_to_ref, sizeof(bool), 0, cudaMemcpyHostToDevice);
		//cudaMemset(dev_eq_to_ref, true, sizeof(bool));

		update_state<<<blocks_count, threads_count>>> (
			nodes_count,
			xs_ptr,
			ys_ptr,
			b_ptr,
			bfs,
			ref_ptr//, // reference state
			//dev_eq_to_ref
		);

		xs.swap(ys);
	
		cudaMemcpyFromSymbol(&host_eq_to_ref, "dev_eq_to_ref", sizeof(bool), 0, cudaMemcpyDeviceToHost);

		/*if(host_eq_to_ref) {
			break;
		}

		if(i == T[k]){
			++k;
			//cout << "ts";
			state0 = xs;
			thrust::fill(dev_behavior.begin(), dev_behavior.end(), node_behavior());
		}*/
	}
	
	thrust::host_vector<int> state1 = xs;
	net.state() = std::vector<int>(state1.begin(), state1.end());
	
	thrust::host_vector<node_behavior> behavior = dev_behavior;
	net.behavior() = std::vector<node_behavior>(behavior.begin(), behavior.end());
	
	if(i == T[max]){
		std::cout << "koniec";// << std::endl;
		return i - T[max - 1];
	}
	else if(k > 0) {
		return i - T[k-1];
	}
	else return i;
}
	
} // namespace gpu_acc
