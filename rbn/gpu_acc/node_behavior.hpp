#ifndef GPU_ACC__NODE_BEHAVIOR_HPP
#define GPU_ACC__NODE_BEHAVIOR_HPP

struct node_behavior {
	node_behavior()
		: sum(0)
		, changes(0) {};
	long sum;
	long changes;
};

#endif