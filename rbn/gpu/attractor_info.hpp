#ifndef GPU__ATTRACTOR_INFO
#define GPU__ATTRACTOR_INFO

#include <vector>

namespace gpu {

struct attractor_info {
	int length;
	int transient;
	std::vector<int> sum;
    std::vector<int> changes;

	bool operator== (const attractor_info& rhs) {
		if(length != rhs.length) return false;
		//if(transient != rhs.transient) return false;
		if(sum != rhs.sum) return false;
		if(changes != rhs.changes) return false;
		return true;
	}
};

} // namespace gpu

#endif // GPU__ATTRACTOR_INFO
