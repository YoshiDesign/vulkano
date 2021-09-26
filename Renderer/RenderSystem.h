#pragma once

#include <memory>
#include <vector>
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

		RenderSystem(EngineDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~RenderSystem();

		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator=(const RenderSystem&) = delete;
		void renderAppObjects(FrameContent& commandBuffer, std::vector<AvengAppObject> &appObjects, uint8_t pipe_no, glm::vec4 mods);
		VkPipelineLayout getPipelineLayout() { return pipelineLayout; }

	private:

		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		// The renderPass will be used to create the pipeline
		void createPipeline(VkRenderPass renderPass);
		
		int r = 0;

		EngineDevice &engineDevice;
		// The Graphics API - Pointer Allocated
		std::unique_ptr<GFXPipeline> gfxPipeline;
		std::unique_ptr<GFXPipeline> gfxPipeline2;
		VkPipelineLayout pipelineLayout;

	};

}