#ifndef GPU_ACC__BOOLEAN_FUNCTIONS_HPP
#define GPU_ACC__BOOLEAN_FUNCTIONS_HPP

#include <ctime>
#include <iterator>
#include <vector>
#include <thrust/device_vector.h>
#include <thrust/host_vector.h>

#include "../node_fwd.hpp"

#include "array_view.hpp"
#include "boolean_function.hpp"

namespace gpu_acc {

class boolean_functions {
public:
	boolean_functions(int func_size, int args_size, const thrust::device_vector<int>& storage);

	__device__ boolean_function operator[] (size_t index) const;
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
