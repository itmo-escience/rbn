#ifndef GPU_ACC__FIND_ATTRACTOR_HPP
#define GPU_ACC__FIND_ATTRACTOR_HPP

#include <set>

#include "../node_fwd.hpp"

namespace gpu_acc {

int find_attractor(std::set<node_ptr>& nodes);

} // namespace gpu_acc

#endif