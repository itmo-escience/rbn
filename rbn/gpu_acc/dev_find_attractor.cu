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

__global__
void update_state(int iter, int nodes_count, const int* xs, int* ys, node_behavior* behavior,
					boolean_functions fs, int* ref, bool* already_found, bool* eq_to_ref)
{
	if(*already_found) return;
	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	if(idx < nodes_count) {
		int x = xs[idx];
		int y = fs(idx, xs);
		if(x != y) {
			behavior[idx].changes += 1;
		}
		behavior[idx].sum += y;
		ys[idx] = y;
		if(y != ref[idx]) {
			*eq_to_ref = false;
		}
	}
}

int equal_to_reference_on_iteration(const thrust::device_vector<bool>& etr_vector, int relative_to) {
	thrust::device_vector<bool>::const_iterator etr_on = thrust::find(etr_vector.begin(), etr_vector.end(), true);
	if(etr_on != etr_vector.end()) {
		return relative_to + thrust::distance(etr_vector.begin(), etr_on);
	} else {
		return 0;
	}
}

void clear_eq_to_ref(thrust::device_vector<bool>& etr_vector) {
	thrust::fill(etr_vector.begin(), etr_vector.end(), true);
	etr_vector[0] = false;
}

void clear_behavior(thrust::device_vector<node_behavior>& behavior) {
	thrust::fill(behavior.begin(), behavior.end(), node_behavior());
}

void get_results_from_gpu(thrust::device_vector<int>& state, thrust::device_vector<node_behavior>& behavior, network& net) {
	thrust::host_vector<int> h_state = state;
	net.state() = std::vector<int>(h_state.begin(), h_state.end());
	
	thrust::host_vector<node_behavior> h_behavior = behavior;
	net.behavior() = std::vector<node_behavior>(h_behavior.begin(), h_behavior.end());
}

} // namespace

int dev_find_attractor(network& net) {
	dev_init();
	unsigned int T[] = {100, 1000, 10000, 100000, 1000000};
	const int max = sizeof(T) / sizeof(unsigned int) - 1;
	unsigned int i, k;
	size_t nodes_count = net.state().size();
	size_t threads_count = 512;
	size_t blocks_count = nodes_count / threads_count + 1;
	if(nodes_count % threads_count == 0) {
		--blocks_count;
	}
	
	thrust::device_vector<int> state0 = net.state(), xs = net.state(), ys(net.state().size(), 0);
	thrust::device_vector<node_behavior> dev_behavior = net.behavior();
	thrust::device_vector<int> functions_storage = net.functions();
	boolean_functions bfs(net.function_size(), net.arguments_size(), functions_storage);
	
	int attractor_found_on = 0;
	// Synchronizing CPU and GPU after each kernel invokation (update_state) is very expensive
	// So we run (check_state_each) iterations, and then check if attractor was found
	// check_state_each must be less than T[k] for all k (will fix later)
	int check_state_each = 90;
	thrust::device_vector<bool> dev_eq_to_ref(check_state_each + 1);
	clear_eq_to_ref(dev_eq_to_ref);

	for(i = 1, k = 0; i < T[max]; ++i){
		int* xs_ptr = thrust::raw_pointer_cast(xs.data());
		int* ys_ptr = thrust::raw_pointer_cast(ys.data());
		node_behavior* b_ptr = thrust::raw_pointer_cast(dev_behavior.data());
		int* ref_ptr = thrust::raw_pointer_cast(state0.data());
		bool* eq_to_ref_ptr = thrust::raw_pointer_cast(dev_eq_to_ref.data());
		
		update_state<<<blocks_count, threads_count>>> (i,
			nodes_count,
			xs_ptr,
			ys_ptr,
			b_ptr,
			bfs,
			ref_ptr, // reference state
			eq_to_ref_ptr + ((i - 1) % check_state_each),
			eq_to_ref_ptr + ((i - 1) % check_state_each + 1)
		);

		xs.swap(ys);

		if(i % check_state_each == 0) {
			int relative_to = ((i - 1) / check_state_each) * check_state_each;
			attractor_found_on = equal_to_reference_on_iteration(dev_eq_to_ref, relative_to);
			if(attractor_found_on > 0) {
				break;
			}
			clear_eq_to_ref(dev_eq_to_ref);
			cudaDeviceSynchronize();
		}

		if(i == T[k]){
			++k;
			state0 = xs;
			clear_behavior(dev_behavior);
		}
	}

	get_results_from_gpu(xs, dev_behavior, net);

	if(attractor_found_on == 0){
		std::cout << "koniec";// << std::endl;
		return T[max] - T[max - 1];
	}
	else if(k > 0) {
		return attractor_found_on - T[k-1];
	}
	else return attractor_found_on;
}
	
} // namespace gpu_acc
