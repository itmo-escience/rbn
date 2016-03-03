#ifndef GPU__DETAIL__DEVICE__STRUCTURE
#define GPU__DETAIL__DEVICE__STRUCTURE

#include <thrust/device_vector.h>
#include <ostream>
#include <algorithm>

namespace gpu {

namespace detail {

namespace device {

class structure {
public:
	template<class HostStructure>
	structure(HostStructure& src);

	int height() const { return m_height; }
	int width() const { return m_width; }
	thrust::device_vector<int>& values() { return m_values; };
	const thrust::device_vector<int>& values() const { return m_values; };
	thrust::device_vector<int>& col_indexes() { return m_col_indexes; };
	const thrust::device_vector<int>& col_indexes() const { return m_col_indexes; };
	thrust::device_vector<int>& row_ptrs() { return m_row_ptrs; };
	const thrust::device_vector<int>& row_ptrs() const { return m_row_ptrs; };

private:
	thrust::device_vector<int> m_values;
	thrust::device_vector<int> m_col_indexes;
	thrust::device_vector<int> m_row_ptrs;
	int m_height;
	int m_width;
};

template<class HostStructure>
structure::structure(HostStructure& src)
		: m_values(src.values()), m_col_indexes(src.col_indexes()), m_row_ptrs(src.row_ptrs()), m_height(src.height()),
		  m_width(src.width()) { }

} // namespace device

} // namespace detail

} // namespace gpu

#endif // GPU__DETAIL__DEVICE__STRUCTURE
