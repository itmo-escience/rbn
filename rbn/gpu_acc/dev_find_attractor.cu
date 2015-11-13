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

__global__
void update_state(int nodes_count, const int* xs, int* ys, node_behavior* behavior, boolean_functions fs, int* reference_state, int* eq_to_ref) {
	int i = blockIdx.x * blockDim.x + threadIdx.x;
	if(i < nodes_count) {
		int x = xs[i];
		int y = fs(i, xs);
		behavior[i].changes += (x != y);
		behavior[i].sum += y;
		ys[i] = y;
		eq_to_ref[i] = (y == reference_state[i]);
	}
}

} // namespace

struct dev_rm {
	~dev_rm() {
		cudaDeviceReset();
	}
};

dev_rm& dev_init() {
	static dev_rm drm;
	return drm;
}

int dev_find_attractor(network& net) {
	dev_init();
	unsigned int T[] = {100, 1000, 10000, 100000};
	const int max = sizeof(T) / sizeof(unsigned int) - 1;
	unsigned int i, k;
	size_t nodes_count = net.state().size();
	size_t blocks_count = nodes_count / 32 + 1;
	if(nodes_count % 32 == 0) {
		--blocks_count;
	}
	size_t threads_count = 32;
	
	thrust::device_vector<int> state0 = net.state(), xs = net.state(), ys = net.state();
	thrust::device_vector<node_behavior> dev_behavior = net.behavior();
	thrust::device_vector<int> functions_storage = net.functions();
	boolean_functions bfs(net.function_size(), net.arguments_size(), functions_storage);
	thrust::device_vector<int> equal_to_reference_state(nodes_count);

	for(i = 1, k = 0; i < 100000/*T[max]*/; ++i){
		int* xs_ptr = thrust::raw_pointer_cast(xs.data());
		int* ys_ptr = thrust::raw_pointer_cast(ys.data());
		node_behavior* b_ptr = thrust::raw_pointer_cast(dev_behavior.data());
		int* ref_ptr = thrust::raw_pointer_cast(state0.data());
		int* ref_eq_ptr = thrust::raw_pointer_cast(equal_to_reference_state.data());

		update_state<<<blocks_count, threads_count>>> (
			nodes_count,
			xs_ptr,
			ys_ptr,
			b_ptr,
			bfs,
			ref_ptr, // reference state
			ref_eq_ptr
		);

		xs.swap(ys);

		//std::cout << i << ": " << reduction_result  << std::endl;
		if(thrust::any_of(equal_to_reference_state.begin(), equal_to_reference_state.end(), thrust::identity<bool>())) {
			//break;
		}

		if(i == T[k]){
			++k;
			//cout << "ts";
			state0 = xs;
			thrust::fill(dev_behavior.begin(), dev_behavior.end(), node_behavior());
		}
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
