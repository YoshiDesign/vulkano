#pragma once

#include "EngineDevice.h"
#include "aveng_buffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace aveng {

	class AvengModel 
	{

		EngineDevice& engineDevice;
		uint32_t vertexCount;
		bool hasIndexBuffer = false;
		uint32_t indexCount;

		/*VkBuffer vertexBuffer;		OLD
		VkDeviceMemory vertexBufferMemory;*/
		// NEW
		std::unique_ptr<AvengBuffer> vertexBuffer;

		/*VkBuffer indexBuffer;			OLD
		VkDeviceMemory indexBufferMemory;*/
		// NEW
		std::unique_ptr<AvengBuffer> indexBuffer;

	public:

		struct Vertex {
			glm::vec3 position;
			glm::vec3 color;
			glm::vec3 normal{}; // surface norms I guess
			glm::vec2 uv{};		// 2d texture coordinates

			/*
			* Descriptions of our vertext input so that our shaders
			* may receive communications from our GFXPipeline and understand
			* how to parse the incoming data
			*/
			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator==(const Vertex& other) const 
			{
				return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
			}

		};

		// Vertex and index information to be sent to the model's vertex and index buffer memory
		struct Builder {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void loadModel(const std::string& filepath);
		};

		AvengModel(EngineDevice& device, const AvengModel::Builder& builder);
		~AvengModel();

		AvengModel(const AvengModel&) = delete;
		AvengModel& operator=(const AvengModel&) = delete;

		static std::unique_ptr<AvengModel> createModelFromFile(EngineDevice& device, const std::string& filepath);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
	
	private:

		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);


	};

} //