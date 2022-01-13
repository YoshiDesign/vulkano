#include "KeyboardController.h"
#include "../GameplayFunctions.h"
#include "../Math/aveng_math.h"
#include "../../avpch.h"

#define exe GameplayFunctions

/*
* Note: The ship's rotation is controlled in 2 separate fashions.
* 1. On the fly - directly modifying the Roll value
* 2. Cumulatively in a vector and added to the player's roll vector before the function returns
* 
* This isn't for any particular reason other than each are modified by different pairs of buttons. (A & D) (L & R)
*/

#define LOG(x, y) std::cout << x << "\t" << y << std::endl
namespace aveng{

	KeyboardController::KeyboardController(AvengAppObject& _viewerObject, Data& _data) 
		: viewerObject{ _viewerObject }, data{_data}
	{};

	void KeyboardController::moveCameraXZ(GLFWwindow* window, float dt) {
		
		glm::vec3 rotate{ 0 };

		// TODO: Option - Lock Z on roll. Will assist first person mode ;)

		//if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS)
		//{
		//	rotate.x += 1.f;
		//}
		//if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS)
		//{
		//	rotate.x -= 1.f;
		//}		
		
		//if (glfwGetKey(window, keys.left) == GLFW_PRESS)
		//{
		//	rotate.y -= 1.f;
		//}
		//if (glfwGetKey(window, keys.right) == GLFW_PRESS)
		//{
		//	rotate.y += 1.f;
		//}

		// This if statement effectively makes sure that rotate (matrix) is non-zero
		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
			// Update according to Delta Time. Normalize keeps multiple rotations in sync so one direction doesn't rotate faster than another
			viewerObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
		}
		//
		//// Prevent things from going upside down
		//viewerObject.transform.rotation.x = glm::clamp(viewerObject.transform.rotation.x, -1.5f, 1.5f);
		//// 360 degree max rotation then repeat
		//viewerObject.transform.rotation.y = glm::mod(viewerObject.transform.rotation.y, glm::two_pi<float>());
		
		
		const glm::vec3 rightDir{ 0.0f, 0.f, 1.0f };
		const glm::vec3 upDir{ 0.f, -1.f, 0.f };

