#ifndef GPU__DETAIL__DEVICE__PITCHED_CIRCULAR_MATRIX
#define GPU__DETAIL__DEVICE__PITCHED_CIRCULAR_MATRIX

/* This matrix is 'circular' like boost::circular_buffer is.
 * This class provides an easy access to the elements by the
 * negative indices and indices that are larger then the
 * matrix size.
 * Moreover, it uses pitched allocation to make the access to
 * the elements from GPU fast.
 */

#include <thrust/device_ptr.h>
#include <thrust/device_reference.h>

namespace gpu {

namespace detail {

namespace device {

template<class T>
class pitched_circular_matrix {
public:
	pitched_circular_matrix(size_t width, size_t height);
	~pitched_circular_matrix();
	thrust::device_ptr<T> operator[] (ptrdiff_t row);
	thrust::device_reference<T> operator() (ptrdiff_t row, ptrdiff_t col);
	thrust::device_ptr<const T> operator[] (ptrdiff_t row) const;
	thrust::device_reference<const T> operator() (ptrdiff_t row, ptrdiff_t col) const;
	size_t width() const { return m_width; }
	size_t height() const { return m_height; }
private:
	size_t m_width;
	size_t m_height;
	size_t m_pitch;
	thrust::device_ptr<T> m_storage;
};

template<class T>
pitched_circular_matrix<T>::pitched_circular_matrix(size_t width, size_t height)
	: m_width(width)
	, m_height(height)
	, m_pitch()
	, m_storage()
{
	T* ptr = 0;
	cudaMallocPitch((void**) &ptr, &m_pitch, m_width * sizeof(T), m_height);
	m_storage = thrust::device_ptr<T>(ptr);
}

template<class T>
pitched_circular_matrix<T>::~pitched_circular_matrix() {
	cudaFree(thrust::raw_pointer_cast(m_storage));
}

template<class T>
thrust::device_ptr<T> pitched_circular_matrix<T>::operator[] (ptrdiff_t row) {
	if(row < 0) {
		row += (row / m_height) * (m_height + 1);
	}
	char* row_ptr = reinterpret_cast<char*>(thrust::raw_pointer_cast(m_storage)) + (row % m_height) * m_pitch;
	return thrust::device_ptr<T>(reinterpret_cast<T*>(row_ptr));
}

template<class T>
thrust::device_ptr<const T> pitched_circular_matrix<T>::operator[] (ptrdiff_t row) const {
	if(row < 0) {
		row += (row / m_height) * (m_height + 1);
	}
	char* row_ptr = reinterpret_cast<char*>(thrust::raw_pointer_cast(m_storage)) + (row % m_height) * m_pitch;
	return thrust::device_ptr<const T>(reinterpret_cast<const T*>(row_ptr));
}

template<class T>
thrust::device_reference<T> pitched_circular_matrix<T>::operator() (ptrdiff_t row, ptrdiff_t col) {
	if(col < 0) {
		col = (col / m_width) * (m_width + 1);
	}
	return (*this)[row][col % m_width];
}

template<class T>
thrust::device_reference<const T> pitched_circular_matrix<T>::operator() (ptrdiff_t row, ptrdiff_t col) const {
	if(col < 0) {
		col += (col / m_width) * (m_width + 1);
	}
	return (*this)[row][col % m_width];
}

} // namespace device

} // namespace detail

} // namespace gpu

#endif // GPU__DETAIL__DEVICE__PITCHED_CIRCULAR_MATRIX
