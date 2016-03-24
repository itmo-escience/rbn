#ifndef GPU__DETAIL__DEVICE__BOOLEAN_FUNCTIONS
#define GPU__DETAIL__DEVICE__BOOLEAN_FUNCTIONS

#include <thrust/device_vector.h>
#include <ostream>

#include "../host/boolean_functions.hpp"
#include "state.hpp"

namespace gpu {

namespace detail {

namespace device {

struct boolean_functions {
    boolean_functions(const host::boolean_functions& fs);

    thrust::device_vector<int>& data() { return m_storage; }
    const thrust::device_vector<int>& data() const { return m_storage; }
    size_t max_kin() const { return m_max_kin; }

private:
    thrust::device_vector<int> m_storage;
    size_t m_max_kin;
};

} // namespace device

} // namespace detail

} // namespace gpu

#endif // GPU__DETAIL__DEVICE__BOOLEAN_FUNCTIONS
