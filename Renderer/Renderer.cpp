#include "Renderer.h"
#include <iostream>
#include <stdexcept>
#include <cassert>
#include <array>

#define LOG(a) std::cout<<a<<std::endl;


namespace aveng {

	Renderer::Renderer(AvengWindow& window, EngineDevice& device) : aveng_window{ window }, engineDevice{ device }
	{
		recreateSwapChain();
		createCommandBuffers();
	}

	Renderer::~Renderer()
	{
		freeCommandBuffers();
	}

	void Renderer::recreateSwapChain()
	{
		// Gety current window size
		auto extent = aveng_window.getExtent();

		// If the program has at least 1 dimension of 0 size (it's minimized); wait
		while (extent.width == 0 || extent.height == 0)
		{
			extent = aveng_window.getExtent();
			glfwWaitEvents();
		}

		// Wait until the current swap chain isn't being used before we attempt to construct the next one.
		vkDeviceWaitIdle(engineDevice.device());

		aveng_swapchain = nullptr;

		if (aveng_swapchain == nullptr) {
			// Create the new swapchain object
			aveng_swapchain = std::make_unique<SwapChain>(engineDevice, extent);

		}
		else {
			// 
			std::shared_ptr<SwapChain> oldSwapChain = std::move(aveng_swapchain);
			aveng_swapchain = std::make_unique<SwapChain>(engineDevice, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapFormats(*aveng_swapchain.get()))
			{
				throw std::runtime_error("Swap chain image format or depth format has changed.");
			}

		}

		// BRB

	}

	void Renderer::freeCommandBuffers()
	{
		vkFreeCommandBuffers(
			engineDevice.device(),
			engineDevice.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data()
		);

		commandBuffers.clear();
	}


	/*
	*/
	void Renderer::createCommandBuffers() {

		commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		// Command buffer memory is allocated from a command pool
		allocInfo.commandPool = engineDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(engineDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffers.");
		}

	}

	//
	VkCommandBuffer Renderer::beginFrame() 
	{
		assert(isFrameStarted && "Can't call beginFrame while already in progress.");

		auto result = aveng_swapchain->acquireNextImage(&currentImageIndex);

		// This error will occur after window resize
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return nullptr;
		}
		// This could potentially occur during window resize events
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to acquire swap chain image.");
		}

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Command Buffer failed to begin recording.");
		}

		return commandBuffer;

	}

	// 
	void  Renderer::endFrame()
	{
		assert(isFrameStarted && "Can't call endFrame while frame is not in progress.");
		auto commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record command buffer.");
		}
		// Submit to graphics queue while handlind cpu and gpu sync, executing the command buffers
		auto result = aveng_swapchain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || aveng_window.wasWindowResized())
		{
			aveng_window.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present swap chain image.");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;

	}
	void  Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
	
		assert(isFrameStarted && "Can't call beginSwapChain if frame is not in progress.");
		assert(commandBuffer == getCurrentCommandBuffer() && 
			"Can't begin render pass on command buffer from a different frame");


		/*
			Record Commands
		*/

		// 1. Begin a render pass
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = aveng_swapchain->getRenderPass();
		renderPassInfo.framebuffer = aveng_swapchain->getFrameBuffer(currentImageIndex);

		// The area where shader loading and storing takes place.
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = aveng_swapchain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// 2. Submit to command buffers to begin the render pass

		// VK_SUBPASS_CONTENTS_INLINE signals that subsequent renderpass commands come directly from the primary command buffer.
		// No secondary buffers are currently being utilized.
		// For this reason we cannot Mix both Inline command buffers AND secondary command buffers in our render pass execution.
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Configure the viewport and scissor
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(aveng_swapchain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(aveng_swapchain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, aveng_swapchain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);


	}

	void  Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Can't call endSwapChain if frame is not in progress.");
		assert(commandBuffer == getCurrentCommandBuffer() &&
			"Can't end render pass on command buffer from a different frame");
		vkCmdEndRenderPass(commandBuffer);
	}

} // NS