#pragma once

#include <memory>
#include <vector>

#include "../app_objects/app_object.h"
#include "../GFXPipeline.h"
#include "../EngineDevice.h"


namespace aveng {

	/**
	* Notes:
	* 
	* The renderSystem's lifecycle is not tied to the RenderPass'
	*/
	class RenderSystem {

	public:

		RenderSystem(EngineDevice &device, VkRenderPass renderPass);
		~RenderSystem();

		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator=(const RenderSystem&) = delete;
		void renderAppObjects(VkCommandBuffer commandBuffer, std::vector<AvengAppObject> &appObjects);



	private:

		void createPipelineLayout();

		// The renderPass will be used to create the pipeline
		void createPipeline(VkRenderPass renderPass);

		// The Graphics API - Pointer Allocated
		std::unique_ptr<GFXPipeline> gfxPipeline;

		EngineDevice &engineDevice;
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