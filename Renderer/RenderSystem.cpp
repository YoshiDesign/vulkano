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


	RenderSystem::RenderSystem(EngineDevice& device, VkRenderPass renderPass) : engineDevice{device}
	{
		createPipelineLayout();
		createPipeline(renderPass);
	}

	RenderSystem::~RenderSystem()
	{
		vkDestroyPipelineLayout(engineDevice.device(), pipelineLayout, nullptr);
	}
	 
	/*
	* Describe the pipeline layout in terms of how
	* we are programming it.
	*/
	void RenderSystem::createPipelineLayout()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		//  Structure specifying the parameters of a newly created pipeline layout object
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		// used to send additional data, other than vertex information, to our shaders 
		// such as textures and uniform buffer objects
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(engineDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout.");
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

		gfxPipeline = std::make_unique<GFXPipeline>(
			engineDevice,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig
		);

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
		if (pipe_no == 0) 
		{ 
			gfxPipeline->bind(frame_content.commandBuffer); 
		} else if (pipe_no == 1) 
		{ 
			gfxPipeline2->bind(frame_content.commandBuffer); 
		}

		// Every rendered object will use the same projection and view matrix
		auto projectionView = frame_content.camera.getProjection() * frame_content.camera.getView();

		for (auto& obj : appObjects) 
		{

			SimplePushConstantData push{};
			auto modelMatrix = obj.transform.mat4();
			push.transform = projectionView * obj.transform.mat4();
			push.modelMatrix = modelMatrix;

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