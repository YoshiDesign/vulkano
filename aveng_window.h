#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace aveng {

	class AvengWindow {

	private:

		const int width;
		const int height;
		std::string windowName;
		GLFWwindow* window;

	public:
		AvengWindow(int w, int h, std::string name);
		~AvengWindow();

		// Removal of copy construction. RAII 'til I die.
		AvengWindow(const AvengWindow&) = delete;
		AvengWindow& operator=(const AvengWindow&) = delete;

		bool shouldClose();

	private:

		void initWindow();

	};

} // NS