#include "boolean_functions.hpp"

#include <thrust/device_vector.h>
#include <thrust/transform.h>
#include <thrust/functional.h>
#include <thrust/iterator/counting_iterator.h>

#include "../host/boolean_functions.hpp"
#include "state.hpp"

namespace gpu {

namespace detail {

namespace device {

boolean_functions::boolean_functions(const host::boolean_functions& fs)
        : m_storage(fs.data()), m_max_kin(fs.max_kin()) { }

} // namespace device

} // namespace detail

} // namespace gpu
