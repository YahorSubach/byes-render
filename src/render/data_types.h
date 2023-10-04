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
	struct Global;

	enum class PrimitiveProps
	{
		kOpaque,
		kViewport,
		kUIShape,

		kDebugPoints,
		kDebugLines,
		kDebugPos,
	};

	using PrimitiveFlags = util::enums::Flags<PrimitiveProps>;

	enum class ExtentType
	{
		kPresentation,
		kViewport,
		kShadowMap,

		Count
	};

	enum class FormatType
	{
		kSwapchain,
		kHighRangeColor,
		kColor,
		kDepth,

		Count
	};

	const int kExtentTypeCnt = static_cast<int>(ExtentType::Count);
	const int kFormatTypeCnt = static_cast<int>(FormatType::Count);

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

	using Extents = std::array<Extent, kExtentTypeCnt>;
	using Formats = std::array<VkFormat, kFormatTypeCnt>;

	using Format = VkFormat;

	enum class ShaderType
	{
		Vertex,
		Geometry,
		Fragment,

		Invalid = -1
	};

	enum class ShaderTypeFlags : uint32_t
	{
		Empty = 0,

		Vertex = 1 << static_cast<int>(ShaderType::Vertex),
		Geometry = 1 << static_cast<int>(ShaderType::Geometry),
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
	};

	using VertexIndex = uint16_t;

	struct Face
	{
		VertexIndex indices[3];
	};

	struct OffsettedMemory
	{
		explicit operator VkDeviceMemory& () { return vk_memory; }

		VkDeviceMemory vk_memory;
		uint32_t offset;
	};
}
#endif  // RENDER_ENGINE_RENDER_DATA_TYPES_H_