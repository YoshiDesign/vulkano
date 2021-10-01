#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <array>

#include "RenderSystem.h"

#define LOG(a) std::cout<<a<<std::endl;

namespace aveng {

	struct SimplePushConstantData 
	{
		alignas(16) glm::mat4 modelMatrix;
		alignas(16) int imDex;

	};

	RenderSystem::RenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayouts) : engineDevice{ device }
	{
		createPipelineLayout(descriptorSetLayouts);
		createPipeline(renderPass);
	}

	RenderSystem::~RenderSystem()
	{
		vkDestroyPipelineLayout(engineDevice.device(), pipelineLayout, nullptr);
	}
	 
	/*
	 * Setup of the pipeline layout. 
	 * Here we include our Push Constant information
	 * as well as our descriptor set layouts.
	 */
	void RenderSystem::createPipelineLayout(VkDescriptorSetLayout descriptorSetLayouts)
	{
		std::cout << "[] Size Of PushConstant\t" << sizeof(int) << std::endl;

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);	// Must be a multiple of 4

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;				// Hook it up
		pipelineLayoutInfo.setLayoutCount = 1; // How many descriptor set layouts are to be hooked into the pipeline
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayouts;							// a pointer to an array of VkDescriptorSetLayout objects.
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		// Create the pipeline layout, updating our pipelineLayout member.
		if (vkCreatePipelineLayout(engineDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}

	}

	/*
	* Call to the construction of a Graphics Pipeline.
	* Note that shader filepaths are relative to the GFXPipeline.cpp file.
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


	void RenderSystem::renderAppObjects(FrameContent& frame_content, std::vector<AvengAppObject>& appObjects, uint8_t pipe_no, glm::vec4& mods, int sec, float frametime)
	{
		// Bind our current pipeline configuration
		//switch (pipe_no)
		//{
		//case 0: gfxPipeline->bind(frame_content.commandBuffer);  break;
		//case 1: gfxPipeline2->bind(frame_content.commandBuffer); break;
		//default:
		gfxPipeline->bind(frame_content.commandBuffer); // 0
		// }
		
		vkCmdBindDescriptorSets(
			frame_content.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&frame_content.globalDescriptorSet,
			0,
			nullptr);

		/*
		* Thread object bind/draw calls here
		*/
		for (auto& obj : appObjects) 
		{

			SimplePushConstantData push{};

			if (last_sec != sec) {
				last_sec = sec;
				std::cout << obj.transform.translation.x << ", " << obj.transform.translation.y << ", " << obj.transform.translation.z << "\nMod w:" << mods.w << std::endl;
			}

			 
			if (obj.transform.translation.x > 10) {
				obj.transform.rotation = {
				static_cast<float>(obj.transform.rotation.x + frametime),
				static_cast<float>(obj.transform.rotation.y + frametime),
				static_cast<float>(obj.transform.rotation.z + frametime)
				};
			}

			if (obj.transform.translation.x < 10) {
				obj.transform.rotation = {
					static_cast<float>(obj.transform.rotation.x - frametime),
					static_cast<float>(obj.transform.rotation.y - frametime),
					static_cast<float>(obj.transform.rotation.z - frametime)
				};
			}

			if (obj.transform.translation.z < 10) {
				obj.transform.rotation = {
					static_cast<float>(obj.transform.rotation.x + frametime * 1.2),
					static_cast<float>(obj.transform.rotation.y + frametime * 1.2),
					static_cast<float>(obj.transform.rotation.z + frametime * 1.2)
				};
			}

			if (obj.transform.translation.z > 10) {
				obj.transform.rotation = {
					static_cast<float>(obj.transform.rotation.x - frametime),
					static_cast<float>(obj.transform.rotation.y + frametime),
					static_cast<float>(obj.transform.rotation.z - frametime * 1.1)
				};
			}

			//push.imDex = obj.get_texture();
			push.modelMatrix = obj.transform._mat4();
			push.imDex = obj.get_texture();

			//std::cout << "DEBUGE" << std::endl;
			//LOG(push.imDex);

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