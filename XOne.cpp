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

#define INFO(ln, why) std::cout << "XOne.cpp::" << ln << ":\n" << why << std::endl;
#define DBUG(x) std::cout << x << std::endl;


namespace aveng {

	int WindowCallbacks::current_pipeline{ 1 };

	XOne::XOne() 
	{
		// Set callback functions for keys bound to the window
		glfwSetKeyCallback(aveng_window.getGLFWwindow(), WindowCallbacks::testKeyCallback);

		/*
		* Call the pool builder to setup our pool for construction.
		*/
		globalPool = AvengDescriptorPool::Builder(engineDevice)
			.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT * 24)
						 // Type							// Max no. of descriptor sets
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT * 8)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT * 16)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, SwapChain::MAX_FRAMES_IN_FLIGHT * 8)
			.build();

		loadAppObjects();

	}

	XOne::~XOne()
	{
		
	}

	void XOne::run()
	{

		// Create global uniform buffers mapped into device memory
		std::vector<std::unique_ptr<AvengBuffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
		std::vector<std::unique_ptr<AvengBuffer>> fragBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<AvengBuffer>(engineDevice,
														  sizeof(GlobalUbo),
														  1,
														  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
														  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}
		for (int i = 0; i < fragBuffers.size(); i++) {
			fragBuffers[i] = std::make_unique<AvengBuffer>(engineDevice,
														   sizeof(RenderSystem::FragUbo) * 8000,
														   1,
														   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
														   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			fragBuffers[i]->map();
		}

		// Descriptor Layout 0 -- Global
		std::unique_ptr<AvengDescriptorSetLayout> globalDescriptorSetLayout =							
			AvengDescriptorSetLayout::Builder(engineDevice)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)			
				.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4)
				//.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.build();	// Initialize the Descriptor Set Layout

		// Descriptor Set 1 -- Per object
		std::unique_ptr<AvengDescriptorSetLayout> fragDescriptorSetLayout =
			AvengDescriptorSetLayout::Builder(engineDevice)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT)
				.build();

		// Write our descriptors according to the layout's bindings once for every possible frame in flight
		std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
		std::vector<VkDescriptorSet> fragDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
		{
			// Write first set
			auto imageInfo	= imageSystem.descriptorInfoForAllImages();
			auto bufferInfo	= uboBuffers[i]->descriptorInfo();

			AvengDescriptorSetWriter(*globalDescriptorSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.writeImage(1, imageInfo.data(), imageSystem.nImages)
				.build(globalDescriptorSets[i]);

			// Write second set
			auto fragBufferInfo = fragBuffers[i]->descriptorInfo(sizeof(RenderSystem::FragUbo), 0);

			AvengDescriptorSetWriter(*fragDescriptorSetLayout, *globalPool)
				.writeBuffer(0, &fragBufferInfo)
				.build(fragDescriptorSets[i]);
		}

		// Note that the renderSystem is initialized with a pointer to the Render Pass
		RenderSystem renderSystem{ 
			engineDevice,
			renderer.getSwapChainRenderPass(), 
			globalDescriptorSetLayout->getDescriptorSetLayout(),
			fragDescriptorSetLayout->getDescriptorSetLayout()
		};

		//camera.setViewTarget(glm::vec3(-1.f, -2.f, -20.f), glm::vec3(0.f, 0.f, 3.5f));

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
		float dx = 0.0f;

		// Keep the window open until shouldClose is truthy
		while (!aveng_window.shouldClose()) {

			// Keep this on top. It can block
			glfwPollEvents();

			// Calculate time between iterations
			auto newTime = std::chrono::high_resolution_clock::now();
			frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;
			//frameTime = glm::min(frameTime, MAX_FRAME_TIME);	// Use this to lock to a specific max frame rate

			// Data & Debug
			updateCamera(frameTime, viewerObject, cameraController, camera);
			updateData();

			auto commandBuffer = renderer.beginFrame();
			if (commandBuffer != nullptr) {
				int frameIndex = renderer.getFrameIndex();
				FrameContent frame_content = {
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex],
					fragDescriptorSets[frameIndex]
				};
				
				dx += frameTime;
				if (dx > 1.0f) {
					dx = 0.0f;
					data.sec = (data.sec + 1) % 10000;
				}

				// Update our global uniform buffer
				ubo.projectionView = camera.getProjection() * camera.getView();
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// Render
				renderer.beginSwapChainRenderPass(commandBuffer);
				renderSystem.renderAppObjects(
					frame_content, 
					appObjects, 
					data, 
					*fragBuffers[frameIndex]);

				aveng_imgui.newFrame();
				aveng_imgui.runGUI(data);
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
		std::shared_ptr<AvengModel> plane    = AvengModel::createModelFromFile(engineDevice, "3D/plane.obj");
		std::shared_ptr<AvengModel> coloredCubeModel = AvengModel::createModelFromFile(engineDevice, "3D/colored_cube.obj");
		std::shared_ptr<AvengModel> rc = AvengModel::createModelFromFile(engineDevice, "3D/rc.obj");
		std::shared_ptr<AvengModel> bc = AvengModel::createModelFromFile(engineDevice, "3D/bc.obj");

		//auto gameObj = AvengAppObject::createAppObject(1);
		//gameObj.model = plane;
		//gameObj.transform.translation = { 0.f, 0.f, 0.f};
		//gameObj.transform.scale = { 1.f,1.f,1.f };

		//appObjects.push_back(std::move(gameObj));

		int t = 0;
		for (int i = 0; i < 3; i++) 
			for (int j = 0; j < 3; j++) 
				for (int k = 0; k < 3; k++) {
					
					auto gameObj = AvengAppObject::createAppObject(t);
					gameObj.model = bc;
					gameObj.transform.translation = { static_cast<float>(i * 4.55f), static_cast<float>(j * 1.55f), static_cast<float>(k * 1.55f) };
					gameObj.transform.scale = { .4f, 0.4f, 0.4f };

					appObjects.push_back(std::move(gameObj));
					t = (t + 1) % 4;
				}
	}

	void XOne::updateCamera(float frameTime, AvengAppObject& viewerObject, KeyboardController& cameraController, AvengCamera& camera)
	{
		aspect = renderer.getAspectRatio();
		// Updates the viewer object transform component based on key input, proportional to the time elapsed since the last frame
		cameraController.moveInPlaneXZ(aveng_window.getGLFWwindow(), frameTime, viewerObject);
		camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
		camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
	}

	void XOne::updateData()
	{
		data.num_objs = appObjects.size();
		data.cur_pipe = WindowCallbacks::getCurPipeline();
		data.dt			= frameTime;
		data.cameraView = camera.getCameraView();
		data.cameraPos = viewerObject.getPosition();
		data.cameraRot = viewerObject.getRotation();
	}

}