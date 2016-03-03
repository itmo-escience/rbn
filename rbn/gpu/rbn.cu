#include "rbn.hpp"
#include "detail/device/rbn.hpp"

namespace gpu {

rbn::rbn(const detail::host::structure& structure, const detail::host::boolean_functions& bfs)
	: m_pimpl(new detail::device::rbn(structure, bfs)) { }


rbn::~rbn() {
	delete m_pimpl;
}

attractor_info rbn::find_attractor(detail::host::state& state/*, size_t iterations_limit*/) {
	return m_pimpl->find_attractor(state);
}

void rbn::print_structure(std::ostream& os) const {
    m_pimpl->print_structure(os);
}

} // namespace gpu
