#ifndef RENDER_ENGINE_RENDER_DATA_TYPES_H_
#define RENDER_ENGINE_RENDER_DATA_TYPES_H_

#include <vector>
#include <array>

#include "vulkan/vulkan.h"
#include "glm/glm/glm.hpp"

#include "common.h"
#include "stl_util.h"

namespace render
{

	class CommandPool;
	class DescriptorPool;
	class Sampler;
	class Image;

	enum class ExtentType
	{
		kPresentation,
		kViewport,
		kShadowMap,

		Count
	};

	const int kExtentTypeCnt = static_cast<int>(ExtentType::Count);

	enum class PipelineId
	{
		kColor,
		kColorSkinned,
		kDepth,
		kDepthSkinned,
		kUI,

		kBuildGBuffers,
		kCollectGBuffers,

		kDebugLines
	};

	enum class RenderPassId
	{
		kSimpleRenderToScreen,
		kBuildDepthmap,
		kBuildGBuffers,
		kCollectGBuffers,

		kUI,
	};

	struct Extent
	{
		uint32_t width;
		uint32_t height;

		bool operator==(const Extent& rhs) const { return (width == rhs.width) && (height == rhs.height); }

		Extent() = default;
		Extent(VkExtent2D vk_ext) : width(vk_ext.width), height(vk_ext.height) {}
		Extent(uint32_t width, uint32_t height) : width(width), height(height) {}
		operator VkExtent2D() const { return VkExtent2D{ width , height }; }
	};


	using Format = VkFormat;

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

		Sampler* texture_sampler;
		Sampler* shadowmap_sampler;

		stl_util::NullableRef<Image> default_image;

		Format presentation_format;
		Format depth_map_format = VK_FORMAT_D32_SFLOAT;
		Format high_range_color_format = VK_FORMAT_R32G32B32A32_SFLOAT;
		Format color_format = VK_FORMAT_B8G8R8A8_SRGB;
	};

	enum class ShaderType
	{
		Vertex,
		Fragment,

		Invalid = -1
	};

	enum class ShaderTypeFlags : uint32_t
	{
		Empty = 0,

		Vertex = 1 << static_cast<int>(ShaderType::Vertex),
		Fragment = 1 << static_cast<int>(ShaderType::Fragment),
	};

	constexpr ShaderTypeFlags operator|(ShaderTypeFlags lhs, ShaderTypeFlags rhs) {
		return static_cast<ShaderTypeFlags>(
			static_cast<std::underlying_type_t<ShaderTypeFlags>>(lhs) |
			static_cast<std::underlying_type_t<ShaderTypeFlags>>(rhs)
			);
	}

	constexpr ShaderTypeFlags operator&(ShaderTypeFlags lhs, ShaderTypeFlags rhs) {
		return static_cast<ShaderTypeFlags>(
			static_cast<std::underlying_type_t<ShaderTypeFlags>>(lhs) &
			static_cast<std::underlying_type_t<ShaderTypeFlags>>(rhs)
			);
	}

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

	template<typename T>
	uint32_t u32(T t) { return static_cast<uint32_t>(t); }

	//struct UniformBufferObject {
	//	glm::mat4 model;
	//	glm::mat4 view;
	//	glm::mat4 proj;
	//};
}
#endif  // RENDER_ENGINE_RENDER_DATA_TYPES_H_