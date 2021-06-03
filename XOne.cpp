#pragma once

#include "XOne.h"

namespace aveng {

	void XOne::run()
	{
	
		// Keep the window open until shouldClose is truthy
		while (!aveng_window.shouldClose()) {

			glfwPollEvents();

		}

	}

}