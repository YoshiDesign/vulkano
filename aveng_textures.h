#pragma once

#include "EngineDevice.h"
#include "aveng_buffer.h"

namespace aveng {

	class AvengTexture {

	public:
		AvengTexture(EngineDevice& device);
		~AvengTexture();
		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		//void createTextureImage();
		//void createTextureSampler();
		//void createTextureImageView(const VkImageView swapChainImageViews);
		VkDeviceMemory textureImageMemory() { return _textureImageMemory; }
		

	private:
		VkDeviceMemory _textureImageMemory = VK_NULL_HANDLE;
		EngineDevice& engineDevice;

	};


}
