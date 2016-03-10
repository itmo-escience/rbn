#include "equal_to_ref_array.hpp"

namespace gpu {

namespace detail {

namespace device {

void equal_to_ref_array::clear() {
    thrust::fill(m_storage.begin(), m_storage.end(), true);
    m_storage[0] = false;
}

equal_to_ref_array::equal_to_ref_array(size_type size)
    : m_storage(size + 1, true)
{
    m_storage[0] = false;
};


thrust::device_reference<bool> equal_to_ref_array::on_current_iteration(int current_iteration) {
    return m_storage[(current_iteration - 1) % (m_storage.size() - 1) + 1];
}


thrust::device_reference<const bool> equal_to_ref_array::on_previous_iteration(int current_iteration) const {
    return m_storage[(current_iteration - 1) % (m_storage.size() - 1)];
}

int equal_to_ref_array::find_first(int current_iteration) const {
	return find_first(current_iteration, m_storage.begin(), m_storage.end());
}

int equal_to_ref_array::find_first(int current_iteration, size_t first_n) const {
	const_iterator end(m_storage.begin());
	thrust::advance(end, first_n);
	return find_first(current_iteration, m_storage.begin(),  end);
}

int equal_to_ref_array::find_first(int current_iteration, const_iterator begin, const_iterator end) const {
    const_iterator it = thrust::find(begin, end, true);
    if (it == end) {
        return 0;
    }
    difference_type check_state_each = m_storage.size() - 1;
    difference_type relative_to = ((current_iteration - 1) / check_state_each) * check_state_each;
    return relative_to + thrust::distance(begin, it);
}

} // namespace device

} // namespace detail

} // namespace gpu