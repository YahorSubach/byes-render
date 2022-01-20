#include "buffer.h"

render::Buffer::Buffer(const DeviceConfiguration& device_cfg, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_flags, const std::vector<uint32_t>& queue_famaly_indeces): RenderObjBase(device_cfg.logical_device)
{
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = queue_famaly_indeces.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    buffer_info.queueFamilyIndexCount = queue_famaly_indeces.size();
    buffer_info.pQueueFamilyIndices = queue_famaly_indeces.data();

    if (vkCreateBuffer(device_cfg.logical_device, &buffer_info, nullptr, &handle_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(device_cfg.logical_device, handle_, &memory_requirements);

    memory_ = std::make_unique<Memory>(device_cfg, memory_requirements.size, memory_requirements.memoryTypeBits);

    vkBindBufferMemory(device_cfg.logical_device, handle_, memory_->GetMemoryHandle(), 0);
}

VkDeviceMemory render::Buffer::GetBufferMemory()
{
    return memory_->GetMemoryHandle();
}

void render::Buffer::LoadData(const void* data, size_t size)
{
    void* mapped_data;
    vkMapMemory(device_, memory_->GetMemoryHandle(), 0, size, 0, &mapped_data);
    memcpy(mapped_data, data, static_cast<size_t>(size));
    vkUnmapMemory(device_, memory_->GetMemoryHandle());
}

render::Buffer::~Buffer()
{
    vkDestroyBuffer(device_, handle_, nullptr);
}
