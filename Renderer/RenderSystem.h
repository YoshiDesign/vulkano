#pragma once

#include <memory>
#include <vector>
#include "../app_objects/app_object.h"
#include "../Camera/aveng_camera.h"
#include "../EngineDevice.h"
#include "../aveng_frame_content.h"
#include "../GFXPipeline.h"

namespace aveng {

	class RenderSystem {

	public:

		RenderSystem(EngineDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayouts);
		~RenderSystem();

		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator=(const RenderSystem&) = delete;
		void renderAppObjects(FrameContent& frame_content, std::vector<AvengAppObject> &appObjects, uint8_t pipe_no, glm::vec4& mods, int dt, float frametime);
		VkPipelineLayout getPipelineLayout() { return pipelineLayout; }

	private:

		void createPipelineLayout(VkDescriptorSetLayout descriptorSetLayouts);
		void createPipeline(VkRenderPass renderPass);

		int last_sec;
		EngineDevice &engineDevice;

		// Rendering Pipelines - Heap Allocated
		std::unique_ptr<GFXPipeline> gfxPipeline;
		std::unique_ptr<GFXPipeline> gfxPipeline2;
		VkPipelineLayout pipelineLayout;

	};

}