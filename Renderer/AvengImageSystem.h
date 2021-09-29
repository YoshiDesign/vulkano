#pragma once
#include "../EngineDevice.h"
#include "Renderer.h"

#include "../stb/stb_image.h"

#include <unordered_map>
#include <vector>

/*
	All of the helper functions that submit commands so far have been set up to execute synchronously
	by waiting for the queue to become idle. For practical applications it is recommended to combine
	these operations in a single command buffer and execute them asynchronously for higher throughput,
	especially the transitions and copy in the createTextureImage function. Try to experiment with this
	by creating a setupCommandBuffer that the helper functions record commands into, and add a
	flushSetupCommands to execute the commands that have been recorded so far
*/
namespace aveng {

	struct Texture {
		stbi_uc* pixels;
		int texWidth;
		int texHeight;
		int texChannels;

	};

	class ImageSystem {

	public:

		ImageSystem(EngineDevice& device);
		~ImageSystem();

		void createTextureImage();
		VkImageView createImageView(VkImage image, VkFormat format, uint32_t mipLevels);
		void createTextureImageView();
		void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void createTextureSampler();

		// void loadTextureFromFile(const char* filepath);
		// Texture getTexture(std::string name) { return textures[name]; }

		VkImage getImageInfo() { return image; };
		VkDescriptorImageInfo& descriptorInfo();

	private:



		// mipLevels is the only dependency keeping us from merging some of these functions with SwapChain's impelmentations of them
		uint32_t mipLevels{0};

		EngineDevice& engineDevice;

		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler textureSampler;
		VkImage image;
		std::vector<VkImage> images;
		std::unordered_map<std::string, Texture> textures;
	};

}