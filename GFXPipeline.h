#pragma once

#include "EngineDevice.hpp"

#include <string>
#include <vector>

/**
* @class GFXPipeline
* Used to implement our precompiled shaders
*/
namespace aveng {

	// This struct is used by our Application code to be able to 
	// completely configure our graphics pipeline config.
	// For this reason, we include it here, instead of in GFXPipeline
	// This way we can share this config between multiple pipelines, too
	struct PipelineConfig {
		VkViewport viewport;
		VkRect2D scissor;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;

		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};
	
	/**
	* The Vulkan Graphics Pipeline implementation
	*/
	class GFXPipeline {
		// @warn - A reference member var to our engine device. If we have a GFX Pipeline, the necessity of an engine device is implicit.
		// You should never copy a GFXPipeline
		EngineDevice& engDevice;
		VkPipeline graphicsPipeline;		// typedef pointer
		VkShaderModule vertShaderModule;	// typedef pointer
		VkShaderModule fragShaderModule;	// typedef pointer

	public:

		GFXPipeline(
			EngineDevice& device, 
			const std::string& vertFilepath, 
			const std::string& fragFilepath, 
			const PipelineConfig& config
		);

		~GFXPipeline();
		GFXPipeline(const GFXPipeline&) = delete;
		void operator=(const GFXPipeline&) = delete;

		static PipelineConfig defaultPipelineConfig(uint32_t width, uint32_t height);

	private:

		static std::vector<char> readFile(const std::string& filepath);

		void createGFXPipeline(
			const std::string& vertFilepath, 
			const std::string& fragFilepath,
			const PipelineConfig& config
		);

		// Used to create module and init variable for shaders
		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

	};


} // NS