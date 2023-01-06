#ifndef RENDER_ENGINE_RENDER_VERTEX_BUFFER_H_
#define RENDER_ENGINE_RENDER_VERTEX_BUFFER_H_

#include <vector>
#include <array>
#include <map>

#include "vulkan/vulkan.h"
#include "glm/glm/glm.hpp"

#include "common.h"
#include "render/object_base.h"

namespace render
{

	enum class VertexBufferType
	{

#define ENUM_OP(val) k##val, 
#include "render/vertex_buffer_types.inl"
#undef ENUM_OP

		Count,

		Begin = kPOSITION,
		End = Count
	};

	const uint32_t kVertexBufferTypesCount = static_cast<uint32_t>(VertexBufferType::Count);

	const std::map<VertexBufferType, std::string>& GetVertexBufferTypesToNames();
	const std::map<std::string, VertexBufferType>& GetVertexBufferNamesToTypes();

	class VertexBuffer : public RenderObjBase<VkBuffer>
	{
	public:
		VertexBuffer(const VkDevice& device, const VkPhysicalDevice& physical_device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& queue_famaly_indeces);

		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer(VertexBuffer&&) = default;

		VertexBuffer& operator=(const VertexBuffer&) = delete;
		VertexBuffer& operator=(VertexBuffer&&) = default;

		void ClearCommandBuffers();

		const VkBuffer& GetVertexBuffer() const;
		uint32_t GetVertexNum() const;

		~VertexBuffer();
	private:

		VkPhysicalDevice physical_device_;

		uint32_t GetMemoryTypeIndex(uint32_t acceptable_memory_types_bits, VkMemoryPropertyFlags memory_flags) const;

		uint32_t vertex_num_;
		VkBuffer vertex_buffer_;
		VkDeviceMemory vertex_buffer_memory_;
	};
}
#endif  // RENDER_ENGINE_RENDER_VERTEX_BUFFER_H_