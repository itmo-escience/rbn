#include "rbn.hpp"

#include <thrust/device_vector.h>
#include <thrust/host_vector.h>
#include <thrust/equal.h>
#include <thrust/copy.h>

#include "knuth_attractor_finder.hpp"
#include "liu_bassler_attractor_finder.hpp"

#define KNUTH 0

namespace gpu {

namespace detail {

namespace device {

rbn::rbn(const host::structure& structure, const host::boolean_functions& bfs)
        : m_structure(structure), m_boolean_functions(bfs) { }

/* Currently we use Knuth's (The Art of Programming, vol. 2)
 * approach to find an attractor, because we can find all
 * the attractors regardless of their lengths and lengths
 * of the transient periods.
 *
 * Probably we will fall back to using the original algorithm
 * because it seems to be faster, but we need to determine
 * the correct values of T array (see: Liu, Bassler, 2006)
 * in order to skip large transient periods */

attractor_info rbn::find_attractor(host::state& xs, size_t max_attractor_length, hparams::algorithm algorithm) const {
    state device_xs1(xs.begin(), xs.end());
    state device_xs2(xs.begin(), xs.end());
	attractor_info ai1, ai2;
	ai1 = knuth_find_attractor(*this, device_xs1, max_attractor_length);
	ai2 = liu_bassler_find_attractor(*this, device_xs2, max_attractor_length);
	//if(!(ai1 == ai2)) {
		std::cout << "T" << ai1.length << " " << ai2.length << std::endl;
		std::cout << "Tr" << ai1.transient << " " << ai2.transient  << std::endl;
	//}
    thrust::copy(device_xs1.begin(), device_xs1.end(), xs.begin());
    return ai1;
}

} // namespace device

} // namespace detail

} // namespace gpu
