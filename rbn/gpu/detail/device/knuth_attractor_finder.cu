#include "knuth_attractor_finder.hpp"

#include <thrust/fill.h>
#include <iostream>
#include <iterator>

#include "equal_to_ref_array.hpp"
#include "pitched_circular_matrix.hpp"
#include "rbn.hpp"

namespace gpu {

namespace detail {

namespace device {


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

struct always {
	__device__
	bool operator()() {
		return true;
	}
};


struct knuth_attractor_finder {
	knuth_attractor_finder(const rbn& net, size_t iterations_limit);

	attractor_info operator()(state& xs);

private:
	int find_attractor_multiple(const state& state_0, state& state_n);

	void advance_fast_state(int iteration, const equal_to_ref_array& equal_to_ref,
							behavior_collector<+1>& add_sum_and_changes);

	void advance_slow_state(int iteration, equal_to_ref_array& equal_to_ref,
							behavior_collector<-1>& substract_sum_and_changes,
							thrust::device_ptr<const int> compare_to);

	const rbn& m_net;
	size_t m_state_buffer_size;
    size_t m_iterations_limit;
	pitched_circular_matrix<int> m_slow_states;
	pitched_circular_matrix<int> m_fast_states;
	thrust::device_vector<int> m_sum;
	thrust::device_vector<int> m_changes;
};


knuth_attractor_finder::knuth_attractor_finder(const rbn& net, size_t iterations_limit)
		: m_net(net)
        , m_iterations_limit(iterations_limit)
        , m_state_buffer_size(1000u)
        , m_slow_states(m_net.size(), m_state_buffer_size)
        , m_fast_states(m_net.size(), m_state_buffer_size * 2)
        , m_sum(m_net.size(), 0)
        , m_changes(m_net.size(), 0)
{ }


void dump_pitched(const pitched_circular_matrix<int>& pm) {
	for (size_t j = 1u; j <= pm.height(); ++j) {
		for (size_t l = 0u; l < pm.width(); ++l) {
			int st = pm(j, l);;
			std::cout << st;
		}
		std::cout << std::endl;
	}
}

void knuth_attractor_finder::advance_fast_state
        ( int iteration, const equal_to_ref_array& equal_to_ref
        , behavior_collector<+1>& add_sum_and_changes)
{
	m_net.get_next_state(
			m_fast_states[2 * iteration - 2], m_fast_states[2 * iteration - 1]
			// If already found, just do nothing
			, if_not(equal_to_ref.on_previous_iteration(iteration))
			// Update behavior information. We will substract the sum
			// and update the changes correspondingly if we're
			// on a transient in the advance_slow_state function
			, add_sum_and_changes
			// No state comparison is needed at this step
			, skip_comparison()
	);
	//std::cout << "fst/2 = " << std::endl;
	//dump_pitched(m_fast_states);
	m_net.get_next_state(
			m_fast_states[2 * iteration - 1], m_fast_states[2 * iteration]
			// Same as the above
			, if_not(equal_to_ref.on_previous_iteration(iteration)), add_sum_and_changes, skip_comparison()
	);
	//std::cout << "fst = " << std::endl;
	//dump_pitched(m_fast_states);
}


void knuth_attractor_finder::advance_slow_state
		( int iteration, equal_to_ref_array& equal_to_ref
        , behavior_collector<-1>& substract_sum_and_changes
		, thrust::device_ptr<const int> compare_to)
{
	m_net.get_next_state(
			m_slow_states[iteration - 1], m_slow_states[iteration]
			// If already found, do nothing
			, if_not(equal_to_ref.on_previous_iteration(iteration))
			// This is a transient, so we need to 'substract' the behavior information
			, substract_sum_and_changes
			// [equal_to_ref.on_current_iteration(t) = true] <=> [State(t) == State(2t)]
			, compare_states(equal_to_ref.on_current_iteration(iteration), compare_to)
	);
}


int get_iteration_number(int i, int k, int r) {
	return (i / r) * r + k;
}


int knuth_attractor_finder::find_attractor_multiple(const state& state_0, state& state_n) {
    thrust::copy(state_0.begin(), state_0.end(), m_slow_states[0]);
    thrust::copy(state_0.begin(), state_0.end(), m_fast_states[0]);

	for (size_t i = 1; i < m_fast_states.height(); ++i) {
		thrust::fill(m_fast_states[i], m_fast_states[i] + m_fast_states.width(), -1);
	}

	for (size_t i = 1; i < m_slow_states.height(); ++i) {
		thrust::fill(m_slow_states[i], m_slow_states[i] + m_slow_states.width(), -1);
	}

	equal_to_ref_array equal_to_ref(m_state_buffer_size);

	behavior_collector<-1> substract_sum_and_changes(m_sum.data(), m_changes.data());
	behavior_collector<+1> add_sum_and_changes(m_sum.data(), m_changes.data());

	for (size_t i = 1; i <= m_iterations_limit; ++i) {
		advance_fast_state(i, equal_to_ref, add_sum_and_changes);
		advance_slow_state(i, equal_to_ref, substract_sum_and_changes, m_fast_states[i * 2]);
		if (i % m_state_buffer_size == 0) {
			int k = equal_to_ref.find_first(i);
			if (k > 0) {
                state_n.resize(m_net.size());
                thrust::copy(m_slow_states[k], m_slow_states[k] + m_net.size(), state_n.begin());
				return k;
			}
            equal_to_ref.clear();
		}
        if(i % 100000u == 0) {
            std::cout << "Iteration " << i << std::endl;
        }
	}
	return 0;
}

attractor_info knuth_attractor_finder::operator()(state& xs) {
	attractor_info attractor = {0, 0};
	state state_n;
	int n = find_attractor_multiple(xs, state_n);

    attractor.changes.resize(m_net.size());
    attractor.sum.resize(m_net.size());
    thrust::copy(m_changes.begin(), m_changes.end(), attractor.changes.begin());
    thrust::copy(m_sum.begin(), m_sum.end(), attractor.sum.begin());

    if(n == 0) {
        attractor.length = m_iterations_limit;
        return attractor;
    }

    equal_to_ref_array equal_to_ref_transient(m_state_buffer_size);
    equal_to_ref_array equal_to_ref_attractor(m_state_buffer_size);

    pitched_circular_matrix<int>& state_i = m_slow_states;
    pitched_circular_matrix<int>& state_n_plus_i = m_fast_states;
    thrust::copy(xs.begin(), xs.end(), state_i[0]);
    thrust::copy(state_n.begin(), state_n.end(), state_n_plus_i[0]);

    for (int i = 1; i <= n; ++i) {
        m_net.get_next_state
            ( state_n_plus_i[i - 1], state_n_plus_i[i]
            , always()
            , skip_behavior_information_collection()
            , compare_states // State(n + i) == State(n)
                ( equal_to_ref_attractor.on_current_iteration(i)
                , state_n.data() )
            );
        if (attractor.transient == 0) {
            m_net.get_next_state
                ( state_i[i - 1], state_i[i]
                , always()
                , skip_behavior_information_collection()
                , compare_states // State(i) == State(n + i)
                    ( equal_to_ref_transient.on_current_iteration(i)
                    , state_n_plus_i[i] )
            );
        }
        if (i == n || i % m_state_buffer_size == 0) {
            if(attractor.transient == 0) {
                attractor.transient = equal_to_ref_transient.find_first(i);
            }
            if(attractor.length == 0) {
                attractor.length = equal_to_ref_attractor.find_first(i);
            }
            equal_to_ref_transient.clear();
            equal_to_ref_attractor.clear();
        }
        if (attractor.transient > 0 && attractor.length > 0) {
            break;
        }
    }
    if(attractor.length == 0) {
        std::cout << "n: " << n << ", t: " << attractor.length << std::endl;
        //std::cin >> attractor.length;
        return attractor;
    }
    int k = n / attractor.length;
    if(k == 0) {
        std::cout << "n: " << n << ", k: " << k << ", t: " << attractor.length << std::endl;
        std::cin >> k;
    }
    if(k != 0) {
        for (size_t i = 0; i < m_net.size(); ++i) {
            attractor.sum[i] /= k;
            attractor.changes[i] /= k;
        }
    }
	return attractor;
}


attractor_info knuth_find_attractor(const rbn& net, state& xs) {
	return knuth_attractor_finder(net, 10u*1000000u)(xs);
}


} // namespace device

} // namespace detail

} // namespace gpu
