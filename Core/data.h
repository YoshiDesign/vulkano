#pragma once
#include <glm/glm.hpp>

namespace aveng {

	struct Data {
		int			num_objs;
		float		dt;
		int			cur_pipe;
		int			sec;
		glm::vec3	cameraView;
		glm::vec3	cameraPos;
		glm::vec3	cameraRot;
	};

}