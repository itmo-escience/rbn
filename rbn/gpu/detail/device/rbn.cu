#include "rbn.hpp"

#include <thrust/device_vector.h>
#include <thrust/host_vector.h>
#include <thrust/equal.h>
#include <thrust/copy.h>

#ifdef KNUTH
    #include "knuth_attractor_finder.hpp"
#else
    #include "liu_bassler_attractor_finder.hpp"
#endif

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

attractor_info rbn::find_attractor(host::state& xs) const {
    state device_xs(xs.begin(), xs.end());
#ifdef KNUTH
    attractor_info ai = knuth_find_attractor(*this, device_xs);
#else
    attractor_info ai = liu_bassler_find_attractor(*this, device_xs);
#endif
    thrust::copy(device_xs.begin(), device_xs.end(), xs.begin());
    return ai;
}

void rbn::print_structure(std::ostream& os) const {
    os << "Structure {" << std::endl;
    const structure& s = m_structure;
    os << "NNZ: ";
    std::copy(s.values().begin(), s.values().end(), std::ostream_iterator<int>(os, " "));
    os << std::endl;
    os << "IA: ";
    std::copy(s.row_ptrs().begin(), s.row_ptrs().end(), std::ostream_iterator<int>(os, " "));
    std::cout << std::endl;
    os << "JA: ";
    std::copy(s.col_indexes().begin(), s.col_indexes().end(), std::ostream_iterator<int>(os, " "));
    thrust::host_vector<int> bfs = m_boolean_functions.data();
    os << std::endl;
    os << "JA: ";
    std::copy(bfs.begin(), bfs.end(), std::ostream_iterator<int>(os, " "));
    os << std::endl;
}

} // namespace device

} // namespace detail

} // namespace gpu
