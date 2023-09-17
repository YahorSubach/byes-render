#include "buffer.h"
#include "command_pool.h"
#include "global.h"

render::Buffer::Buffer(const Global& global, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_flags, const std::vector<uint32_t>& queue_famaly_indeces): RenderObjBase(global), size_(size)
{
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = queue_famaly_indeces.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    buffer_info.queueFamilyIndexCount = u32(queue_famaly_indeces.size());
    buffer_info.pQueueFamilyIndices = queue_famaly_indeces.data();

    if (vkCreateBuffer(global.logical_device, &buffer_info, nullptr, &handle_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    if (handle_ == (VkBuffer)0x41862000000117e)
    {
        int a = 1;
    }

	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(global.logical_device, handle_, &memory_requirements);

    memory_ = std::make_unique<Memory>(global, (uint32_t)memory_requirements.size, memory_requirements.memoryTypeBits, memory_flags);

    vkBindBufferMemory(global.logical_device, handle_, memory_->GetMemoryHandle(), 0);
}

render::Buffer::Buffer(const Global& global, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_flags) : Buffer(global, size, usage, memory_flags, {})
{
}

size_t render::Buffer::GetSize() const
{
    return size_;
}

VkDeviceMemory render::Buffer::GetBufferMemory()
{
    return memory_->GetMemoryHandle();
}



render::Buffer::~Buffer()
{
    if (handle_ != VK_NULL_HANDLE)
    {
        if (!deferred_destroy_)
        {
            vkDestroyBuffer(global_.logical_device, handle_, nullptr);
        }
        else
        {
            if (handle_ == (VkBuffer)0x41862000000117e)
            {
                int a = 1;
            }
            global_.delete_list.push_back({ global_.frame_ind, handle_ });
        }
    }
}

void render::GPULocalBuffer::LoadData(const void* data, size_t size)
{
    StagingBuffer staging_buffer(global_, size);

    staging_buffer.LoadData(data, size);

    global_.transfer_cmd_pool->ExecuteOneTimeCommand([size, &staging_buffer, this](VkCommandBuffer command_buffer) {

        VkBufferCopy copy_region{};
        copy_region.srcOffset = 0; // Optional
        copy_region.dstOffset = 0; // Optional
        copy_region.size = size;
        vkCmdCopyBuffer(command_buffer, staging_buffer.GetHandle(), GetHandle(), 1, &copy_region);

        });
}

void render::StagingBuffer::LoadData(const void* data, size_t size)
{
    void* mapped_data;
    vkMapMemory(global_.logical_device, memory_->GetMemoryHandle(), 0, size, 0, &mapped_data);
    memcpy(mapped_data, data, static_cast<size_t>(size));
    vkUnmapMemory(global_.logical_device, memory_->GetMemoryHandle());
}

void render::UniformBuffer::LoadData(const void* data, size_t size)
{
    void* mapped_data;
    vkMapMemory(global_.logical_device, memory_->GetMemoryHandle(), 0, size, 0, &mapped_data);
    memcpy(mapped_data, data, static_cast<size_t>(size));
    vkUnmapMemory(global_.logical_device, memory_->GetMemoryHandle());
}
