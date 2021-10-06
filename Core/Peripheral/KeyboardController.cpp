#include "KeyboardController.h"

namespace aveng{

	void KeyboardController::moveInPlaneXZ(GLFWwindow* window, float dt, AvengAppObject& appObject) {
		
		glm::vec3 rotate{ 0 };

		// TODO: Option - Lock Z on roll. Will assist first person mode ;)

		/*if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) 
		{
			rotate.y += 1.f;
		}
		if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS)
		{
			rotate.y -= 1.f;
		}*/
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
		if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS)
		{
			moveDir += forwardDir;
		}
		if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS)
		{
			moveDir -= forwardDir;
		}
		if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS)
		{
			moveDir -= rightDir;
		}
		if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS)
		{
			moveDir += rightDir;
		}
		if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS)
		{
			moveDir -= upDir;
		}
		if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS)
		{
			moveDir += upDir;
		}

		// This if statement effectively makes sure that rotate (matrix) is non-zero
		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
			// Update according to Delta Time. Normalize keeps multiple rotaions in sync so one direction doesn't rotate faster than another
			appObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
		}

	}
} // ns aveng