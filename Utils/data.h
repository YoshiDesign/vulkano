#pragma once
#include <glm/glm.hpp>

namespace aveng {

	struct Data {

		glm::vec4	mods;
		int			num_objs;
		float		dt;
		int			cur_pipe;
		int			sec;
	};

}