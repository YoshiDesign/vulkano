#include "KeyboardController.h"
#include "../GameplayFunctions.h"
#include "../../avpch.h"

#define exe GameplayFunctions
#define LOG(x, y) std::cout << x << "\t" << y << std::endl
namespace aveng{

	void KeyboardController::moveInPlaneXZ(GLFWwindow* window, float dt, AvengAppObject& appObject) {
		
		glm::vec3 rotate{ 0 };

		// TODO: Option - Lock Z on roll. Will assist first person mode ;)

		if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS)
		{
			rotate.x += 1.f;
		}
		if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS)
		{
			rotate.x -= 1.f;
		}

		// This if statement effectively makes sure that rotate (matrix) is non-zero
		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
			// Update according to Delta Time. Normalize keeps multiple rotations in sync so one direction doesn't rotate faster than another
			appObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
		}
		
		// Prevent things from going upside down
		appObject.transform.rotation.x = glm::clamp(appObject.transform.rotation.x, -1.5f, 1.5f);
		// 360 degree max rotation then repeat
		appObject.transform.rotation.y = glm::mod(appObject.transform.rotation.y, glm::two_pi<float>());

		float yaw = appObject.transform.rotation.y;
		const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
		const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
		const glm::vec3 upDir{ 0.f, -1.f, 0.f };

		glm::vec3 moveDir{ 0.f };
		if (glfwGetKey(window, keys.w) == GLFW_PRESS)
		{
			moveDir += forwardDir;
		}
		if (glfwGetKey(window, keys.s) == GLFW_PRESS)
		{
			moveDir -= forwardDir;
		}
		if (glfwGetKey(window, keys.a) == GLFW_PRESS)
		{
			moveDir -= rightDir;
		}
		if (glfwGetKey(window, keys.d) == GLFW_PRESS)
		{
			moveDir += rightDir;
		}
		if (glfwGetKey(window, keys.q) == GLFW_PRESS)
		{
			moveDir -= upDir;
		}
		if (glfwGetKey(window, keys.e) == GLFW_PRESS)
		{
			moveDir += upDir;
		}

		// This if statement effectively makes sure that rotate (matrix) is non-zero
		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
			// Update according to Delta Time. Normalize keeps multiple rotaions in sync so one direction doesn't rotate faster than another
			appObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
		}

	}

	void KeyboardController::updatePlayer(GLFWwindow* window, AvengAppObject& appObject, float dt)
	{
		glm::vec3 rotate{ 0 };
		int W = glfwGetKey(window, keys.w);
		int A = glfwGetKey(window, keys.a);
		int R = glfwGetKey(window, keys.right);
		int L = glfwGetKey(window, keys.left);
		int D = glfwGetKey(window, keys.d);

		{

			// a/d modifies pi up to 1/4th, or so, of what a full tilt to one side would do.
			// That way, if the player decides to go full tilt, we increase the speed at which
			// we are moving across the X axis

			if (A == GLFW_PRESS && appObject.transform.modPI > 1.4f)
			{

				appObject.transform.modPI -= PLAYER_ROLL_SPEED * dt;

			}
			if (D == GLFW_PRESS && appObject.transform.modPI < 4.5f)
			{

				appObject.transform.modPI += PLAYER_ROLL_SPEED * dt;

			}

			if (L == GLFW_PRESS && appObject.transform.rotation.z > -1.4 )
			{
				appObject.transform.rotation.z -= PLAYER_ROLL_SPEED * dt;
				appObject.transform.modPI += PLAYER_ROLL_SPEED * dt;
			}
			if (R == GLFW_PRESS && appObject.transform.rotation.z < 1.4 )
			{
				appObject.transform.rotation.z += PLAYER_ROLL_SPEED * dt;
				appObject.transform.modPI -= PLAYER_ROLL_SPEED * dt;
			}

			//appObject.transform.rotation.z = glm::clamp(appObject.transform.rotation.z, -1.8f, 1.8f);
			//

		}

		// Reverse the Z rotation
		if (appObject.transform.rotation.z != 0.0f) {
			// Return to level flight
			appObject.transform.rotation.z += exe::counterTorque(dt, appObject.transform.rotation.z);
		}
		 
	}


} 