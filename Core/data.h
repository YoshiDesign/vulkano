#pragma once
#include <glm/glm.hpp>

namespace aveng {

	const int NO_TEXTURE = 1000;

	enum texture_map {
		SURFACE_GRID_1 = 0,
		THEME_1,
		THEME_2,
		THEME_3,
		THEME_4,
		RAND_1,
		RAND_2,
		RAND_3,
	};

	// Used by MetaComponents
	enum types {
		GROUND = 0,
		PLAYER,
		ENEMY,
		STATIC,
		DYNAMIC
	};

	// Debug info for GUI
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