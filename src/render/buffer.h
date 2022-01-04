#ifndef RENDER_ENGINE_RENDER_VERTEX_BUFFER_H_
#define RENDER_ENGINE_RENDER_VERTEX_BUFFER_H_

#include <vector>
#include <array>

#include "vulkan/vulkan.h"
#include "glm/glm/glm.hpp"

#include "common.h"
#include "render/object_base.h"

namespace render
{
	class Buffer : public RenderObjBase
	{
	public:
		Buffer(const VkDevice& device, const VkPhysicalDevice& physical_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_flags, const std::vector<uint32_t>& queue_famaly_indices);

		Buffer(const Buffer&) = delete;
		Buffer(Buffer&&) = default;

		Buffer& operator=(const Buffer&) = delete;
		Buffer& operator=(Buffer&&) = default;

		VkDeviceMemory GetBufferMemory();

		const VkBuffer& GetBuffer() const;
		uint32_t GetVertexNum() const;

		~Buffer();
	private:

		VkPhysicalDevice physical_device_;

		uint32_t GetMemoryTypeIndex(uint32_t acceptable_memory_types_bits, VkMemoryPropertyFlags memory_flags) const;

		VkBuffer buffer_;
		VkDeviceMemory buffer_memory_;
	};
}
#endif  // RENDER_ENGINE_RENDER_VERTEX_BUFFER_H_