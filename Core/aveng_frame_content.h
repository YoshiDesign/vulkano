#pragma once

#include "Camera/aveng_camera.h"

namespace aveng {
	struct FrameContent {

		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		AvengCamera& camera;
		VkDescriptorSet globalDescriptorSet;
		VkDescriptorSet fragDescriptorSet;
		float aspect;

	};
}