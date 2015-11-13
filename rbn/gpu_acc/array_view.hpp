#ifndef GPU_ACC__ARRAY_VIEW_HPP
#define GPU_ACC__ARRAY_VIEW_HPP

namespace gpu_acc {

template<class T>
struct array_view {
    __device__
    array_view(T* storage, size_t size)
            : m_storage(storage)
            , m_size(size) {};
    __device__ const T& operator[] (size_t i) const { return m_storage[i]; };
    __device__ T& operator[] (size_t i) { return m_storage[i]; };
    __device__ size_t size() const { return m_size; };
private:
    T* m_storage;
    size_t m_size;
};

} // namespace gpu_acc

#endif