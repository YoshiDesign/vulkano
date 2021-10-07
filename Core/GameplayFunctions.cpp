#include "GameplayFunctions.h"
#include "data.h"
#include <iostream>
#define LOG(x, y) std::cout << x << "\t" << y << std::endl

namespace aveng {
	
	float GameplayFunctions::torque = 1.f;
	float GameplayFunctions::ease = 1.5;
	float GameplayFunctions::clamp_speed = PLAYER_ROLL_SPEED;
	bool GameplayFunctions::clamped = false;

	void GameplayFunctions::doBarrelRoll(AvengAppObject& appObject, float dt, int dir)
	{
		
	}

	float GameplayFunctions::counterTorque(float dt, float z)
	{
		// Save last torque state
		torque = z > 0.0f ? -1.0f : 1.0f;
		if (abs(z) < .003f) return 0.0f;
		return (.1 * torque) * dt;
	}

	float GameplayFunctions::clamp_roll()
	{
		--ease;
		if (abs(ease) < 0.01f) ease = 0.0f;
		return ease * torque;
	}

}