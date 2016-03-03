#ifndef GPU__DETAIL__HOST__STRUCTURE
#define GPU__DETAIL__HOST__STRUCTURE

#include <vector>

namespace gpu {

namespace detail {

namespace host {

struct structure {
    structure(int width, int height)
    : m_values(), m_col_indexes(), m_row_ptrs(), m_width(width) {
        m_row_ptrs.reserve(height);
    }
    int height() const { return m_row_ptrs.size() - 1; }
    int width() const { return m_width; }
    std::vector<int>& values() { return m_values; };
    const std::vector<int>& values() const { return m_values; };
    std::vector<int>& col_indexes() { return m_col_indexes; };
    const std::vector<int>& col_indexes() const { return m_col_indexes; };
    std::vector<int>& row_ptrs() { return m_row_ptrs; };
    const std::vector<int>& row_ptrs() const { return m_row_ptrs; };
private:
    std::vector<int> m_values;
    std::vector<int> m_col_indexes;
    std::vector<int> m_row_ptrs;
    int m_width;
};

} // namespace host

} // namespace detail

} // namespace gpu

#endif // GPU__DETAIL__HOST__STRUCTURE
