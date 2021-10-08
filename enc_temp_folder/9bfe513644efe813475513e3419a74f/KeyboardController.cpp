#include "KeyboardController.h"
#include "../GameplayFunctions.h"
#include "../Math/aveng_math.h"
#include "../../avpch.h"

#define exe GameplayFunctions

#define LOG(x, y) std::cout << x << "\t" << y << std::endl
namespace aveng{

	KeyboardController::KeyboardController(AvengAppObject& _viewerObject) 
		: viewerObject{_viewerObject}
	{};

	void KeyboardController::moveCameraXZ(GLFWwindow* window, float dt) {
		
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
			viewerObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
		}
		
		// Prevent things from going upside down
		viewerObject.transform.rotation.x = glm::clamp(viewerObject.transform.rotation.x, -1.5f, 1.5f);
		// 360 degree max rotation then repeat
		viewerObject.transform.rotation.y = glm::mod(viewerObject.transform.rotation.y, glm::two_pi<float>());

		float yaw = viewerObject.transform.rotation.y;
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
			viewerObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
		}

	}

	void KeyboardController::updatePlayer(GLFWwindow* window, AvengAppObject& playerObject, float dt)
	{

		// Is the camera to the right or left of the ship?
		int orientation = playerObject.transform.translation.x < viewerObject.transform.translation.x ? 1 : -1;

		// Anchor to the camera in all degrees of motion, hopping up and down dependent on the z-translation.  cos(z / f) * a
		playerObject.transform.translation =
			glm::vec3(0.0f, (glm::cos(playerObject.transform.translation.z / 8) * .004) + .1f, -1.0f)
			+ glm::vec3(unitCircleTransform_vec3(viewerObject.transform.rotation.y, viewerObject.transform.translation, viewRadius, playerObject.transform.modPI, playerObject.transform.translation));

		playerObject.transform.rotation.x = 0.0 + viewerObject.transform.rotation.x;
		playerObject.transform.rotation.y = PI + .142 + viewerObject.transform.rotation.y;


		glm::vec3 rotate{ 0.0f };
		glm::vec3 affine{ 0.f };
		float modPI = 0.0;

		int W = glfwGetKey(window, keys.w);
		int A = glfwGetKey(window, keys.a);
		int R = glfwGetKey(window, keys.right);
		int L = glfwGetKey(window, keys.left);
		int D = glfwGetKey(window, keys.d);
		
		
		{

			if (!A && !D) 
			{
				// Correct the players offset from the center of the camera's view
				//if (playerObject.transform.modPI > 3.14) playerObject.transform.modPI -= dt * .5;
				//if (playerObject.transform.modPI < 3.14) playerObject.transform.modPI += dt * .5;
				
				if (viewerObject.transform.modPI > playerObject.transform.modPI) viewerObject.transform.modPI -= dt * .1;
				if (viewerObject.transform.modPI < playerObject.transform.modPI) viewerObject.transform.modPI += dt * .1;
			}

			// a/d modifies pi up to 1/4th, or so, of what a full tilt to one side would do.
			// That way, if the player decides to go full tilt, we increase the speed at which
			// we are moving across the X axis
			if (A || D) {

				// If pi is already out of clamped bounds, return to the bounded region
				if (playerObject.transform.rotation.z > 1.35)
				{
					playerObject.transform.rotation.z -= dt * 15;
				}
				else if (playerObject.transform.rotation.z < -1.35)
				{
					playerObject.transform.rotation.z += dt * 15;
				}
				else { 
					
					if (A == GLFW_PRESS)
					{
						//affine.x -= 1;
						rotate.z += 1;
						playerObject.transform.modPI -= dt * 1;

					}
					if (D == GLFW_PRESS)
					{
						//affine.x += 1;
						rotate.z -= 1;
						playerObject.transform.modPI += dt * 1;

					}
					if ((L || R) && barrelCooldown >= 99) { // Do a barrel roll

						barrelRolling = true;

					}
					
					else {
						// Clamp while steering
						playerObject.transform.modPI = glm::clamp(playerObject.transform.modPI, 2.7f, 3.6f);
						playerObject.transform.rotation.z = glm::clamp(playerObject.transform.rotation.z, -1.1f, 1.1f);
					}
					
				}
			}

			else if ((L || R) && (A || D)) {
				LOG("????", "????");
			}
			else if (L || R)
			{
				if (L == GLFW_PRESS)
				{
					rotate.z += 1;
				}
				if (R == GLFW_PRESS)
				{
					rotate.z -= 1;
				}
			}
			else {

				if (playerObject.transform.rotation.z != 0.0f) 
				{
					// Gradually return the player to level flight
					rotate.z += exe::counterTorque(dt, playerObject.transform.rotation.z);
				}

			}

		}

		// Floating point modulus for rotation
		if (playerObject.transform.rotation.z >= 6.28 || playerObject.transform.rotation.z <= -6.28) playerObject.transform.rotation.z = 0.0f;

		// Update the player's tranform component
		playerObject.transform.rotation.z += rollSpeed * dt * rotate.z;

		// Basically this means - if our affine transform is non zero, apply the translation
		if (glm::dot(affine, affine) > std::numeric_limits<float>::epsilon()) {
			playerObject.transform.translation += moveSpeed * dt * glm::normalize(affine);
		}
		 
	}

} 