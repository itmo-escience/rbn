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

	int find_first(int current_iteration) const;
	int find_first(int current_iteration, size_t first_n) const;
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
	int find_first(int current_iteration, const_iterator begin, const_iterator end) const;
};

} // namespace device

} // namespace detail

} // namespace gpu

#endif // GPU__DETAIL__DEVICE__EQUAL_TO_REF_ARRAY
