#pragma once

#include <memory>
#include <vector>
#include "../app_objects/app_object.h"
#include "../Camera/aveng_camera.h"
#include "../EngineDevice.h"
#include "../aveng_frame_content.h"
#include "../GFXPipeline.h"
#include "../Utils/data.h"

namespace aveng {

	class RenderSystem {

	public:

		struct FragUbo {
			alignas(sizeof(int)) int imDex;
		};


		RenderSystem(EngineDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalDescriptorSetLayout, VkDescriptorSetLayout fragDescriptorSetLayouts);
		~RenderSystem();

		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator=(const RenderSystem&) = delete;
		void renderAppObjects(FrameContent& frame_content, std::vector<AvengAppObject> &appObjects, Data data, AvengBuffer& fragBuffer);
		VkPipelineLayout getPipelineLayout() { return pipelineLayout; }

	private:

		void createPipelineLayout(VkDescriptorSetLayout* descriptorSetLayouts);
		void createPipeline(VkRenderPass renderPass);

		int last_sec;
		EngineDevice &engineDevice;
		size_t deviceAlignment = engineDevice.properties.limits.minUniformBufferOffsetAlignment;

		// Rendering Pipelines - Heap Allocated
		std::unique_ptr<GFXPipeline> gfxPipeline;
		std::unique_ptr<GFXPipeline> gfxPipeline2;
		VkPipelineLayout pipelineLayout;

	};

}