#ifndef GPU__ATTRACTOR_INFO
#define GPU__ATTRACTOR_INFO

#include <vector>

namespace gpu {

struct attractor_info {
	int length;
	int transient;
	std::vector<int> sum;
    std::vector<int> changes;
};

} // namespace gpu

#endif // GPU__ATTRACTOR_INFO
