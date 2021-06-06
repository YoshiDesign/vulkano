#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace aveng {

	class AvengWindow {
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

		VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:

		void initWindow();

	};

} // NS