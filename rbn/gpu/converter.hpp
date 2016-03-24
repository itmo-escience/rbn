#ifndef GPU__CONVERTER
#define GPU__CONVERTER

#include <functional>
#include <vector>
#include <stddef.h>

#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include "../hsystem.hpp"
#include "attractor_info.hpp"
#include "rbn.hpp"
#include "detail/host/state.hpp"
#include "detail/host/structure.hpp"
#include "detail/host/boolean_functions.hpp"

#include <iostream>
namespace gpu {

namespace {

struct get_bf_value_functor
	: std::unary_function<int, int>
{
	get_bf_value_functor(const funs_ptr& funs)
		: m_funs(funs) { }

	int operator() (int k) const {
		return m_funs->get_value(k);
	}
private:
	funs_ptr m_funs;
};

} // namespace

using detail::host::state;

struct converter {
	static rbn make_rbn(const hsystem& hs) {
		const nodes& all = hs.get_all();
		int max_kin = 0u;
		for(nodes::const_iterator it = all.begin(); it != all.end(); ++it) {
			node& n = **it;
			int kin = n.get_Kin();
			if(kin > max_kin) {
				max_kin = kin;
			}
		}
		detail::host::structure structure(all.size(), all.size());
		detail::host::boolean_functions boolean_functions(max_kin, all.size());
		size_t node_index = 0u;
		size_t i = 0u;
		structure.row_ptrs().push_back(0u);
		for(nodes::const_iterator it = all.begin(); it != all.end(); ++it, ++i) {
			structure.row_ptrs().push_back(structure.row_ptrs()[i]);
			nodes input = (*it)->get_input();
			if(input.empty()) {
				// If no inputs - make a loop (*it -> *it) and add an appropriate BF
				structure.values().push_back(1);
				structure.row_ptrs()[i + 1] += 1;
				structure.col_indexes().push_back(get_node_number(all, *it));
                const int bfs[] = {0, 1};
				boolean_functions.set_function(i, bfs, bfs + 2);
				continue;
			} else {
				int k = 1u;
				nodes::const_iterator in;
				for(in = input.begin(); in != input.end(); ++in) {
					structure.values().push_back(k);
					structure.row_ptrs()[i + 1] += 1;
					structure.col_indexes().push_back(get_node_number(all, *in));
					k *= 2;
				}
				get_bf_value_functor get_bf_value((*it)->get_boolean_functions());
				boost::counting_iterator<int> bf_indexes_begin(0);
				boost::counting_iterator<int> bf_indexes_end(1 << (*it)->get_Kin());
				boolean_functions.set_function(i,
						boost::make_transform_iterator(bf_indexes_begin, get_bf_value),
						boost::make_transform_iterator(bf_indexes_end, get_bf_value));
			}
		}
		return rbn(structure, boolean_functions);
	}

	static state get_state(hsystem& hs) {
		size_t i = 0u;
		nodes& all = hs.get_all();
        detail::host::state state(all.size());
		for(nodes::iterator it = all.begin(); it != all.end(); ++it, ++i) {
			node& n = **it;
			state[i] = n.get_state();
		}
		return state;
	}

	static void update_original_network(const attractor_info& ai, hsystem& hs) {
		size_t i = 0u;
		nodes& all = hs.get_all();
		for(nodes::iterator it = all.begin(); it != all.end(); ++it, ++i) {
			node& n = **it;
			n.set_sum(ai.sum[i]);
			n.set_changes(ai.changes[i]);
		}
	}
};

} // namespace gpu

#endif // GPU__CONVERTER