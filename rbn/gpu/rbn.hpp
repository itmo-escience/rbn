#ifndef GPU__RBN
#define GPU__RBN

#include <ostream>

#include "detail/host/state.hpp"
#include "detail/host/structure.hpp"
#include "detail/host/boolean_functions.hpp"
#include "attractor_info.hpp"

namespace gpu {

/* Unlike the original `hsystem` class, this
 * `rbn` class represents RBN's structure and 
 * the associated Boolean functions only.
 * It doesn't store any state information and
 * contains no implementation of any
 * algorithm for finding an attractor.
 * On the contrary: it works more like a facade
 * that hides all the details on _how_ we find
 * an attractor and collect nodes' behavior 
 * information.
 *
 * We use pImpl idiom so that we can
 * compile GPU-specific code with NVCC and
 * other parts of the program with general-purpose
 * compilers such as GCC or MSVC.
 *
 * One can find the implementation in 'detail/device/rbn.hpp'.
 * Currently, the only supported attractor finding 
 * algorithm is in 'detail/device/knuth_attractor_finder.hpp' */

namespace detail {
	namespace device {
		struct rbn;
	}
}

struct rbn {
    rbn(const detail::host::structure& structure, const detail::host::boolean_functions& bfs);
    ~rbn();
	attractor_info find_attractor(detail::host::state& state/*, size_t iterations_limit*/);
    void print_structure(std::ostream& os) const;

private:
    detail::device::rbn* m_pimpl;
};

} // namespace gpu

#endif //GPU__RBN
