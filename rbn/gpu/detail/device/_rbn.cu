#include "rbn.hpp"

#include <thrust/device_vector.h>
#include <thrust/host_vector.h>
#include <thrust/equal.h>
#include <thrust/copy.h>

#include "knuth_attractor_finder.hpp"
#include "liu_bassler_attractor_finder.hpp"

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

attractor_info rbn::find_attractor(host::state& xs, size_t max_attractor_length, bool use_knuth) const {
    state device_xs(xs.begin(), xs.end());
	attractor_info ai;
	if(use_knuth) {
		ai = knuth_find_attractor(*this, device_xs, max_attractor_length);
	} else {
		ai = liu_bassler_find_attractor(*this, device_xs, max_attractor_length);
	}
    thrust::copy(device_xs.begin(), device_xs.end(), xs.begin());
    return ai;
}

} // namespace device

} // namespace detail

} // namespace gpu
