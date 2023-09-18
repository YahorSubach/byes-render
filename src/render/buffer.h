#ifndef RENDER_ENGINE_RENDER_BUFFER_H_
#define RENDER_ENGINE_RENDER_BUFFER_H_

#include <vector>
#include <array>
#include <optional>

#include "vulkan/vulkan.h"

#include "byes-reference-to-movable\reference_to_movable.h"

#include "stl_util.h"
#include "common.h"
#include "render/object_base.h"
#include "render/data_types.h"
#include "render/memory.h"

namespace render
{
	class Buffer : public byes::RM<Buffer>, public RenderObjBase<VkBuffer>
	{
	public:

		Buffer(const Global& global, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_flags, const std::vector<uint32_t>& queue_famaly_indices, bool deferred_destroy = true);
		Buffer(const Global& global, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_flags);

		Buffer(const Buffer&) = delete;
		Buffer(Buffer&&) = default;

		Buffer& operator=(const Buffer&) = delete;
		Buffer& operator=(Buffer&&) = default;

		size_t GetSize() const;

		OffsettedMemory GetBufferMemory();

		//virtual void LoadData(const void* data, size_t size) = 0;

		virtual ~Buffer() override;
	protected:
		std::unique_ptr<Memory> memory_;
		size_t size_;
		bool deferred_destroy_ = true;
	};

	template<typename T>
	struct ElemType {};

	struct BufferAccessor
	{
		BufferAccessor(const Buffer& buffer, size_t stride, size_t offset, size_t count) :buffer(buffer), stride(stride), offset(offset), count(count) {}
		//fferAccessor(BufferAccessor&&) = default;
		//BufferAccessor(const Buffer& buffer) :BufferAccessor(buffer, 0, 0, buffer.GetSize()) {}

		//BufferAccessor() :stride(0), offset(0), count(0) {}

		template<typename T>
		BufferAccessor(ElemType<T>, const Buffer& buffer) : BufferAccessor(buffer, sizeof(T), 0, buffer.GetSize() / sizeof(T)) {}

		byes::RTM<const Buffer> buffer;
		const size_t stride;

		const size_t offset;
		size_t count;
	};

	class HostVisibleBuffer : public Buffer
	{
	public:
		HostVisibleBuffer(const Global& global, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_flags = 0, const std::vector<uint32_t>& queue_famaly_indices = {}, bool deferred_destroy = true) :
			Buffer(global, size, usage, memory_flags | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, queue_famaly_indices, deferred_destroy)
		{}

		void LoadData(const void* data, size_t size);
	};

	class StagingBuffer : public HostVisibleBuffer
	{
	public:
		StagingBuffer(const Global& global, VkDeviceSize size, VkBufferUsageFlags usage = 0, const std::vector<uint32_t>& queue_famaly_indices = {}) :
			HostVisibleBuffer(global, size, usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, queue_famaly_indices, false)
		{}
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

	class UniformBuffer : public HostVisibleBuffer
	{
	public:


		UniformBuffer(const Global& global, VkDeviceSize size) : HostVisibleBuffer(global, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {}
		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer(UniformBuffer&&) = default;

		UniformBuffer& operator=(const UniformBuffer&) = delete;
		UniformBuffer& operator=(UniformBuffer&&) = default;
	};
}
#endif  // RENDER_ENGINE_RENDER_BUFFER_H_