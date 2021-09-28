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

		ImageSystem(EngineDevice& device);
		~ImageSystem();

		void createTextureImage(const char* filepath);
		VkImageView createImageView(VkImage image, VkFormat format, uint32_t mipLevels);
		void createTextureImageView();
		void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void createTextureSampler();

		VkImage getImageInfo() { return textureImage; };
		VkDescriptorImageInfo& descriptorInfo();

	private:

		// mipLevels is the only dependency keeping us from merging some of these functions with SwapChain's impelmentations of them
		uint32_t mipLevels{0};

		EngineDevice& engineDevice;
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler textureSampler;
	};

}