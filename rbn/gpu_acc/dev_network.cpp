// Compiled by C++ compiler
#ifdef ENABLE_GPU_ACCELERATION

#include "dev_network.hpp"

#include <iostream>

#include "../node.hpp"

namespace gpu_acc {

void network::notify_nodes(std::set<node_ptr>& nodes) const {
	std::set<node_ptr>::iterator nodes_it = nodes.begin();
	std::vector<node_behavior>::const_iterator behavior_it = m_behavior.begin();
	std::vector<int>::const_iterator state_it = m_state.begin();
	for(; nodes_it != nodes.end(); ++nodes_it, ++behavior_it, ++state_it) {
		const node_behavior& b = *behavior_it;
		node& n = **nodes_it;
		n.sum = b.sum;
		n.changes = b.changes;
		n.state = *state_it;
		//std::cout << b.changes << " ";
	}
}

void network::push_function(const std::vector<int>& func, const std::set<node_ptr>& args) {
	m_bfs.push_back(args.size());
	//std::cout << "[" << args.size() << "] ";
	std::vector<int>::const_iterator f_it = func.begin();
	for(int i = 0; i < m_func_size; ++i) {
		if(f_it != func.end()) {
			//std::cout << *f_it << " ";
			m_bfs.push_back(*f_it);
			++f_it;
		} else {
			//std::cout << "X ";
			m_bfs.push_back(-1);
		}
	}
	std::set<node_ptr>::const_iterator a_it = args.begin();
	//std::cout << "(";
	for(int i = 0; i < m_args_size; ++i) {
		if(a_it != args.end()) {
			//std::cout << (*a_it)->get_codes(0) << " ";
			m_bfs.push_back((*a_it)->get_codes(0));
			++a_it;
		} else {
			//std::cout << "X ";
			m_bfs.push_back(-1);
		}
	}
	//std::cout << ")" << std::endl;
}

struct less_connections {
	bool operator() (const node_ptr& a, const node_ptr& b) {
		return a->get_Kin() < b->get_Kin();
	}
};

network::network(const std::set<node_ptr>& nodes)
	: m_state()
	, m_behavior(nodes.size())
	, m_args_size(0)
	, m_func_size(0)
	, m_bfs()
{
	node_ptr max_conn = *std::max_element(nodes.begin(), nodes.end(), less_connections());
	m_args_size = max_conn->get_Kin();
	m_func_size = 1 << m_args_size;
	m_state.reserve(nodes.size());
	m_bfs.reserve(nodes.size() * (m_func_size + m_args_size + 1));
	std::set<node_ptr>::const_iterator it = nodes.begin();
	for(; it != nodes.end(); ++it) {
		m_state.push_back((*it)->get_state());
		push_function((*it)->funs->functions, (*it)->input);
	}
}

} // namespace gpu_acc

#endif