#ifndef EXECUTION_POLICY_HPP
#define EXECUTION_POLICY_HPP

namespace execution_policy {

struct sequential_tag {};
struct openmp_parallel_tag {};
struct nested_openmp_parallel_tag {};
struct gpu_tag {};
struct hybrid_tag {};

inline sequential_tag             default()         { return sequential_tag(); }
inline sequential_tag             sequential()      { return sequential_tag(); }
inline openmp_parallel_tag        parallel()        { return openmp_parallel_tag(); }
inline nested_openmp_parallel_tag nested_parallel() { return nested_openmp_parallel_tag(); }
inline gpu_tag                    gpu()             { return gpu_tag(); }
inline hybrid_tag                 hybrid()          { return hybrid_tag(); }

} // namespace execution_policy

#endif // EXECUTION_POLICY_HPP