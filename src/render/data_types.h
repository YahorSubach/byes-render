#ifndef RENDER_ENGINE_RENDER_DATA_TYPES_H_
#define RENDER_ENGINE_RENDER_DATA_TYPES_H_

#include <vector>
#include <array>

#include "vulkan/vulkan.h"
#include "glm/glm/glm.hpp"

#include "common.h"
#include "render/object_base.h"

namespace render
{
	class CommandPool;

	struct DeviceConfiguration
	{
		VkPhysicalDevice physical_device;
		VkPhysicalDeviceProperties physical_device_properties;
		
		VkDevice  logical_device;
		
		VkQueue transfer_queue;
		uint32_t transfer_queue_index;

		VkQueue graphics_queue;
		uint32_t graphics_queue_index;

		CommandPool* graphics_cmd_pool;
		CommandPool* transfer_cmd_pool;
	};


	struct Vertex {
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 tex_coord;

		static VkVertexInputBindingDescription GetBindingDescription()
		{
			VkVertexInputBindingDescription binding_description{};

			binding_description.binding = 0;
			binding_description.stride = sizeof(Vertex);
			binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return binding_description;
		}

		static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions{};

			attribute_descriptions[0].binding = 0;
			attribute_descriptions[0].location = 0;
			attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // we can use more component but they will be discarted
			attribute_descriptions[0].offset = offsetof(Vertex, pos);

			attribute_descriptions[1].binding = 0;
			attribute_descriptions[1].location = 1;
			attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions[1].offset = offsetof(Vertex, color);

			attribute_descriptions[2].binding = 0;
			attribute_descriptions[2].location = 2;
			attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attribute_descriptions[2].offset = offsetof(Vertex, tex_coord);

			return attribute_descriptions;
		}
	};

	using VertexIndex = uint16_t;

	struct Face
	{
		VertexIndex indices[3];
	};

	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};
}
#endif  // RENDER_ENGINE_RENDER_DATA_TYPES_H_