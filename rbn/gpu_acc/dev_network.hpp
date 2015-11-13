#ifndef GPU_ACC__NETWORK_HPP
#define GPU_ACC__NETWORK_HPP

#include <set>
#include <vector>

#include "../node_fwd.hpp"

#include "node_behavior.hpp"

namespace gpu_acc {

struct network {
	network(const std::set<node_ptr>& nodes);

	void notify_nodes(std::set<node_ptr>& nodes) const;

	std::vector<int>& state() { return m_state; };
	const std::vector<int>& state() const { return m_state; };

	std::vector<node_behavior>& behavior() { return m_behavior; };
	const std::vector<node_behavior>& behavior() const { return m_behavior; };

	std::vector<int>& functions() { return m_bfs; };
	const std::vector<int>& functions() const { return m_bfs; };

	int arguments_size() const { return m_args_size; };
	int function_size() const { return m_func_size; };

private:
	void push_function(const std::vector<int>& func, const std::set<node_ptr>& args);
	std::vector<int> m_state;
	std::vector<node_behavior> m_behavior;
	int m_args_size;
	int m_func_size;
	std::vector<int> m_bfs;
};

} // namespace gpu_acc

#endif