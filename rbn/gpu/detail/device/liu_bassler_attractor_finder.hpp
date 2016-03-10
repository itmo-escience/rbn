#ifndef GPU__DETAIL__DEVICE__LIU_BASSLER_ATTRACTOR_FINDER
#define GPU__DETAIL__DEVICE__LIU_BASSLER_ATTRACTOR_FINDER

#include "../../attractor_info.hpp"
#include "state.hpp"

namespace gpu {

namespace detail {

namespace device {

struct rbn;

attractor_info liu_bassler_find_attractor(const rbn& net, state& xs);

} // namespace device

} // namespace detail

} // namespace gpu

#endif //GPU__DETAIL__DEVICE__LIU_BASSLER_ATTRACTOR_FINDER
