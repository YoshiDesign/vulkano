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
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		// @type GLFWwindow* window;
		// @4th arg - Using windowed mode
		// @5th arg - Using OpenGL context
		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);

		// The parent AvengWindow object is 'this'
		glfwSetWindowUserPointer(window, this);

		// Whenever our window is resized, this callback is executed with the args in the callback's signature (new width and height)
		glfwSetFramebufferSizeCallback(window, framebufferResizedCallback);
	}

	void AvengWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface");
		}
	}
	bool AvengWindow::shouldClose() { return glfwWindowShouldClose(window); }

	void AvengWindow::framebufferResizedCallback(GLFWwindow* window, int width, int height)
	{
		auto avengWindow = reinterpret_cast<AvengWindow*>(glfwGetWindowUserPointer(window));
		avengWindow->framebufferResized = true;
		avengWindow->width = width;
		avengWindow->height = height;
	}

} // NS