#pragma once

#include <memory>
#include <vector>
#include "Renderer.h"
#include "../app_objects/app_object.h"
#include "../Camera/aveng_camera.h"
#include "../EngineDevice.h"
#include "../aveng_frame_content.h"
#include "../GFXPipeline.h"


namespace aveng {

	/**
	* Notes:
	* 
	* The renderSystem's lifecycle is not tied to the RenderPass'
	*/
	class RenderSystem {

	public:

		RenderSystem(EngineDevice &device, Renderer &renderer, VkRenderPass renderPass);
		~RenderSystem();

		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator=(const RenderSystem&) = delete;
		void renderAppObjects(FrameContent& commandBuffer, std::vector<AvengAppObject> &appObjects, uint8_t pipe_no);
		VkPipelineLayout getPipelineLayout() { return pipelineLayout; }

	private:

		void createPipelineLayout();

		// The renderPass will be used to create the pipeline
		void createPipeline(VkRenderPass renderPass);

		// The Graphics API - Pointer Allocated
		std::unique_ptr<GFXPipeline> gfxPipeline;
		std::unique_ptr<GFXPipeline> gfxPipeline2;

		EngineDevice &engineDevice;
		Renderer& renderer;
		VkPipelineLayout pipelineLayout;

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