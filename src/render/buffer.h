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

		uint64_t GetSize() const;

		VkDeviceMemory GetBufferMemory();

		virtual void LoadData(const void* data, size_t size);

		virtual ~Buffer() override;
	private:
		std::unique_ptr<Memory> memory_;
		uint64_t size_;
	};

	struct BufferAccessor
	{
		BufferAccessor(const Buffer& buffer, uint32_t stride, uint64_t offset, uint64_t count) :buffer(&buffer), stride(stride), offset(offset), count(count) {}
		BufferAccessor(const Buffer& buffer) :BufferAccessor(buffer, 0, 0, buffer.GetSize()) {}

		BufferAccessor() :buffer(nullptr), stride(0), offset(0), count(0) {}


		const Buffer* buffer;
		uint32_t stride;

		uint64_t offset;
		uint64_t count;
	};

	class GPULocalBuffer : public Buffer
	{
	public:
		GPULocalBuffer(const DeviceConfiguration& device_cfg, VkDeviceSize size, VkBufferUsageFlags usage, const std::vector<uint32_t>& queue_famaly_indices) :
			Buffer(device_cfg, size, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, queue_famaly_indices) {}

		virtual void LoadData(const void* data, size_t size) override;
	};

	class UniformBuffer : public Buffer
	{
	public:


		UniformBuffer(const DeviceConfiguration& device_cfg, VkDeviceSize size) :
			Buffer(device_cfg, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {}) {}

		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer(UniformBuffer&&) = default;

		UniformBuffer& operator=(const UniformBuffer&) = delete;
		UniformBuffer& operator=(UniformBuffer&&) = default;
	};
}
#endif  // RENDER_ENGINE_RENDER_VERTEX_BUFFER_H_