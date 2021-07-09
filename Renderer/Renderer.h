#pragma once

#include <memory>
#include <vector>
#include <cassert>
#include "../aveng_window.h"
#include "../EngineDevice.h"
#include "../swapchain.h"
#include "../cool.h"

namespace aveng {



	class Renderer {

	public:

		Renderer(AvengWindow &window, EngineDevice &device);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer &operator=(const Renderer&) = delete;

		// Our app needs to be able to access the swap chain render pass in order to configure any pipelines it creates
		VkRenderPass getSwapChainRenderPass() const { return aveng_swapchain->getRenderPass(); }

		bool isFrameInProgress() const { return isFrameStarted; }

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer. The frame is not in progress.");
			return commandBuffers[currentFrameIndex];
		}

		int getFrameIndex() const
		{
			assert(isFrameStarted && "Cannot get the frame index when frame is not in progress.");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);



	private:

		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		AvengWindow& aveng_window;
		EngineDevice& engineDevice;

		// SwapChain aveng_swapchain{ engineDevice, aveng_window.getExtent() };	// previous stack allocated. Ptr makes it easier to rebuild when the window resizes
		std::unique_ptr<SwapChain> aveng_swapchain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex{0};
		int currentFrameIndex; // Not tied to the image index
		bool isFrameStarted{ false };
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