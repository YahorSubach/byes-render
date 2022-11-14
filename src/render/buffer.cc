#include "buffer.h"
#include "command_pool.h"

render::Buffer::Buffer(const DeviceConfiguration& device_cfg, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_flags, const std::vector<uint32_t>& queue_famaly_indeces): RenderObjBase(device_cfg), size_(size)
{
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = queue_famaly_indeces.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    buffer_info.queueFamilyIndexCount = u32(queue_famaly_indeces.size());
    buffer_info.pQueueFamilyIndices = queue_famaly_indeces.data();

    if (vkCreateBuffer(device_cfg.logical_device, &buffer_info, nullptr, &handle_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(device_cfg.logical_device, handle_, &memory_requirements);

    memory_ = std::make_unique<Memory>(device_cfg, memory_requirements.size, memory_requirements.memoryTypeBits, memory_flags);

    vkBindBufferMemory(device_cfg.logical_device, handle_, memory_->GetMemoryHandle(), 0);
}

render::Buffer::Buffer(const DeviceConfiguration& device_cfg, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_flags) : Buffer(device_cfg, size, usage, memory_flags, {})
{
}

uint64_t render::Buffer::GetSize() const
{
    return size_;
}

VkDeviceMemory render::Buffer::GetBufferMemory()
{
    return memory_->GetMemoryHandle();
}

void render::Buffer::LoadData(const void* data, size_t size)
{
	//TODO add checking buffer type

    void* mapped_data;
    vkMapMemory(device_cfg_.logical_device, memory_->GetMemoryHandle(), 0, size, 0, &mapped_data);
    memcpy(mapped_data, data, static_cast<size_t>(size));
    vkUnmapMemory(device_cfg_.logical_device, memory_->GetMemoryHandle());
}

render::Buffer::~Buffer()
{
    if (handle_ != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device_cfg_.logical_device, handle_, nullptr);
    }
}

void render::GPULocalBuffer::LoadData(const void* data, size_t size)
{
    Buffer staging_buffer(device_cfg_, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {});

    staging_buffer.LoadData(data, size);

    device_cfg_.transfer_cmd_pool->ExecuteOneTimeCommand([size, &staging_buffer, this](VkCommandBuffer command_buffer) {

        VkBufferCopy copy_region{};
        copy_region.srcOffset = 0; // Optional
        copy_region.dstOffset = 0; // Optional
        copy_region.size = size;
        vkCmdCopyBuffer(command_buffer, staging_buffer.GetHandle(), GetHandle(), 1, &copy_region);

        });
}
