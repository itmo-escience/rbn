#ifdef ENABLE_GPU_ACCELERATION

// Compiled with C++ compiler

#include "find_attractor.hpp"

#include <set>

#include "../node_fwd.hpp"

#include "dev_find_attractor.hpp"
#include "node_behavior.hpp"

#include <iostream>

namespace gpu_acc {

int find_attractor(std::set<node_ptr>& nodes) {
	network net(nodes);
	int length = dev_find_attractor(net);
	//std::cout << "gpu sum[0]: " << net.behavior()[0].sum << std::endl;
	net.notify_nodes(nodes);
	return length;
}

} // namespace gpu_acc

#endif