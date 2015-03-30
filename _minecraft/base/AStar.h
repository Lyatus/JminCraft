#pragma once

#include <stdio.h>
#include <math.h>
#include <list>
#include "world.h"

class AStar
{
public:
	typedef struct{
		int x, y, z;
	} Node;
	typedef struct{
		Node node;
		float truc;
	} TreatingNode;
	static std::vector<Node> compute(const Node& start, const Node& end) {
		std::list<TreatingNode> treating;
		treating.push_back({ start, });

	}
};