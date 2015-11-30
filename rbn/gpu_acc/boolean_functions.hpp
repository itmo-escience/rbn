#ifndef GPU_ACC__BOOLEAN_FUNCTIONS_HPP
#define GPU_ACC__BOOLEAN_FUNCTIONS_HPP

#include <thrust/device_vector.h>

#include "../node_fwd.hpp"

namespace gpu_acc {

/*
	Current boolean functions representation is quite messy, and there is a better way to do it,
	but for now it's okay: performance implications on non-coalesced memory access are small
	compared to	time needed to call the kernel (GPU update_state function).
	
	Here is how one node's BF is represented in memory (max K_in = 3, node's K_in = 2):
	
	[ 2 inputs ][ 1, 0, 1, 0, ?, ?, ?, ? ][ node0, node2, ? ] // C++ type system cries out loud here
	
	"?" means that the value is undefined (we don't use it)

	This class uses the following assumptions:
		- m_args_size is always equal to `max K_in`,
		- m_function_size is always equal to 2 ^ `max K_in`
*/

class boolean_functions {
public:
	boolean_functions(int func_size, int args_size, const thrust::device_vector<int>& storage);

	__device__ int operator() (size_t index, const int* xs) const;
private:
    int m_function_size;
    int m_args_size;
    const int* m_storage;
};

__device__ int boolean_functions::operator() (size_t index, const int* xs) const {
	const int* it = m_storage + index * (m_function_size + m_args_size + 1);
	int args_count = *it;
	const int* function = it + 1;
	const int* args = function + m_function_size;
	size_t sum = 0, mul = 1;
    for(size_t i = 0; i < args_count; ++i) {
		sum += mul * xs[ args[i] ];
		mul *= 2;
    }
    return function[sum];
}

boolean_functions::boolean_functions(int func_size, int args_size, const thrust::device_vector<int>& storage)
	: m_function_size(func_size)
	, m_args_size(args_size)
	, m_storage(thrust::raw_pointer_cast(storage.data())) {}

} // namespace gpu_acc

#endif
