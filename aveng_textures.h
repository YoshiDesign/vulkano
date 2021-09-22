#pragma once

#include "EngineDevice.h"
#include "Renderer/Renderer.h"
#include "aveng_buffer.h"

namespace aveng {

	class AvengTexture {

	public:
		AvengTexture(EngineDevice& device, Renderer& renderer);
		~AvengTexture();
		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void createTextureImage();
		void createTextureImageView(const VkImageView swapChainImageViews);

	private:
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler textureSampler;
		EngineDevice& engineDevice;
		Renderer& renderer;

	};


}
