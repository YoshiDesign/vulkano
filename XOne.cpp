#include <iostream>
#include "Camera/aveng_camera.h"
#include "aveng_imgui.h"
#include "aveng_buffer.h"
#include "XOne.h"
#include "Utils/window_callbacks.h"
#include "Utils/aveng_utils.h"
#include "aveng_frame_content.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <array>
#include <numeric>
#include <chrono>
#include <memory>


namespace aveng {

	int WindowCallbacks::current_pipeline{ 1 };

	struct GuiStuff {

		glm::vec4 _mods;
		int no_objects;
		float dt;

	};

	XOne::XOne() 
	{
		// Set callback functions for keys bound to the window
		glfwSetKeyCallback(aveng_window.getGLFWwindow(), WindowCallbacks::testKeyCallback);

		/*
		* Call the pool builder to setup our pool for construction.
		*/
		globalPool = AvengDescriptorPool::Builder(engineDevice)
			.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
						 // Type							// Max no. of descriptor sets
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT * 4)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();

		loadAppObjects();

	}

	XOne::~XOne()
	{
		
	}

	void XOne::run()
	{

		auto minOffsetAlignment = std::lcm(
			engineDevice.properties.limits.minUniformBufferOffsetAlignment,
			engineDevice.properties.limits.nonCoherentAtomSize);

		// For use similar to a push_constant struct. Passing in read-only data to the pipeline shader modules
		struct GlobalUbo {
			alignas(16) glm::mat4 projectionView{ 1.f };
			alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3{ -1.f, -3.f, 1.f });
		};

		struct FragUbo {
			alignas(16) int consts[4] = { 0, 0, 0, 0 };
		};

		// Creating a uniform buffer
		AvengBuffer globalUboBuffer{
			engineDevice,
			sizeof(GlobalUbo),
			SwapChain::MAX_FRAMES_IN_FLIGHT,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			minOffsetAlignment
		};

		// enable writing to it's memory
		globalUboBuffer.map();

		ImageSystem imageSystem{ engineDevice };
		// Bind descriptor sets to the graphics pipeline

		// Descriptor Layout 0 -- Global
		std::unique_ptr<AvengDescriptorSetLayout> globalDescriptorSetLayout =									// The layout
			AvengDescriptorSetLayout::Builder(engineDevice)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)					// Its bindings
				.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4)		// Its bindings
				//.addBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.build();	// Initialize the Descriptor Set Layout

		// Write our descriptors according to the layout's bindings
		std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);			// A global descriptor set for each frame presented from our SwapChain
		for (int i = 0; i < globalDescriptorSets.size(); i++)
		{
			auto imageInfo		= imageSystem.descriptorInfoForAllImages();
			auto bufferInfo		= globalUboBuffer.descriptorInfoForIndex(i);
			//auto fragBufferInfo = fragBuffers[i]->descriptorInfo();
			AvengDescriptorSetWriter(*globalDescriptorSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.writeImage(1, imageInfo.data(), imageSystem.nImages)
				//.writeBuffer(2, &fragBufferInfo)
				.build(globalDescriptorSets[i]);
		}

		// Note that the renderSystem is initialized with a pointer to the Render Pass
		RenderSystem renderSystem{ engineDevice, renderer.getSwapChainRenderPass(), globalDescriptorSetLayout->getDescriptorSetLayout() };

		//camera.setViewTarget(glm::vec3(-1.f, -2.f, -20.f), glm::vec3(0.f, 0.f, 3.5f));

		// Has no model or rendering. Used to store the camera's current state
		AvengAppObject viewerObject = AvengAppObject::createAppObject();

		KeyboardController cameraController{};

		AvengImgui aveng_imgui{
			aveng_window,
			engineDevice,
			renderer.getSwapChainRenderPass(),
			renderer.getImageCount() 
		};

		/*
			Things to keep in mind:
			Object Space - Objects initially exist at the origin of object space
			World Space  - The model matrix created by the AppObject's transform component coordinates objects with World Space
			Camera Space - The view transformation, applied to our objects, moves objects from World Space into Camera Space;
							where the camera is at the origin and all object's coord's are relative to their position and orientation

					* The camera does not actually exist, we're just transforming objects AS IF the camera were there
					
				We then apply the projection matrix, capturing whatever is contained by the viewing frustrum, which then transforms
				it to the canonical view volume. As a final step the viewport transformation maps this region to actual pixel values.
					
		*/

		auto currentTime = std::chrono::high_resolution_clock::now();
		float x = 0.0f;
		int dt = 0;

		GuiStuff stuff{ mods, appObjects.size() };

		// Keep the window open until shouldClose is truthy
		while (!aveng_window.shouldClose()) {

			// Keep this on top. It can block
			glfwPollEvents();

			// Maintain the aspect ratio
			float aspect = renderer.getAspectRatio();

			// Calculate time between iterations
			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			//frameTime = glm::min(frameTime, MAX_FRAME_TIME);	// Use this to lock to a specific max frame rate

			updateCamera(frameTime, viewerObject, aspect, cameraController, camera);

			auto commandBuffer = renderer.beginFrame();

			if (commandBuffer != nullptr) {

				int frameIndex = renderer.getFrameIndex();
				FrameContent frame_content{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex],
				};

				x += frameTime;
				if (x > 1.0f) {
					x = 0.0f;
					dt += 1;
					if (dt > 10000) dt = 0;
					std::cout << dt << std::endl;
				}

				// Update our global uniform buffer
				GlobalUbo ubo{};
				ubo.projectionView = camera.getProjection() * camera.getView();
				globalUboBuffer.writeToIndex(&ubo, frameIndex);
				globalUboBuffer.flushIndex(frameIndex);

				// Render
				renderer.beginSwapChainRenderPass(commandBuffer);
				renderSystem.renderAppObjects(frame_content, appObjects, WindowCallbacks::getCurPipeline(), stuff._mods, dt, frameTime);
				aveng_imgui.newFrame();
				
				aveng_imgui.runGUI(stuff._mods, stuff.no_objects, stuff.dt);
				aveng_imgui.render(commandBuffer);

				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
				
			}

		}

		// Block until all GPU operations quit.
		vkDeviceWaitIdle(engineDevice.device());
	}

	/*
	* 
	*/
	void XOne::loadAppObjects() 
	{
		//fib(1000);
		std::shared_ptr<AvengModel> holyShipModel    = AvengModel::createModelFromFile(engineDevice, "3D/holy_ship.obj");
		std::shared_ptr<AvengModel> coloredCubeModel = AvengModel::createModelFromFile(engineDevice, "3D/colored_cube.obj");

		size_t t = 0;
		for (int i = 0; i < 2; i++) 
			for (int j = 0; j < 2; j++) 
				for (int k = 0; k < 1; k++) {
					std::cout << t << std::endl;
					t = (t + 1) % 4;
					auto gameObj = AvengAppObject::createAppObject();
					gameObj.model = coloredCubeModel;
					gameObj.transform.translation = { static_cast<float>(i * 7.2f), static_cast<float>(j * 7.2f), static_cast<float>(k * 7.2f) };
					gameObj.transform.scale = { .5f, 0.5f, 0.5f };
					gameObj.set_texture(t);

					appObjects.push_back(std::move(gameObj));
				
				}
		
	}

	/*int XOne::fib(int n, int a, int b)
	{
		auto gameObj = AvengAppObject::createAppObject();

		gameObj.model = AvengModel;
		gameObj.transform.translation = {
			static_cast<float>(static_cast<int>(b) % 250) * .001,
			static_cast<float>(static_cast<int>(b) % 550) * .005,
			static_cast<float>(static_cast<int>(b) % 50) * .005f
		};

		gameObj.transform.scale = { .05f , 0.05f, 0.05f };

		if (n == 0) {
			avengModelF = nullptr;
			return a;
		}
		if (n == 1) {
			return b;
		}

		appObjects.push_back(std::move(gameObj));
		return fib(n - 1, b, a + b);

	}*/

	void XOne::updateCamera(float frameTime, AvengAppObject& viewerObject, float aspect, KeyboardController& cameraController, AvengCamera& camera)
	{
		// Updates the viewer object transform component based on key input, proportional to the time elapsed since the last frame
		cameraController.moveInPlaneXZ(aveng_window.getGLFWwindow(), frameTime, viewerObject);
		camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
		camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
	}

} // ns aveng