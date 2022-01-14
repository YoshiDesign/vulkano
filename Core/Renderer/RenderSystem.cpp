#include "RenderSystem.h"
#include "../Math/aveng_math.h"
#include "../Utils/window_callbacks.h"
#include "../GameplayFunctions.h"

#define exe GameplayFunctions

#define LOG(x, y) std::cout << x << "\t" << y << std::endl
#define BYPASS_FBO 0

namespace aveng {

	struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

	RenderSystem::RenderSystem(
		EngineDevice& device,
		AvengAppObject& viewer,
		AvengWindow& _aveng_window,
		VkRenderPass renderPass, 
		KeyboardController& _keyboardController,
		VkDescriptorSetLayout globalDescriptorSetLayouts,
		VkDescriptorSetLayout fragDescriptorSetLayouts) 
			: engineDevice{ device }, aveng_window{ _aveng_window }, viewerObject{ viewer }, keyboard_controller{_keyboardController}
	{
		VkDescriptorSetLayout descriptorSetLayouts[2] = { globalDescriptorSetLayouts , fragDescriptorSetLayouts };
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
	void RenderSystem::createPipelineLayout(VkDescriptorSetLayout* descriptorSetLayouts)
	{

		// Initialize push constant range(s)
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);	// Must be a multiple of 4

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 2;										// How many descriptor set layouts are to be hooked into the pipeline
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;						// a pointer to an array of VkDescriptorSetLayout objects.
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

	void RenderSystem::renderAppObjects(FrameContent& frame_content, std::vector<AvengAppObject>& appObjects, Data& data, AvengBuffer& fragBuffer)
	{
		int obj_no = 0;
		int direction = 1;

		// Bind our current pipeline configuration
		switch (data.cur_pipe)
		{
			case 98: gfxPipeline->bind(frame_content.commandBuffer);  break;
			case 99: gfxPipeline2->bind(frame_content.commandBuffer); break;
			default:
				gfxPipeline->bind(frame_content.commandBuffer); // 0
		}
		
		vkCmdBindDescriptorSets(
			frame_content.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&frame_content.globalDescriptorSet,
			0,
			nullptr);

		updateData(appObjects.size(), frame_content.frameTime, data);

		/*
		* Thread object bind/draw calls here
		*/
		for (auto& obj : appObjects) 
		{
			// This object's dynamic offset in relation to the Dynamic UBO
			uint32_t dynamicOffset = obj.get_texture() * static_cast<uint32_t>(deviceAlignment);
			obj_no += 1;

			FragUbo fubo{ obj.get_texture() };	// Texture information -- within our dynamic UBO (FragUbo is a bad name for this, but that's its only usecase right now)

			SimplePushConstantData push{};
			
			// 1s tick
			if (last_sec != data.sec) {
				last_sec  = data.sec;
			}

			// The matrix describing this model's current orientation
			push.modelMatrix  = obj.transform._mat4();
			push.normalMatrix = obj.transform.normalMatrix();

			if (!BYPASS_FBO) {

				if (dynamicOffset > engineDevice.properties.limits.maxUniformBufferRange) {
					std::cout << "Range: " << engineDevice.properties.limits.maxUniformBufferRange << std::endl;
					throw std::runtime_error("Attempting to allocate buffer beyond device uniform buffer memory limit.");
				}

				fragBuffer.writeToBuffer(&fubo, sizeof(FragUbo), dynamicOffset);
				fragBuffer.flush();
				vkCmdBindDescriptorSets(
					frame_content.commandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					pipelineLayout,
					1,
					1,
					&frame_content.fragDescriptorSet,
					1,
					&dynamicOffset);
			}

			vkCmdPushConstants(
				frame_content.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);

			obj.model->bind(frame_content.commandBuffer);
			obj.model->draw(frame_content.commandBuffer);

		}
	}

	void RenderSystem::updateData(size_t size, float frameTime, Data& data)
	{

		data.num_objs = size;
		data.cur_pipe = WindowCallbacks::getCurPipeline();
		data.dt = frameTime;
		data.camera_modPI = viewerObject.transform.modPI;

	}

} //