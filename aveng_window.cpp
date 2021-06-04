#include "aveng_window.h"
#include <stdexcept>
namespace aveng {

	AvengWindow::AvengWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name }
	{
		initWindow();
	}

	AvengWindow::~AvengWindow()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	/**
	* Initialize the window
	* 
	*/
	void AvengWindow::initWindow()
	{
		glfwInit();

		// Instruct GLFW to NOT use the OpenGL API since we're using Vulkan
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// Open in windowed mode - Since we're using Vulkan we need to handle window resizing in a different way : See : TODO
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		// @type GLFWwindow* window;
		// @4th arg - Using windowed mode
		// @5th arg - Using OpenGL context
		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
	}

	void AvengWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window");
		}
	}
	bool AvengWindow::shouldClose() { return glfwWindowShouldClose(window); }

} // NS