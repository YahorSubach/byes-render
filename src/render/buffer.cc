#include "buffer.h"

render::Buffer::Buffer(const VkDevice& device, const VkPhysicalDevice& physical_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_flags, const std::vector<uint32_t>& queue_famaly_indeces): RenderObjBase(device), physical_device_(physical_device)
{
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = queue_famaly_indeces.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    buffer_info.queueFamilyIndexCount = queue_famaly_indeces.size();
    buffer_info.pQueueFamilyIndices = queue_famaly_indeces.data();



    if (vkCreateBuffer(device, &buffer_info, nullptr, &buffer_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device, buffer_, &memory_requirements);

    uint32_t memory_type_index = GetMemoryTypeIndex(memory_requirements.memoryTypeBits, memory_flags);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memory_requirements.size;
    allocInfo.memoryTypeIndex = memory_type_index;

    if (vkAllocateMemory(device, &allocInfo, nullptr, &buffer_memory_) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(device, buffer_, buffer_memory_, 0);
}

VkDeviceMemory render::Buffer::GetBufferMemory()
{
    return buffer_memory_;
}

const VkBuffer& render::Buffer::GetBuffer() const
{
    return buffer_;
}

render::Buffer::~Buffer()
{
    vkDestroyBuffer(device_, buffer_, nullptr);
    vkFreeMemory(device_, buffer_memory_, nullptr);
}

uint32_t render::Buffer::GetMemoryTypeIndex(uint32_t acceptable_memory_types_bits, VkMemoryPropertyFlags memory_flags) const
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device_, &memory_properties);

    uint32_t memory_type_index = 0;

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
        if ((acceptable_memory_types_bits & (1 << i)) &&
            (memory_properties.memoryTypes[i].propertyFlags & memory_flags) == memory_flags) {
            memory_type_index = i;
        }
    }

    return memory_type_index;
}
