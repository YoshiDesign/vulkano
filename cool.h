#pragma once
#include <vector>
#include "aveng_model.h"

std::vector<aveng::AvengModel::Vertex> getVertices(std::vector<aveng::AvengModel::Vertex> vertices, int iter)
{
	if (iter == 1000)
		return vertices;
	iter++;
	std::vector<aveng::AvengModel::Vertex> vertices{ // vector
		{ {0.0f, -0.5f} /* glm vec2 position */ }, // Model Vertex
		{ {0.5f,  0.5f } },
		{ {-0.5f, 0.5f} }
	};

	return getVertices(vertices, iter);
}
