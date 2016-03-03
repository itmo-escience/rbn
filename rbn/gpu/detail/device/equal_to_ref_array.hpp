#ifndef GPU__DETAIL__DEVICE__EQUAL_TO_REF_ARRAY
#define GPU__DETAIL__DEVICE__EQUAL_TO_REF_ARRAY

#include <thrust/device_vector.h>

namespace gpu {

namespace detail {

namespace device {

class equal_to_ref_array {
public:
    typedef thrust::device_vector<bool>::iterator iterator;
    typedef thrust::device_vector<bool>::const_iterator const_iterator;
    typedef thrust::device_vector<bool>::size_type size_type;
    typedef thrust::device_vector<bool>::difference_type difference_type;
    typedef bool value_type;

    equal_to_ref_array(size_type size);

    void clear();

    int find_first(int current_iteration);

    thrust::device_reference<bool> on_current_iteration(int current_iteration);

    thrust::device_reference<const bool> on_previous_iteration(int current_iteration) const;

    size_type size() const { return m_storage.size() - 1; }

    iterator begin() { return m_storage.begin(); }

    iterator end() { return m_storage.end(); }

    const_iterator begin() const { return m_storage.begin(); }

    const_iterator end() const { return m_storage.end(); }

    const_iterator cbegin() const { return m_storage.begin(); }

    const_iterator cend() const { return m_storage.end(); }

private:
    thrust::device_vector<bool> m_storage;
};

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


int equal_to_ref_array::find_first(int current_iteration) {
    iterator it = thrust::find(m_storage.begin(), m_storage.end(), true);
    if (it == m_storage.end()) {
        return 0;
    }
    difference_type check_state_each = m_storage.size() - 1;
    difference_type relative_to = ((current_iteration - 1) / check_state_each) * check_state_each;
    return relative_to + thrust::distance(m_storage.begin(), it);
}

} // namespace device

} // namespace detail

} // namespace gpu

#endif // GPU__DETAIL__DEVICE__EQUAL_TO_REF_ARRAY
