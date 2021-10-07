#pragma once
#include <glm/glm.hpp>

namespace aveng {

	const int NO_TEXTURE = 1000;
	const int PI = 3.14159265f; // 3589793238462643383279502884197969399375105820974944592 check yo'self
	const float PLAYER_ROLL_SPEED = 2.5f;

	enum texture {
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
		int			pn;	// PosNeg modifier
		glm::vec3	cameraView;
		glm::vec3	cameraPos;
		glm::vec3	cameraRot;
		glm::vec3	playerPos;
		glm::vec3	playerRot;
		glm::vec3	modRot;
		glm::vec3	modPos;
		float		modPI;
		float		player_modPI;
	};

}