#pragma once
#include "../Peripheral/aveng_window.h"
#include "../data.h"
#include "../../avpch.h"

#define MAX_PIPELINES 2

namespace aveng {
	
	class WindowCallbacks {

	public:

		static void testKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			switch (key)
			{
			case GLFW_KEY_SPACE			: if (action == GLFW_PRESS) updatePipeline(); 
				break;
			case GLFW_KEY_RIGHT_BRACKET	: if (action == GLFW_PRESS) modRot.z += 0.010f * posNeg;
				break;
			case GLFW_KEY_LEFT_BRACKET	: if (action == GLFW_PRESS) modRot.y += 0.010f * posNeg;
				break;
			case GLFW_KEY_P				: if (action == GLFW_PRESS) modRot.x += 0.010f * posNeg;
				break;
			case GLFW_KEY_APOSTROPHE	: if (action == GLFW_PRESS)	 modTrans.z += 0.010f * posNeg;
				break;
			case GLFW_KEY_SEMICOLON		: if (action == GLFW_PRESS)	 modTrans.y += 0.010f * posNeg;
				break;
			case GLFW_KEY_L				: if (action == GLFW_PRESS)	 modTrans.x += 0.010f * posNeg;
				break;
			case GLFW_KEY_EQUAL			: if (action == GLFW_PRESS)  modPI += 0.010f * posNeg;
				break;
			case GLFW_KEY_HOME			: if (action == GLFW_PRESS)	 posNeg *= -1;
				break;
			default:
				break;
			}

		}
		//
		static void updatePipeline() 
		{
			current_pipeline = (current_pipeline + 1) % MAX_PIPELINES;
		}

		//
		static int getCurPipeline()
		{
			return current_pipeline;
		}

		static int current_pipeline;
		static glm::vec3 modRot;
		static glm::vec3 modTrans;
		static int posNeg;
		static float modPI;

	private:

	};

}