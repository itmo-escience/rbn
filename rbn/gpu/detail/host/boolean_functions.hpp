#ifndef GPU__DETAIL__HOST__BOOLEAN_FUNCTIONS
#define GPU__DETAIL__HOST__BOOLEAN_FUNCTIONS

#include <vector>

namespace gpu {

namespace detail {

namespace host {

struct boolean_functions {
	boolean_functions(size_t max_kin, size_t nodes_count)
			: m_data((1u << max_kin) * nodes_count, 0), m_max_kin(max_kin) { };

	size_t function_size() const { return 1u << m_max_kin; };

	template<class It>
	void set_function(size_t node_index, It from, It to) {
		std::vector<int>::iterator bfs_from = m_data.begin();
		std::advance(bfs_from, node_index * function_size());
		std::copy(from, to, bfs_from);
	}

	size_t max_kin() const { return m_max_kin; };

	const std::vector<int>& data() const { return m_data; };
private:
	std::vector<int> m_data;
	size_t m_max_kin;
};

} // namespace host

} // namespace detail

} // namespace gpu

#endif // GPU__DETAIL__HOST__BOOLEAN_FUNCTIONS
