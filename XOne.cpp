#pragma once

#include "XOne.h"
#include <stdexcept>
#include <array>

namespace aveng {

	XOne::XOne() 
	{
		createPipelineLayout();
		createPipeline();
		createCommandBuffers();
	}

	XOne::~XOne()
	{
		vkDestroyPipelineLayout(engineDevice.device(), pipelineLayout, nullptr);
	}

	void XOne::run()
	{
		// Keep the window open until shouldClose is truthy
		while (!aveng_window.shouldClose()) {
			glfwPollEvents();
			drawFrame();
		}

		// Block until all GPU operations quit.
		vkDeviceWaitIdle(engineDevice.device());
	}

	void XOne::createPipelineLayout() 
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		// used to send additional data, other than vertex information, to our shaders 
		// such as textures and uniform buffer objects
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(engineDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout.");
		}
	}

	void XOne::createPipeline()
	{
		auto pipelineConfig = GFXPipeline::defaultPipelineConfig(aveng_swapchain.width(), aveng_swapchain.height());
		pipelineConfig.renderPass = aveng_swapchain.getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		gfxPipeline = std::make_unique<GFXPipeline>(
			engineDevice,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig
		);
	}


	void XOne::createCommandBuffers() {
	
		commandBuffers.resize(aveng_swapchain.imageCount());

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

		for (int i = 0; i < commandBuffers.size(); i++)
		{
		
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
			{
			
				std::runtime_error("Command Buffer failed to begin recording.");
			
			}
			/*
				Record Commands
			*/

			// 1. Begin a render pass
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = aveng_swapchain.getRenderPass();
			renderPassInfo.framebuffer = aveng_swapchain.getFrameBuffer(i);
			
			// The area where shader loading and storing takes place.
			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = aveng_swapchain.getSwapChainExtent();
		
			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			// 2. Submit to command buffers to begin the render pass

			// VK_SUBPASS_CONTENTS_INLINE signals that subsequent renderpass commands come directly from the primary command buffer.
			// No secondary buffers are currently being utilized.
			// For this reason we cannot Mix both Inline command buffers AND secondary command buffers in our render pass execution.
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			gfxPipeline->bind(commandBuffers[i]);
			// Records a draw command. 3 vertices in 1 instance. Instances can be used when you want to draw multiple copies of the same vertex data
			// e.g. particle systems
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0); // There are no data offsets here, we hardcoded vertices here.
			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to record command buffer.");
			}


		}

	}

	void XOne:: drawFrame() {
		uint32_t imageIndex;
		auto result = aveng_swapchain.acquireNextImage(&imageIndex);
		// This could potentially occur during window resize events
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to acquire swap chain image.");
		}

		// Submit to graphics queue while handlind cpu and gpu sync
		result = aveng_swapchain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		// The command buffer will now be executed

		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present swap chain image.");
		}
	}


} //