		glm::vec3 moveDir{ 0.f };
		//if (glfwGetKey(window, keys.w) == GLFW_PRESS)
		//{
		//	moveDir += forwardDir;
		//}
		//if (glfwGetKey(window, keys.s) == GLFW_PRESS)
		//{
		//	moveDir -= forwardDir;
		//}
		//if (glfwGetKey(window, keys.a) == GLFW_PRESS)
		//{
		//	moveDir -= rightDir;
		//}
		//if (glfwGetKey(window, keys.d) == GLFW_PRESS)
		//{
		//	moveDir += rightDir;
		//}
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
			// Update according to Delta Time. Normalize keeps multiple rotations in sync so one direction doesn't rotate faster than another
			viewerObject.transform.translation += climbSpeed * dt * glm::normalize(moveDir);
		}

	}

	void KeyboardController::updatePlayer(GLFWwindow* window, AvengAppObject& playerObject, float dt)
	{

		// Is the camera to the right or left of the ship?
		int orientation = playerObject.transform.translation.x < viewerObject.transform.translation.x ? 1 : -1;

		// Anchor to the camera in all degrees of motion, hopping up and down dependent on the z-translation.  cos(z / f) * a
		playerObject.transform.translation =
			glm::vec3(0.0f, (glm::cos(playerObject.transform.translation.z / 8) * .004) + .1f, -1.0f)
			+ glm::vec3(
				unitCircleTransform_vec3(viewerObject.transform.rotation.y, viewerObject.transform.translation, viewRadius, playerObject.transform.modPI, playerObject.transform.translation));

		playerObject.transform.rotation.x = 0.0 + viewerObject.transform.rotation.x;
		playerObject.transform.rotation.y = PI + .142 + viewerObject.transform.rotation.y;

		// These can be initialized to 0 in KeyboarController.h
		float absPI = abs(playerObject.transform.modPI - viewerObject.transform.modPI);
		glm::vec3 p_rotate{ 0.f };
		glm::vec3 p_affine{ 0.f };
		int W = glfwGetKey(window, keys.w);
		int S = glfwGetKey(window, keys.s);
		int A = glfwGetKey(window, keys.a);
		int R = glfwGetKey(window, keys.right);
		int L = glfwGetKey(window, keys.left);
		int D = glfwGetKey(window, keys.d);

		forwardDir = { sin(viewerObject.transform.rotation.y), 0.f, cos(viewerObject.transform.rotation.y) };

		{

			if (!A && !D) 
			{

				/*
					Note: What if viewerobj mod pi = playerobj mod pi
				*/

				// Correct the players offset from the center of the camera's view
				if (playerObject.transform.modPI > 3.14) playerObject.transform.modPI -= dt * abs(playerObject.transform.modPI - PI); // TODO Easing
				if (playerObject.transform.modPI < 3.14) playerObject.transform.modPI += dt * abs(playerObject.transform.modPI - PI);; // TODO Easing
				// Synchronize ModPI - Unused as of right now
				//if (viewerObject.transform.modPI > playerObject.transform.modPI) viewerObject.transform.modPI -= dt * .1;
				//if (viewerObject.transform.modPI < playerObject.transform.modPI) viewerObject.transform.modPI += dt * .1;

			}

			if (L == GLFW_PRESS || R == GLFW_PRESS)
			{
				if (L)
				{
					exe::torque = 1.0f;
					p_rotate.z += 1.0f;
				}
				if (R)
				{
					exe::torque = -1.0f;
					p_rotate.z -= 1.0f;
				}
				// High to low z delta
				p_rotate.z += exe::dz_low(dt) * exe::torque;
			}

			if (A == GLFW_PRESS || D == GLFW_PRESS) {

				// Rotation correction
				if ((!L && !R)) {
					if (playerObject.transform.rotation.z > 1.3)
					{
						playerObject.transform.rotation.z -= .30f;
					}

					if (playerObject.transform.rotation.z < -1.3)
					{
						playerObject.transform.rotation.z += .30f;
					}
				}

				if (A)
				{
					velocity.x -= .5f;
					c_affine.x = 1.0f;
					// Roll the craft according to the direction it is steering
					if (playerObject.transform.rotation.z < 1.0f) {
						playerObject.transform.rotation.z += 0.07;
					}
					else if (playerObject.transform.rotation.z < 1.2f) {
						playerObject.transform.rotation.z += 0.01;
					}
					
					// Shift the craft to one side
					if (playerObject.transform.modPI > 2.7) 
					{
						// Decrease PI delta
						playerObject.transform.modPI += exe::dpi_low(dt, abs(playerObject.transform.modPI - 2.7f));
					}
					else {

						playerObject.transform.modPI += exe::dpi_high(dt, 0.2f);
					}

					// 3D rotation is allowed --
					if (data.fly_mode) {

				
						// The player's model will be rotated due to the camera.
						// Rotate the camera
						c_rotate.y -= 5.f * dt;						// ... THIS
		
					}

				}
				if (D)
				{
					velocity.x += .5f;
					c_affine.x = 1.0f;
					// Roll the craft according to the direction it is steering
					if (playerObject.transform.rotation.z > -1.0f) {
						playerObject.transform.rotation.z -= 0.07;
					}
					else if (playerObject.transform.rotation.z > -1.2f) {
						playerObject.transform.rotation.z -= 0.01;
					}

					// Shift the craft to one side
					if (playerObject.transform.modPI < 3.6)
					{
						// Increase PI Delta
						playerObject.transform.modPI += exe::dpi_high(dt, abs(playerObject.transform.modPI - 3.6f));
					}
					else {
						playerObject.transform.modPI += exe::dpi_low(dt, 0.2f);
					}

					// 3D rotation is allowed --
					if (data.fly_mode) {

						// The player's model will be rotated due to the camera.
						// Rotate the camera
						c_rotate.y += 5.f * dt;						// ... THIS, when 3D
						
					}

				}
			
			}
			
			else {

				if (playerObject.transform.rotation.z != 0.0f && (!A && !D) && (!L && !R)) 
				{
					// Gradually return the player to level flight
					p_rotate.z += exe::counterTorque(dt, playerObject.transform.rotation.z);
				}

			}

		}

		if (W == GLFW_PRESS)
		{
			//c_affine.z = 1.0f;
			
			if (data.fly_mode)
			{
				velocity += forwardDir * dt;
			} 
			else {
				velocity.z += 0.1;
			}
		}
		if (S == GLFW_PRESS)
		{
			if (data.fly_mode)
			{
				velocity -= forwardDir * dt;
			}
			else {
				velocity.z -= 0.1;
			}
		}

		//c_affine.x += velocity.x;

		// Regenerate Barrel Roll
		if (!L && !R && exe::deltaRoll < 100.0f)
			exe::deltaRoll += 1.0f;

		// Floating point modulator for rotation
		if (playerObject.transform.rotation.z >= 6.28 || playerObject.transform.rotation.z <= -6.28) 
			playerObject.transform.rotation.z = 0.0f;

		// Update player affine transform. This is currently unused. The player is locked to the camera.
		/*if (glm::dot(c_affine, c_affine) > std::numeric_limits<float>::epsilon()) {
			playerObject.transform.translation += moveSpeed * dt * glm::normalize(p_affine);
		}*/

		// Update camera affine transform.
		if (glm::dot(c_affine, c_affine) > std::numeric_limits<float>::epsilon()) {
			viewerObject.transform.translation += velocity * glm::normalize(c_affine) * dt;
		}

		playerObject.transform.rotation.z += rollSpeed * dt * p_rotate.z;

		// update the camera's rotation components. This really only ever happens in 3D (fly_free_mode)
		viewerObject.transform.rotation += c_rotate * dt;
		//viewerObject.transform.modPI = playerObject.transform.modPI;

		// Data updates
		{
			data.pn = exe::torque;
			data.DPI = exe::deltaPI;
			data.DeltaRoll = exe::deltaRoll;
			data.player_z_rot = playerObject.transform.rotation.z;
			data.cameraDX = exe::camera_deltaX;
			data.velocity = velocity;
			data.forwardDir = forwardDir;
		}

	}

} 