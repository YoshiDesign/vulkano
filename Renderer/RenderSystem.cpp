#include <iostream>
#define LOG(a) std::cout<<a<<std::endl;

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <array>

#include "RenderSystem.h"

namespace aveng {

	struct SimplePushConstantData 
	{
		glm::mat4 transform{ 1.f };
		glm::mat4 modelMatrix{ 1.f };
	};

	RenderSystem::RenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : engineDevice{ device }
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	RenderSystem::~RenderSystem()
	{
		vkDestroyPipelineLayout(engineDevice.device(), pipelineLayout, nullptr);
	}
	 
	/*
	* Describe the pipeline layout in terms of how
	* we are programming it.
	* 
	* A pipeline layout includes location, size and offset information about:
	* Descriptor sets and their layout
	* Push Constant data
	*/
	void RenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		//  Structure specifying the parameters of a newly created pipeline layout object
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		// Tell the pipeline about our descriptor sets
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()); // How many
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data(); // a pointer to an array of VkDescriptorSetLayout objects.
		// Push constants are used to send additional data to our shaders, similar to uniform buffers but much smaller and faster
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(engineDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

	}

	/*
	*/
	void RenderSystem::createPipeline(VkRenderPass renderPass)
	{
		// Initialize the pipeline 
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfig pipelineConfig{};
		GFXPipeline::defaultPipelineConfig(pipelineConfig);
		pipelineConfig.renderPass = renderPass;		
		pipelineConfig.pipelineLayout = pipelineLayout;

		// A GFXPipeline
		gfxPipeline = std::make_unique<GFXPipeline>(
			engineDevice,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig
		);

		// Another GFXPipeline
		gfxPipeline2 = std::make_unique<GFXPipeline>(
			engineDevice,
			"shaders/simple_shader2.vert.spv",
			"shaders/simple_shader2.frag.spv",
			pipelineConfig
		);
	}


	void RenderSystem::renderAppObjects(FrameContent& frame_content, std::vector<AvengAppObject>& appObjects, uint8_t pipe_no)
	{
		// Bind our current pipeline configuration
		//switch (pipe_no)
		//{
		//case 0: gfxPipeline->bind(frame_content.commandBuffer);  break;
		//case 1: gfxPipeline2->bind(frame_content.commandBuffer); break;
		//default:
		gfxPipeline->bind(frame_content.commandBuffer); // 0
		// }

		vkCmdBindDescriptorSets (
			frame_content.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&frame_content.globalDescriptorSet,
			0,
			nullptr);

		// Every rendered object will use the same projection and view matrix
		auto projectionView = frame_content.camera.getProjection() * frame_content.camera.getView();

		for (auto& obj : appObjects) 
		{

			SimplePushConstantData push{};
			push.modelMatrix = obj.transform.mat4();
			push.transform = projectionView * obj.transform.mat4();

			vkCmdPushConstants(
				frame_content.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push
			); 

			obj.model->bind(frame_content.commandBuffer);
			obj.model->draw(frame_content.commandBuffer);
		}
	}

} //