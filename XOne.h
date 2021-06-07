#pragma once

#include <memory>
#include <vector>
#include "aveng_window.h"
#include "GFXPipeline.h"
#include "EngineDevice.h"
#include "swapchain.h"
#include "aveng_model.h"
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

	private:
		void loadModels();
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void drawFrame();

		// The window API - Stack allocated
		AvengWindow aveng_window{ WIDTH, HEIGHT, "Vulkan 0" };

		EngineDevice engineDevice{ aveng_window };

		SwapChain aveng_swapchain{ engineDevice, aveng_window.getExtent() };



		// The Graphics API - Pointer Allocated
		std::unique_ptr<GFXPipeline> gfxPipeline;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;
		std::unique_ptr<AvengModel> avengModel;

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