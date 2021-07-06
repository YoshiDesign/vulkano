#pragma once

#include <memory>
#include <vector>
#include "app_objects/app_object.h"
#include "aveng_window.h"
#include "GFXPipeline.h"
#include "EngineDevice.h"
#include "swapchain.h"
#include "cool.h"

namespace aveng {



	class XOne {

	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		XOne();
		~XOne();

		XOne(const XOne&) = delete;
		XOne &operator=(const XOne&) = delete;
		
		void run();

		void sierpinski(
			std::vector<AvengModel::Vertex>& vertices,
			int depth,
			glm::vec2 left,
			glm::vec2 right,
			glm::vec2 top
		);

	private:
		void loadAppObjects();
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void freeCommandBuffers();
		void drawFrame();
		void recreateSwapChain();
		void recordCommandBuffer(int imageIndex);
		void renderAppObjects(VkCommandBuffer commandBuffer);

		// The window API - Stack allocated
		AvengWindow aveng_window{ WIDTH, HEIGHT, "Vulkan 0" };

		EngineDevice engineDevice{ aveng_window };

		// SwapChain aveng_swapchain{ engineDevice, aveng_window.getExtent() };	// previous stack allocated. Ptr makes it easier to rebuild when the window resizes
		std::unique_ptr<SwapChain> aveng_swapchain;

		// The Graphics API - Pointer Allocated
		std::unique_ptr<GFXPipeline> gfxPipeline;

		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;
		std::vector<AvengAppObject> appObjects;


	};

}


/* Note */
// The Graphics API - Previously stack allocated
//GFXPipeline gfxPipeline{
//	engineDevice, 
//	"shaders/simple_shader.vert.spv", 
//	"shaders/simple_shader.frag.spv", 
//	GFXPipeline::defaultPipelineConfig(WIDTH, HEIGHT) 
//};