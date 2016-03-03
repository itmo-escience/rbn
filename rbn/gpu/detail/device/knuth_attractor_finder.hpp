#ifndef GPU__DETAIL__DEVICE__KNUTH_ATTRACTOR_FINDER
#define GPU__DETAIL__DEVICE__KNUTH_ATTRACTOR_FINDER

#include "../../attractor_info.hpp"
#include "state.hpp"

namespace gpu {

namespace detail {

namespace device {

struct rbn;

attractor_info knuth_find_attractor(const rbn& net, state& xs);

} // namespace device

} // namespace detail

} // namespace gpu

#endif // GPU__DETAIL__DEVICE__KNUTH_ATTRACTOR_FINDER
