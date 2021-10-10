#include "GameplayFunctions.h"
#include "data.h"
#include <iostream>
#define LOG(x, y) std::cout << x << "\t" << y << std::endl

namespace aveng {
	
	float GameplayFunctions::torque  = 1.f;
	float GameplayFunctions::deltaX  = 0.f;
	float GameplayFunctions::deltaZ  = 100.f;
	float GameplayFunctions::e_out   = 1.f;
	float GameplayFunctions::_nudge  = 0.5f;
	float GameplayFunctions::camera_deltaX = 0.0;
	bool  GameplayFunctions::clamped = false;


	/**/
	glm::vec3 GameplayFunctions::player_spawn_point()
	{
		return { 0.0f, -0.45f, 0.0f };
	}

	/**/
	bool GameplayFunctions::doBarrelRoll(AvengAppObject& appObject, float dt, int dir)
	{
		
	}

	// Delta X vectoring
	float GameplayFunctions::dx_high(float dt, float rate)
	{
		deltaX += 0.1f;
		if (deltaX > 1.0f) deltaX = 1.0f;
		return (rate + 0.1 * .01) * deltaX * dt;
	}
	float GameplayFunctions::dx_low(float dt, float rate)
	{
		deltaX -= 0.1;
		if (deltaX < -1.0f) deltaX = -1.0f;
		return (rate + 0.1 * .01) * deltaX * dt;
	}

	// Camera delta X vectoring
	float GameplayFunctions::camera_dx_high(float dt, float rate)
	{
		camera_deltaX += 0.1f;
		if (deltaX > 1.0f) deltaX = 1.0f;
		return deltaX * dt * rate;
	}
	float GameplayFunctions::camera_dx_low(float dt, float rate)
	{
		camera_deltaX -= 0.1;
		if (deltaX < -1.0f) deltaX = -1.0f;
		return deltaX * dt * rate;
	}

	// Delta Z vectoring
	float GameplayFunctions::dz_high(float dt)
	{
		// Not sure I'll need this
		if (deltaZ >=  100.0f)  return  100.0f;
		return ++deltaZ * dt;
	}
	float GameplayFunctions::dz_low(float dt)
	{
		if (deltaZ <= 0.0f)  return 0.0f;
		return --deltaZ * dt;
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
		return (1.9 * torque) * dt;
	}

	float GameplayFunctions::nudge(int direction, float dt)
	{
		_nudge -= 0.1f;
		return _nudge * dt;
	}

	float GameplayFunctions::recenterObject(float dt, float x, float vx)
	{
		float d = vx > x ? -.1f : .1f;
		return d * dt;
	}

}