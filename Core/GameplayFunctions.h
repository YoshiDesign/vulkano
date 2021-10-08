#pragma once
#include "app_object.h"
#include "Peripheral/KeyboardController.h"

namespace aveng {

	class GameplayFunctions {

	public:

		static float torque;
		static float ease;
		static float clamp_speed;
		static bool clamped;

		struct init {

		};

		GameplayFunctions()  = delete;

		static bool doBarrelRoll(AvengAppObject& appObject, float dt, int dir);
		static float counterTorque(float dt, float torque);
		//static float centerCorrection(float dt, float x, float vx);
		static float recenterObject(float dt, float x, float vx);
		static float clamp_roll();
		static glm::vec3 player_spawn_point();

	private:

	};

}