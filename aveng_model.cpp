#include "aveng_model.h"
#include <cassert>
#include <cstring>

namespace aveng {

	AvengModel::AvengModel(EngineDevice& device, const std::vector<Vertex>& vertices) 
		: engineDevice{ device } 
	{
		createVertexBuffers(vertices);
	}
	AvengModel::~AvengModel()
	{
		// Deallocation
		vkDestroyBuffer(engineDevice.device(), vertexBuffer, nullptr);
		vkFreeMemory(engineDevice.device(), vertexBufferMemory, nullptr);
	}

	// 
	void AvengModel::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");
		// Size of a vertex * number of vertices
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		engineDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			// Host Coherent bit ensures the *data buffer is flushed automatically so we dont have to call the VkFlushMappedMemoryRanges
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexBuffer,
			vertexBufferMemory
		);

		void* data;

		// Creates a region of host memory, mapping it to the device memory, and sets data to point to its first byte in vertex buffer memory
		vkMapMemory(engineDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
		// Take the vertices data and copy it into the Host-mapped memory region
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(engineDevice.device(), vertexBufferMemory);

	}

	void AvengModel::draw(VkCommandBuffer commandBuffer) 
	{
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}

	void AvengModel::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

	std::vector<VkVertexInputBindingDescription> AvengModel::Vertex::getBindingDescriptions()
	{
		// This VkVertexInputBindingDescription corresponds to a single vertex buffer
		// it will occupy the binding at index 0.
		// The stride advances at sizeof(Vertex) bytes per vertex
		/*
		    uint32_t             binding;
			uint32_t             stride;
			VkVertexInputRate    inputRate;
		*/
		return { {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX } };
	}


	std::vector<VkVertexInputAttributeDescription> AvengModel::Vertex::getAttributeDescriptions()
	{

		 /*
			uint32_t    location;	-- This specifies the location as assigned in the vertex shader i.e. layout( location = 0 )
			uint32_t    binding;
			VkFormat    format;
			uint32_t    offset;
		 */
		// return { {0, 0, VK_FORMAT_R32G32_SFLOAT, 0} };
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // Datatype - 2 components each 32bit signed floats
		attributeDescriptions[0].offset = 0;

		return attributeDescriptions;

	}

} // 