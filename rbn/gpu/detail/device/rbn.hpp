#ifndef GPU__DETAIL__DEVICE__RBN
#define GPU__DETAIL__DEVICE__RBN

#include <thrust/device_ptr.h>
#include <ostream>

#include "../../../hparams.hpp"

#include "boolean_functions.hpp"
#include "structure.hpp"
#include "state.hpp"
#include "../host/structure.hpp"
#include "../host/state.hpp"
#include "../../attractor_info.hpp"

namespace gpu {

namespace detail {

namespace device {

/* Network structure is stored in an interesting way.
 * We use sparse matrix, in which
 *                 _
 *                |  0, if there's no connection from i to j
 *     a(i, j) = <
 *                |_ 2 ^ k, otherwise
 *
 * , where k is an index of the connection in the tuple of all connections to this node.
 *
 * Thus we can easily calculate the index (m) of BF value for each node:
 *
 *     | 1 |T     | 1 1 0 0 0 |     | 5 |
 *     | 0 |      | 0 2 0 1 0 |     | 1 |
 *     | 0 |   X  | 2 4 0 0 1 |  =  | 1 |
 *     | 1 |      | 4 0 1 2 0 |     | 2 |
 *     | 0 |      | 0 0 0 4 2 |     | 0 |
 *    (state)      (structure)       (m)
 *
 * Then we can determine the next state of each node using the gather operation:
 *
 *     | 5 |      | 0 0 0 0 0 1 0 0 |     | 1 |
 *     | 1 |      | 0 1 0 0 0 0 0 0 |     | 1 |
 *     | 1 | FROM | 1 0 1 1 1 1 1 1 |  =  | 0 |
 *     | 2 |      | 0 1 1 1 0 1 0 0 |     | 1 |
 *     | 0 |      | 0 0 0 1 1 1 1 0 |     | 0 |
 *      (m)       (Boolean functions)    (state)
 *
 * */

struct skip_behavior_information_collection {
	__device__
	void operator()(size_t, int, int) { };
};

struct skip_comparison {
	__device__
	void operator()(size_t, int) { };
};

struct rbn {
	rbn(const host::structure& structure, const host::boolean_functions& bfs);

	attractor_info find_attractor(host::state& state, size_t max_attractor_length, hparams::algorithm algorithm) const;

	size_t size() const { return m_structure.width(); }

	template<class When, class BehaviorInformationCollector, class Comparator>
	void get_next_state(thrust::device_ptr<const int> xs, thrust::device_ptr<int> ys, When predicate,
						BehaviorInformationCollector collector = skip_behavior_information_collection(),
						Comparator comparator = skip_comparison()
	) const;

private:
	structure m_structure;
	boolean_functions m_boolean_functions;
};

/* Unfortunately I have to put these in here because C++ doesn't
 * support defining templates in .cpp (or .cu) files yet. */

template<class When, class BehaviorInformationCollector, class Comparator>
__global__
void next_state_kernel
		(int num_rows, int max_kin, const int* ptr       // These are
		, const int* indices   // the sparse matrix
		, const int* data      // parameters
		, const int* boolean_funcs
        , const int* xs
        , int* ys
        , When predicate
		, BehaviorInformationCollector collector = skip_behavior_information_collection()
		, Comparator comparator = skip_comparison()
		)
{
	int row = blockDim.x * blockIdx.x + threadIdx.x;
	if (row < num_rows) {
        if (!predicate()) {
            ys[row] = -2;
            return;
        }
		int index = 0;
		int row_start = ptr[row];
		int row_end = ptr[row + 1];
		for (int jj = row_start; jj < row_end; jj++) {
			index += data[jj] * xs[indices[jj]];
		}
		int y = boolean_funcs[row * (size_t(1) << max_kin) + index];
		ys[row] = y;
		collector(row, xs[row], y);
		comparator(row, y);
	}
}


template<class When, class BehaviorInformationCollector, class Comparator>
void rbn::get_next_state
		( thrust::device_ptr<const int> xs
        , thrust::device_ptr<int> ys
        , When predicate
        , BehaviorInformationCollector collector
        , Comparator comparator
		) const
{
	next_state_kernel <<< static_cast<int>(size()) / 256 + 1, 256 >>>
        ( size()
        , m_boolean_functions.max_kin()
        , thrust::raw_pointer_cast(m_structure.row_ptrs().data())
        , thrust::raw_pointer_cast(m_structure.col_indexes().data())
        , thrust::raw_pointer_cast(m_structure.values().data())
        , thrust::raw_pointer_cast(m_boolean_functions.data().data())
        , thrust::raw_pointer_cast(xs)
        , thrust::raw_pointer_cast(ys)
        , predicate
        , collector
        , comparator
        );

}

} // namespace device

} // namespace detail

} // namespace gpu

#endif // GPU__DETAIL__DEVICE__RBN
