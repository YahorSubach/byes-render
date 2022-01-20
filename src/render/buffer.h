#ifndef RENDER_ENGINE_RENDER_VERTEX_BUFFER_H_
#define RENDER_ENGINE_RENDER_VERTEX_BUFFER_H_

#include <vector>
#include <array>

#include "vulkan/vulkan.h"

#include "common.h"
#include "render/object_base.h"
#include "render/data_types.h"
#include "render/memory.h"

namespace render
{
	class Buffer : public RenderObjBase<VkBuffer>
	{
	public:
		Buffer(const DeviceConfiguration& device_cfg, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_flags, const std::vector<uint32_t>& queue_famaly_indices);

		Buffer(const Buffer&) = delete;
		Buffer(Buffer&&) = default;

		Buffer& operator=(const Buffer&) = delete;
		Buffer& operator=(Buffer&&) = default;

		VkDeviceMemory GetBufferMemory();

		void LoadData(const void* data, size_t size);

		virtual ~Buffer() override;
	private:
		std::unique_ptr<Memory> memory_;
	};
}
#endif  // RENDER_ENGINE_RENDER_VERTEX_BUFFER_H_