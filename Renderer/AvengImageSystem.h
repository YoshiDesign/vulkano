#pragma once
#include "../EngineDevice.h"
#include "Renderer.h"

#include "../stb/stb_image.h"

#include <iostream>
#include <unordered_map>
#include <vector>

#define INFO(ln, sep) std::cout << "AvengImageSystem.cpp::" << ln << ":\n" << sep;
#define DBUG(x) std::cout << x << std::endl;

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

		void createTextureImage(const char* filepath, size_t i);
		VkImageView createImageView(VkImage image, VkFormat format, uint32_t mipLevels);
		void createTextureImageView(VkImage image, size_t i);
		void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void createTextureSampler();

		void createImageDescriptors(std::vector<VkImageView> views);
		VkDescriptorImageInfo getImageInfoAtIndex(int index) { return imageInfosArray[index]; };
		std::vector<VkDescriptorImageInfo> descriptorInfoForAllImages()
		{ 
			return imageInfosArray; 
		}

		int nImages = 4;
	private:

		// mipLevels is one dependency keeping us from merging some of these functions with SwapChain's impelmentations of them

		EngineDevice& engineDevice;

		std::vector<uint32_t> mipLevels;
		std::vector<VkDescriptorImageInfo> imageInfosArray;

		std::vector<VkDeviceMemory> allImageMemory;
		VkSampler textureSampler;
		
		std::vector<VkImageView> textureImageViews;
		std::vector<VkImage> images;
		std::unordered_map<std::string, Texture> textures;
	};

}