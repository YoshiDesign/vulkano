#include "Renderer/RenderSystem.h"
#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <array>
#include "XOne.h"

#define LOG(a) std::cout<<a<<std::endl;

namespace aveng {

	XOne::XOne() 
	{
		loadAppObjects();
	}

	XOne::~XOne(){}

	void XOne::run()
	{
		RenderSystem renderSystem{ engineDevice, renderer.getSwapChainRenderPass() };
		// Keep the window open until shouldClose is truthy
		while (!aveng_window.shouldClose()) {
			glfwPollEvents();

			// The beginFrame function will return a nullptr if the swapchain needs to be recreated
			if (auto commandBuffer = renderer.beginFrame()) {
			
				renderer.beginSwapChainRenderPass(commandBuffer);
				renderSystem.renderAppObjects(commandBuffer, appObjects);
				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();

			}
		}

		// Block until all GPU operations quit.
		vkDeviceWaitIdle(engineDevice.device());
	}

	/*
	*/
	void XOne::loadAppObjects() 
	{
		std::vector<AvengModel::Vertex> vertices { // vector
			{ {0.0f, -0.5f }, {1.0f, 0.0f, 0.0f} }, // Model Vertex
			{ {0.5f,  0.5f }, {0.0f, 1.0f, 0.0f} },
			{ {-0.5f, 0.5f }, {0.0f, 0.0f, 1.0f} }
		};
		auto avengModel = std::make_shared<AvengModel>(engineDevice, vertices);

		std::vector<glm::vec3> colors{
			{1.f, .9f, .9f},
			{1.f, .2f, .53f},
			{.8f, 1.f, .43f},
			{.2f, 1.f, .8f},
			{.3f, .88f, 1.f}
		};

		for (auto& color : colors) {
			color = glm::pow(color, glm::vec3{ 2.2f });
		}

		for (int i = 1; i < 1000; i++) {

			// By using a shared ptr here we are making sure that 1 model instance can be used by multiple AppObjects
			// It will stay in memory so long as 1 object is still using it
			

			auto triangle = AvengAppObject::createAppObject();
			triangle.model = avengModel;
			triangle.transform2d.translation.x = i * .01f - 1;
			triangle.transform2d.scale = { i * .01f, i * .01f };
			triangle.transform2d.rotation = i * glm::two_pi<float>() * .025f;
			triangle.color = colors[i % colors.size()];
			appObjects.push_back(std::move(triangle));

		}

	}

} //