#include "GameplayFunctions.h"
#include "data.h"
#include <iostream>
#define LOG(x, y) std::cout << x << "\t" << y << std::endl

namespace aveng {
	
	float GameplayFunctions::torque = 1.f;
	float GameplayFunctions::ease = 1.5;
	bool GameplayFunctions::clamped = false;

	/**/
	glm::vec3 GameplayFunctions::player_spawn_point()
	{
		return { 0.0f, -0.45f, 0.0f };
	}

	/**/
	bool GameplayFunctions::doBarrelRoll(AvengAppObject& appObject, float dt, int dir)
	{
		
	}

	/**/
	float GameplayFunctions::counterTorque(float dt, float z)
	{
		// Save last torque state
		torque = z > 0.0f ? -1.0f : 1.0f;

		// Flip the torque if we're beyond 3.14 radians in rotation
		if (abs(z) > PI)
		{
			torque = -torque;
		}

		if (abs(z) < .003f) return 0.0f;
		return (.7 * torque) * dt;
	}

	/**/
	float GameplayFunctions::clamp_roll()
	{
		--ease;
		if (abs(ease) < 0.01f) ease = 0.0f;
		return ease * torque;
	}

	float GameplayFunctions::recenterObject(float dt, float x, float vx)
	{
		float d = vx > x ? -.1f : .1f;
		return d * dt;
	}

}