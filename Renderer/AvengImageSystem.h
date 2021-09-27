#pragma once
#include "../EngineDevice.h"
#include "Renderer.h"

/*
	All of the helper functions that submit commands so far have been set up to execute synchronously
	by waiting for the queue to become idle. For practical applications it is recommended to combine
	these operations in a single command buffer and execute them asynchronously for higher throughput,
	especially the transitions and copy in the createTextureImage function. Try to experiment with this
	by creating a setupCommandBuffer that the helper functions record commands into, and add a
	flushSetupCommands to execute the commands that have been recorded so far
*/
namespace aveng {

	class ImageSystem {

	public:

		ImageSystem(EngineDevice& device, const char* filepath);
		~ImageSystem();

		void createTextureImage(const char* filepath);
		VkImageView createImageView(VkImage image, VkFormat format);
		void createTextureImageView();
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void createTextureSampler();

		VkImage getImageInfo() { return textureImage; };
		VkDescriptorImageInfo& descriptorInfo();

	private:
		EngineDevice& engineDevice;
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler textureSampler;
	};

}