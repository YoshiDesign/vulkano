#include "avpch.h"
#include "XOne.h"
#include "Core/Math/aveng_math.h"
#include "Core/data.h"
#include "Core/Utils/aveng_utils.h"
#include "Core/aveng_frame_content.h"
#include "Core/Camera/aveng_camera.h"
#include "Core/Utils/window_callbacks.h"
#include "Core/GameplayFunctions.h"

#define INFO(ln, why) std::cout << "XOne.cpp::" << ln << ":\n" << why << std::endl;
#define DBUG(x) std::cout << x << std::endl;

namespace aveng {

	// Dynamic Helpers on window callback keys
	int WindowCallbacks::current_pipeline{ 1 };
	glm::vec3 WindowCallbacks::modRot{ 0.0f, 0.0f, 0.0f };
	glm::vec3 WindowCallbacks::modTrans{ 0.0f, 0.0f, 0.0f };
	int WindowCallbacks::posNeg = 1;
	bool WindowCallbacks::flightMode = false;
	float WindowCallbacks::modPI = PI;

	XOne::XOne() 
	{

		// Set callback functions for keys bound to the window
		glfwSetKeyCallback(aveng_window.getGLFWwindow(), WindowCallbacks::testKeyCallback);

		/*
		* Call the pool builder to setup our pool for construction.
		*/
		globalPool = AvengDescriptorPool::Builder(engineDevice)
			.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT * 4)
						 // Type							// Max no. of descriptor sets
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT * 8)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT * 16)
			//.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, SwapChain::MAX_FRAMES_IN_FLIGHT * 8)
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
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)			
				.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 8)
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
			// Write first set - Uniform Buffer containing our UBO and our Imager Sampler
			auto bufferInfo	= uboBuffers[i]->descriptorInfo();
			auto imageInfo	= imageSystem.descriptorInfoForAllImages();
			AvengDescriptorSetWriter(*globalDescriptorSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)	// First Binding
				.writeImage(1, imageInfo.data(), imageSystem.texture_paths.size()) // Second Binding
				.build(globalDescriptorSets[i]);

			// Write second set - Also a uniform buffer
			auto fragBufferInfo = fragBuffers[i]->descriptorInfo(sizeof(RenderSystem::FragUbo), 0);
			AvengDescriptorSetWriter(*fragDescriptorSetLayout, *globalPool)
				.writeBuffer(0, &fragBufferInfo)
				.build(fragDescriptorSets[i]);
		}

		KeyboardController keyboardController{viewerObject, data};
		// Note that the renderSystem is initialized with a pointer to the Render Pass
		RenderSystem renderSystem{
			engineDevice,
			viewerObject,
			renderer.getSwapChainRenderPass(),
			globalDescriptorSetLayout->getDescriptorSetLayout(),
			fragDescriptorSetLayout->getDescriptorSetLayout()
		};
		
		PointLightSystem pointLightSystem{
			engineDevice,
			renderer.getSwapChainRenderPass(),
			globalDescriptorSetLayout->getDescriptorSetLayout(),
		};

		//camera.setViewTarget(glm::vec3(-1.f, -2.f, -20.f), glm::vec3(0.f, 0.f, 3.5f));

		aveng_imgui.init (
			aveng_window,
			renderer.getSwapChainRenderPass(),
			renderer.getImageCount() 
		);

		/*
			Things to keep in mind:
			Object Space - Objects initially exist at the origin of object space
			World Space  - The model matrix created by the AppObject's transform component coordinates objects with World Space
			Camera Space - The view transformation, applied to our objects, moves objects from World Space into the camera's perspective,
						   where the camera is at the origin and all object's coord's are relative to their position and orientation

					* The camera does not actually exist, we're just transforming objects AS IF the camera were there
					
				We then apply the projection matrix, capturing whatever is contained by the viewing frustrum, which then transforms
				it to the canonical view volume. As a final step the viewport transformation maps this region to actual pixel values.
					
		*/

		auto currentTime = std::chrono::high_resolution_clock::now();

		// When IMGui gets disabled this will be responsible for the initial PI
		data.modPI = PI;

		// Position the camera
		viewerObject.transform.translation.z = -2.5f;

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
			updateCamera(frameTime, viewerObject, keyboardController, camera);
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
					fragDescriptorSets[frameIndex],
					appObjects
				};

				ubo.projection = camera.getProjection();
				ubo.view = camera.getView();

				// Update our global uniform buffer 
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// Render
				renderer.beginSwapChainRenderPass(commandBuffer);
				renderSystem.renderAppObjects(
					frame_content, 
					data, 
					*fragBuffers[frameIndex]);
				pointLightSystem.render(frame_content);

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

		//auto triangle = AvengAppObject::createAppObject(THEME_1);
		//triangle.meta.type = SCENE;
		//triangle.model = AvengModel::drawTriangle(engineDevice);
		//appObjects.push_back(std::move(triangle));

		for (size_t i = 0; i < 1; i++)
		{

			for (size_t j = 0; j < 1; j++)
			{

				auto grid = AvengAppObject::createAppObject(THEME_2);
				grid.meta.type = GROUND;
				grid.model = AvengModel::createModelFromFile(engineDevice, "3D/plane.obj");
				grid.transform.translation = { 136.0f * i, -.1f, 0.0f};
				appObjects.emplace(grid.getId(), std::move(grid));

				auto grid2 = AvengAppObject::createAppObject(THEME_1);
				grid2.meta.type = GROUND;
				grid2.model = AvengModel::createModelFromFile(engineDevice, "3D/plane.obj");
				grid2.transform.translation = { 150.0f, -.1f, 170.0f };
				appObjects.emplace(grid2.getId(), std::move(grid2));

			}

		}

		//for (size_t i = 0; i < 10; i++) {
		//	for (size_t j = 0; j < 15; j++) {
		//		for (size_t k = 0; k < 5; k++) {

		//			auto triangle = AvengAppObject::createAppObject(NO_TEXTURE);
		//			triangle.meta.type = SCENE;
		//			triangle.model = AvengModel::drawTriangle(engineDevice, {static_cast<float>(i), static_cast<float>(j) * -1.0f, static_cast<float>(k) });
		//			appObjects.push_back(std::move(triangle));

		//		}
		//	}
		//}

		for (size_t i = 0; i < 10; i++)
		{

			for (size_t j = 0; j < 10; j++) 
			{

				for (size_t k = 0; k < 4; k++) {
					auto sphere = AvengAppObject::createAppObject(NO_TEXTURE);
					sphere.meta.type = SCENE;
					sphere.model = AvengModel::createModelFromFile(engineDevice, "3D/sphere.obj");
					sphere.transform.translation = { static_cast<float>(i) * 1.5f, static_cast<float>(j) * -1.0f, static_cast<float>(k) * 2.0f };
					sphere.transform.scale = {0.1f, 0.1f, 0.1f};
					appObjects.emplace(sphere.getId(), std::move(sphere));
				
				}
			
			}

		}

		//int t = 0;
		//for (int i = 0; i < 3; i++) 
		//	for (int j = 0; j < 3; j++) 
		//		for (int k = 0; k < 3; k++) {
		//			
		//			auto gameObj = AvengAppObject::createAppObject(1000);
		//			gameObj.model = coloredCubeModel;
		//			gameObj.meta.type = SCENE;
		//			gameObj.transform.translation = { static_cast<float>(-1.5 + (i * 1.5f)), static_cast<float>(-7 + -(j * 1.5f)), static_cast<float>(-(k * 1.5f)) };
		//			gameObj.transform.scale = { .4f, 0.4f, 0.4f };

		//			appObjects.push_back(std::move(gameObj));
		//			t = (t + 1) % 4;
		//		}

		// pendulum(engineDevice, 50);

	}

	void XOne::updateCamera(float frameTime, AvengAppObject& viewerObject, KeyboardController& keyboardController, AvengCamera& camera)
	{
		aspect = renderer.getAspectRatio();
		// Updates the viewer object transform component based on key input, proportional to the time elapsed since the last frame
		keyboardController.moveCameraXZ(aveng_window.getGLFWwindow(), frameTime);
		camera.setViewYXZ(viewerObject.transform.translation + glm::vec3(0.f, 0.f, -.80f), viewerObject.transform.rotation + glm::vec3());
		camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 1000.f);
	}

	void XOne::updateData()
	{
		data.cameraView = camera.getCameraView();
		data.cameraPos  = viewerObject.getPosition();
		data.cameraRot  = viewerObject.getRotation();
		data.fly_mode   = WindowCallbacks::flightMode;
	}

	void XOne::pendulum(EngineDevice& engineDevice, int _max_rows)
	{

		std::vector<float> factors;
		float length;
		float time = 7.0f;
		float gravity = 3.45f;
		float k = 7.0f;
		int max_rows = _max_rows;
		int row_modifier = 0;

		//std::unique_ptr<AvengModel> coloredCubeModel = AvengModel::createModelFromFile(engineDevice, "3D/colored_cube.obj");

		for (size_t i = 0; i < max_rows; i++)
		{
			//row_modifier = row_modifier % static_cast<int>(ceil(max_rows / 2) + 1);
			for (size_t j = 0; j < 1; j++) {
				auto gameObj = AvengAppObject::createAppObject(1000);
				//gameObj.model = coloredCubeModel;
				gameObj.model = AvengModel::createModelFromFile(engineDevice, "3D/colored_cube.obj");
				gameObj.meta.type = SCENE;

				if (i >= std::floor(max_rows / 2))
					gameObj.visual.pendulum_row = max_rows - row_modifier;
				else
					gameObj.visual.pendulum_row = row_modifier;

				length = gravity * glm::pow((time / (2 * glm::pi<float>()) * (k + gameObj.visual.pendulum_row + 1)), 2);
				length = length * .003;

				gameObj.visual.pendulum_delta = 0.0f;
				// To make this an actual pendulum, make the extent constant across all objects
				gameObj.visual.pendulum_extent = 70;
				gameObj.transform.velocity.x = length;
				gameObj.transform.translation = { 0.0f, static_cast<float>((i * -1.0f)), 0.0f };
				gameObj.transform.scale = { .4f, 0.4f, 0.4f };

				appObjects.emplace(gameObj.getId(), std::move(gameObj));
			}
			row_modifier++;
		}

	}

}