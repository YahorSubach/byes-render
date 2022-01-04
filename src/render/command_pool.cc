#include "command_pool.h"

render::CommandPool::CommandPool(const VkDevice & device, uint32_t queue_famaly_index): RenderObjBase(device), command_pool_(VK_NULL_HANDLE)
{
	VkCommandPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = queue_famaly_index;
	pool_info.flags = 0; // Optional

	if (vkCreateCommandPool(device, &pool_info, nullptr, &command_pool_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

bool render::CommandPool::CreateCommandBuffers(uint32_t size)
{
	if (command_buffers_.size() > 0)
	{
		vkFreeCommandBuffers(device_, command_pool_, static_cast<uint32_t>(command_buffers_.size()), command_buffers_.data());
		command_buffers_.clear();
	}

	command_buffers_.resize(size);

	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = command_pool_;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = size;

	if (vkAllocateCommandBuffers(device_, &alloc_info, command_buffers_.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	return true;
}

void render::CommandPool::ClearCommandBuffers()
{
	if (command_buffers_.size() > 0)
	{
		vkFreeCommandBuffers(device_, command_pool_, static_cast<uint32_t>(command_buffers_.size()), command_buffers_.data());
		command_buffers_.clear();
	}
}

const VkCommandPool & render::CommandPool::GetCommandPool() const
{
	return command_pool_;
}

const VkCommandBuffer & render::CommandPool::GetCommandBuffer(size_t index) const
{
	return command_buffers_[index];
}

render::CommandPool::~CommandPool()
{
	if (command_pool_ != VK_NULL_HANDLE)
	{
		if (command_buffers_.size() > 0)
		{
			vkFreeCommandBuffers(device_, command_pool_, static_cast<uint32_t>(command_buffers_.size()), command_buffers_.data());
			command_buffers_.clear();
		}

		vkDestroyCommandPool(device_, command_pool_, nullptr);
	}
}
