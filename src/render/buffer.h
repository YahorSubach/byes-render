#ifndef RENDER_ENGINE_RENDER_BUFFER_H_
#define RENDER_ENGINE_RENDER_BUFFER_H_

#include <vector>
#include <array>
#include <optional>

#include "vulkan/vulkan.h"

#include "stl_util.h"
#include "common.h"
#include "render/object_base.h"
#include "render/data_types.h"
#include "render/memory.h"

namespace render
{
	class Buffer : public RenderObjBase<VkBuffer>
	{
	public:
		Buffer(const Global& global, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_flags, const std::vector<uint32_t>& queue_famaly_indices);
		Buffer(const Global& global, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_flags);

		Buffer(const Buffer&) = delete;
		Buffer(Buffer&&) = default;

		Buffer& operator=(const Buffer&) = delete;
		Buffer& operator=(Buffer&&) = default;

		size_t GetSize() const;

		VkDeviceMemory GetBufferMemory();

		//virtual void LoadData(const void* data, size_t size) = 0;

		virtual ~Buffer() override;
	protected:
		std::unique_ptr<Memory> memory_;
		size_t size_;
	};

	struct BufferAccessor
	{
		BufferAccessor(const Buffer& buffer, size_t stride, size_t offset, size_t count) :buffer(buffer), stride(stride), offset(offset), count(count) {}
		//fferAccessor(BufferAccessor&&) = default;
		//BufferAccessor(const Buffer& buffer) :BufferAccessor(buffer, 0, 0, buffer.GetSize()) {}

		//BufferAccessor() :stride(0), offset(0), count(0) {}

		template<typename T>
		BufferAccessor(const Buffer& buffer) : BufferAccessor(buffer, sizeof(T), 0, buffer.GetSize() / sizeof(T)) {}

		const util::NullableRef<const Buffer> buffer;
		const size_t stride;

		const size_t offset;
		size_t count;
	};

	class StagingBuffer : public Buffer
	{
	public:
		StagingBuffer(const Global& global, VkDeviceSize size, VkBufferUsageFlags usage = 0, const std::vector<uint32_t>& queue_famaly_indices = {}) :
			Buffer(global, size, usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, queue_famaly_indices) {}

		virtual void LoadData(const void* data, size_t size);
	};

	class GPULocalBuffer : public Buffer
	{
	public:
		GPULocalBuffer(const Global& global, VkDeviceSize size, VkBufferUsageFlags usage = 0, const std::vector<uint32_t>& queue_famaly_indices = {}) :
			Buffer(global, size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, queue_famaly_indices) {}

		virtual void LoadData(const void* data, size_t size);
	};

	class GPULocalVertexBuffer : public GPULocalBuffer
	{
	public:
		GPULocalVertexBuffer(const Global& global, VkDeviceSize size) :
			GPULocalBuffer(global, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {}
	};

	class UniformBuffer : public Buffer
	{
	public:


		UniformBuffer(const Global& global, VkDeviceSize size) :
			Buffer(global, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {}) {}

		virtual void LoadData(const void* data, size_t size);

		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer(UniformBuffer&&) = default;

		UniformBuffer& operator=(const UniformBuffer&) = delete;
		UniformBuffer& operator=(UniformBuffer&&) = default;
	};
}
#endif  // RENDER_ENGINE_RENDER_BUFFER_H_