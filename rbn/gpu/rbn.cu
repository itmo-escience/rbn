#include "rbn.hpp"
#include "detail/device/rbn.hpp"

namespace gpu {

rbn::rbn(const detail::host::structure& structure, const detail::host::boolean_functions& bfs)
	: m_pimpl(new detail::device::rbn(structure, bfs)) { }


rbn::~rbn() {
	delete m_pimpl;
}

attractor_info rbn::find_attractor(detail::host::state& state, size_t max_attractor_length, bool use_knuth) {
	return m_pimpl->find_attractor(state, max_attractor_length, use_knuth);
}


} // namespace gpu
