#pragma once
#include "../aveng_window.h"

#define MAX_PIPELINES 2

namespace aveng {
	
	class WindowCallbacks {

	public:

		static void testKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
				updatePipeline();
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

	private:
		static int current_pipeline;
	};

}