#include "AvengImageSystem.h"
#include "../aveng_model.h"
#include "../stb/stb_image.h"
#include <iostream>
#include <stdexcept>
#define LOG(a) std::cout<<a<<std::endl;

/*
	All of the helper functions that submit commands so far have been set up to execute synchronously 
	by waiting for the queue to become idle. For practical applications it is recommended to combine 
	these operations in a single command buffer and execute them asynchronously for higher throughput, 
	especially the transitions and copy in the createTextureImage function. Try to experiment with this 
	by creating a setupCommandBuffer that the helper functions record commands into, and add a 
	flushSetupCommands to execute the commands that have been recorded so far
*/

namespace aveng {

	ImageSystem::ImageSystem(EngineDevice& device, const char* filepath) : engineDevice{ device } {
		LOG("Constructing Image System");
		createTextureImage(filepath);
		createTextureImageView();
		createTextureSampler();
	}
	ImageSystem::~ImageSystem() 
	{
		vkDestroySampler(engineDevice.device(), textureSampler, nullptr);
		vkDestroyImageView(engineDevice.device(), textureImageView, nullptr);
		vkDestroyImage(engineDevice.device(), textureImage, nullptr);
		vkFreeMemory(engineDevice.device(), textureImageMemory, nullptr);
	}

	void ImageSystem::createTextureImage(const char* filepath)
	{
		LOG("Create Texture Image");
		// Load our image
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(filepath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}

		// Move the pixels into a staging buffer
		AvengBuffer stagingBuffer{
			engineDevice,
			imageSize,
			1,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			// Host Coherent bit ensures the *data buffer is flushed to the device's buffer automatically, so we dont have to call the VkFlushMappedMemoryRanges
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};
		stagingBuffer.map();
		stagingBuffer.writeToBuffer(pixels, imageSize);

		stbi_image_free(pixels);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(texWidth);
		imageInfo.extent.height = static_cast<uint32_t>(texHeight);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;	// Being sure to utilize the same image format for the texels as the pixels in the buffer, or the copy op will fail
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;	// Specifying an implementation defined ordering of the data, instead of something like row major order
		// Note: If you want to access individual texels of the image data, you need to use VK_IMAGE_TILING_LINEAR instead
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // No need to preserve texel data during the first transition to the image memory from the staging buffer
		// e.g. You're using an image as a transfer source
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;	// Destination for the buffer copy. VK_IMAGE_USAGE_SAMPLED_BIT means we'd like to be able to access the image from our shaders
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Exclusive to 1 queue family, graphics
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // > 1 if images as attachments?
		imageInfo.flags = 0; // Optional
		/*
		* TODO It is possible that the VK_FORMAT_R8G8B8A8_SRGB format is not supported by the graphics hardware. 
		* You should have a list of acceptable alternatives and go with the best one that is supported.
		*/
		if (vkCreateImage(engineDevice.device(), &imageInfo, nullptr, &textureImage) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		engineDevice.createImageWithInfo(
			imageInfo,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			textureImage,
			textureImageMemory
		);
		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		engineDevice.copyBufferToImage(stagingBuffer.getBuffer(), textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	}

	void ImageSystem::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		LOG("Transition image layout");
		// Start writing a command buffer
		VkCommandBuffer commandBuffer = engineDevice.beginSingleTimeCommands();

		/*
		* Set up an image memory barrier. This will ensure we are never
		* reading from a resource which hasn't been completely written.
		*/
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;	// These next 2 fields are used when transferring queue family ownership of the image
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;	// These would then be the indices of the queue families
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			// Undefined transfer destination : transfer writes that don't need to wait on anything
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;	// A very early stage
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			// shader reading: shader reads should wait on transfer writes, 
			// specifically the shader reads in the fragment shader, because 
			// that's where we're going to use the texture
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // A pseudostage. See https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineStageFlagBits.html
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		// Record the command to submit pipeline barriers
		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage,		// The pipeline stage which the operations occur that should happen before the barrier
			destinationStage,	// The pipeline stage in which operations will wait on the barrier
			0,					//  either 0 or VK_DEPENDENCY_BY_REGION_BIT. The latter turns the barrier into a per-region condition. That means that the implementation is allowed to already begin reading from the parts of a resource that were written so far, for example
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		/*
			The pipeline stages that you are allowed to specify before and 
			after the barrier depend on how you use the resource before and 
			after the barrier. The allowed values are listed here
			https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#synchronization-access-types-supported

			For example,  if you're going to read from a uniform after the barrier, you would 
			specify a usage of VK_ACCESS_UNIFORM_READ_BIT and the earliest shader that will 
			read from the uniform as pipeline stage, for example VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT. 
			It would not make sense to specify a non-shader pipeline stage for this type of usage and the 
			validation layers will warn you when you specify a pipeline stage that does not match the type of usage
		*/

		engineDevice.endSingleTimeCommands(commandBuffer);
	}


	void ImageSystem::createTextureImageView()
	{
		textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
	}

	VkImageView ImageSystem::createImageView(VkImage image, VkFormat format)
	{
		LOG("Create Image view for texture");
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(engineDevice.device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}

	void ImageSystem::createTextureSampler() {
		LOG("Create Texture Sampler");
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(engineDevice.physicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		if (vkCreateSampler(engineDevice.device(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	VkDescriptorImageInfo& ImageSystem::descriptorInfo()
	{
		LOG("Constructing Image Descriptor");
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;
		return imageInfo;
	}

}