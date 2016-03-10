#include "liu_bassler_attractor_finder.hpp"

#include <thrust/fill.h>
#include <iostream>
#include <iterator>
#include <stdexcept>

#include "equal_to_ref_array.hpp"
#include "pitched_circular_matrix.hpp"
#include "rbn.hpp"

namespace gpu {

namespace detail {

namespace device {

namespace {

template<int M>
struct behavior_collector {
    behavior_collector(thrust::device_ptr<int> sum_array, thrust::device_ptr<int> changes_array)
            : m_sum(thrust::raw_pointer_cast(sum_array)), m_changes(thrust::raw_pointer_cast(changes_array)) { }

    __device__
    void operator()(int node_index, int x, int y) {
        m_sum[node_index] += M * y;
        if (y != x) {
            m_changes[node_index] += M;
        }
    }

private:
    int* m_sum;
    int* m_changes;
};


struct compare_states {
    compare_states(thrust::device_reference<bool> equal_to_ref, thrust::device_ptr<const int> reference_state)
            : m_reference_state(thrust::raw_pointer_cast(reference_state)),
              m_equal_to_ref(thrust::raw_pointer_cast(&equal_to_ref)) { }

    __device__
    void operator()(int node_index, int x) {
        if (x != m_reference_state[node_index]) {
            *m_equal_to_ref = false;
        }
    }

private:
    const int* m_reference_state;
    bool* m_equal_to_ref;
};


struct if_not {
    if_not(thrust::device_reference<const bool> var)
            : m_var(thrust::raw_pointer_cast(&var)) { };

    __device__
    bool operator()() {
        return !(*m_var);
    }

private:
    const bool* m_var;
};

struct liu_bassler_attractor_finder {
    liu_bassler_attractor_finder(const rbn& net, size_t iterations_limit);

    attractor_info operator()(state& xs);

private:
    const rbn& m_net;
    const size_t m_state_buffer_size;
    size_t m_iterations_limit;
    std::vector<size_t> m_thresholds;
    state m_reference_state;
    pitched_circular_matrix<int> m_states;
    thrust::device_vector<int> m_sum;
    thrust::device_vector<int> m_changes;
};


liu_bassler_attractor_finder::liu_bassler_attractor_finder(const rbn& net, size_t iterations_limit)
        : m_net(net)
        , m_state_buffer_size(3u)
        , m_iterations_limit(iterations_limit)
        , m_thresholds()
        , m_reference_state(m_net.size(), 0)
        , m_states(m_net.size(), m_state_buffer_size)
        , m_sum(m_net.size(), 0)
        , m_changes(m_net.size(), 0)
{
    for(size_t i = 100u; i <= (iterations_limit / 2); i *= 10u) {
        m_thresholds.push_back(i);
        m_thresholds.push_back(2u * i);
    }
	assert(m_state_buffer_size < m_thresholds[0]);
}

attractor_info liu_bassler_attractor_finder::operator()(state& xs) {
    attractor_info attractor = {0, 0};

    attractor.changes.resize(m_net.size());
    attractor.sum.resize(m_net.size());

    behavior_collector<+1> add_sum_and_changes(m_sum.data(), m_changes.data());
	
    equal_to_ref_array equal_to_ref(m_state_buffer_size);
	thrust::copy(xs.begin(), xs.end(), m_states.data());
    thrust::copy(xs.begin(), xs.end(), m_reference_state.begin());
    thrust::device_ptr<int> reference_state = m_reference_state.data();

    for (size_t i = 1, k = 0; i <= m_iterations_limit; ++i) {
        m_net.get_next_state
            ( m_states[i - 1], m_states[i]
            , if_not(equal_to_ref.on_previous_iteration(i))
            , add_sum_and_changes
            , compare_states // State(i) == State(ref)
                ( equal_to_ref.on_current_iteration(i)
                , reference_state
                )
            );
        if (i == m_iterations_limit || i % m_state_buffer_size == 0) {
            attractor.length = equal_to_ref.find_first(i);
            if(attractor.length > 0) {
                if(k == 0) {
                    break;
                }
                if(attractor.length == m_iterations_limit + 1) {
                    attractor.length = m_iterations_limit;
                    attractor.transient = m_iterations_limit;
                    break;
                }
                attractor.length -= m_thresholds[k - 1];
                attractor.transient = m_thresholds[k - 1];
                break;
            }
            equal_to_ref.clear();
        }
        if(i == m_thresholds[k]) {
            thrust::fill(m_changes.begin(), m_changes.end(), 0);
            thrust::fill(m_sum.begin(), m_sum.end(), 0);
            thrust::copy(m_states[i], m_states[i] + m_net.size(), reference_state);
            ++k;
        }
    }
    thrust::copy(m_changes.begin(), m_changes.end(), attractor.changes.begin());
    thrust::copy(m_sum.begin(), m_sum.end(), attractor.sum.begin());
    return attractor;
}


} // namespace


attractor_info liu_bassler_find_attractor(const rbn& net, state& xs, size_t max_attractor_length) {
    return liu_bassler_attractor_finder(net, 2u * max_attractor_length)(xs);
}


} // namespace device

} // namespace detail

} // namespace gpu
