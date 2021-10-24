#include "vertex_buffer.h"

render::VertexBuffer::VertexBuffer(const VkDevice& device, const VkPhysicalDevice& physical_device, const std::vector<Vertex>& vertices) :RenderObjBase(device)
{
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = sizeof(vertices[0]) * vertices.size();
    buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &buffer_info, nullptr, &vertex_buffer_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device, vertex_buffer_, &memory_requirements);

    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

    uint32_t memory_type_index;

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
        if ((memory_requirements.memoryTypeBits & (1 << i) ) && 
            (memory_properties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))) {
            memory_type_index = i;
        }
    }

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memory_requirements.size;
    allocInfo.memoryTypeIndex = memory_type_index;

    if (vkAllocateMemory(device, &allocInfo, nullptr, &vertex_buffer_memory_) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(device, vertex_buffer_, vertex_buffer_memory_, 0);

    void* data;
    vkMapMemory(device, vertex_buffer_memory_, 0, buffer_info.size, 0, &data);

    memcpy(data, vertices.data(), (size_t)buffer_info.size);

    vkUnmapMemory(device, vertex_buffer_memory_);

    vertex_num_ = vertices.size();
}

void render::VertexBuffer::ClearCommandBuffers()
{
}

const VkBuffer& render::VertexBuffer::GetVertexBuffer() const
{
    return vertex_buffer_;
}

uint32_t render::VertexBuffer::GetVertexNum() const
{
    return vertex_num_;
}

render::VertexBuffer::~VertexBuffer()
{
    vkDestroyBuffer(device_, vertex_buffer_, nullptr);
    vkFreeMemory(device_, vertex_buffer_memory_, nullptr);
}
