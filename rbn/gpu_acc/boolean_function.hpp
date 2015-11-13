#ifndef GPU_ACC__BOOLEAN_FUNCTION
#define GPU_ACC__BOOLEAN_FUNCTION

#include "array_view.hpp"

namespace gpu_acc {

struct boolean_function {
    __device__
    boolean_function(array_view<const int> func, array_view<const int> args)
            : m_bf(func)
            , m_args(args) {};

    __device__
    int operator() (const int* xs) const {
		size_t sum = 0, mul = 1;
        for(size_t i = 0; i < m_args.size(); ++i) {
			sum += mul * xs[ m_args[i] ];
			mul *= 2;
        }
        return m_bf[sum];
    };
private:
    const array_view<const int> m_bf;
    const array_view<const int> m_args;
};

} // namespace gpu_acc

#endif
