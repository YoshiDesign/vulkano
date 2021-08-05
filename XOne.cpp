#include <iostream>
#include "Renderer/RenderSystem.h"
#include "Camera/aveng_camera.h"
#include "KeyControl/KeyboardController.h"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <array>
#include <chrono>
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
		AvengCamera camera{};

		camera.setViewTarget(glm::vec3(-1.f, -2.f, -20.f), glm::vec3(0.f, 0.f, 3.5f));

		// Has no model or rendering. Used to store the camera's current state
		auto viewerObject = AvengAppObject::createAppObject();

		KeyboardController cameraController{};


		/*
			Things to keep in mind:
			Object Space - Objects initially exist at the origin of object space
			World Space - The model matrix created by the AppObject's transform component brings objects into World Space
			Camera Space - The view transformation, applied to our objects, moves objects from World Space into Camera Space
							where the camera is at the origin and all objects coords are relative to its position and orientation

					* The camera does not actually exist, we're just transforming objects AS IF the camera were there
					
				We then apply the projection matrix, capturing whatever is contained by the viewing frustrum and transforms
				it to the canonical view volume. As a final step the viewport transformation maps this region to actual pixel values.
					
		*/

		auto currentTime = std::chrono::high_resolution_clock::now();

		// Keep the window open until shouldClose is truthy
		while (!aveng_window.shouldClose()) {

			// Keep this on top bc it can block
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			// The amount of time which has passed since the last loop iteration
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			// Updates the viewer object transform component based on key input, proportional to the time elapsed since the last frame
			cameraController.moveInPlaneXZ(aveng_window.getGLFWwindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			// Keeps our projection in tune with our window aspect ratio during rendering
			float aspect = renderer.getAspectRatio();

			camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

			// The beginFrame function will return a nullptr if the swapchain needs to be recreated
			if (auto commandBuffer = renderer.beginFrame()) {
			
				renderer.beginSwapChainRenderPass(commandBuffer);
				renderSystem.renderAppObjects(commandBuffer, appObjects, camera);
				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();

			}
		}

		// Block until all GPU operations quit.
		vkDeviceWaitIdle(engineDevice.device());
	}

	// temporary helper function, creates a 1x1x1 cube centered at offset
	std::unique_ptr<AvengModel> createCubeModel(EngineDevice& device, glm::vec3 offset) {
		std::vector<AvengModel::Vertex> vertices{

			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

		};
		for (auto& v : vertices) {
			v.position += offset;
		}
		return std::make_unique<AvengModel>(device, vertices);
	}

	/*
	*/
	void XOne::loadAppObjects() 
	{
		
		std::shared_ptr<AvengModel> avengModel = createCubeModel(engineDevice, { .0f, .0f, .0f });

		auto cube = AvengAppObject::createAppObject();

		// This cube gets shrunk to half its size and centered in the view
		cube.model = avengModel;
		cube.transform.translation = { .0f, .0f, 3.5f };
		cube.transform.scale = { .5f, .5f, .5f };

		appObjects.push_back(std::move(cube));

	}

} // ns aveng