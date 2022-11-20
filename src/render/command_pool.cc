#include "command_pool.h"

render::CommandPool::CommandPool(const DeviceConfiguration& device_cfg, PoolType pool_type): RenderObjBase(device_cfg), next_available_buffer_(0)
{
	VkCommandPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.flags = 0; // Optional

	if (pool_type == PoolType::kGraphics)
	{
		pool_info.queueFamilyIndex = device_cfg.graphics_queue_index;
		pool_queue_ = device_cfg.graphics_queue;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	}
	else if (pool_type == PoolType::kTransfer)
	{
		pool_info.queueFamilyIndex = device_cfg.transfer_queue_index;
		pool_queue_ = device_cfg.transfer_queue;
	}



	if (vkCreateCommandPool(device_cfg_.logical_device, &pool_info, nullptr, &handle_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

bool render::CommandPool::CreateCommandBuffers(uint32_t size)
{
	if (command_buffers_.size() > 0)
	{
		vkFreeCommandBuffers(device_cfg_.logical_device, handle_, static_cast<uint32_t>(command_buffers_.size()), command_buffers_.data());
		command_buffers_.clear();
	}

	command_buffers_.resize(size);

	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = handle_;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = size;

	if (vkAllocateCommandBuffers(device_cfg_.logical_device, &alloc_info, command_buffers_.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	return true;
}

VkCommandBuffer render::CommandPool::GetCommandBuffer()
{
	return command_buffers_[next_available_buffer_++];
}

void render::CommandPool::Reset()
{
	next_available_buffer_ = 0;
}

void render::CommandPool::ClearCommandBuffers()
{
	if (command_buffers_.size() > 0)
	{
		vkFreeCommandBuffers(device_cfg_.logical_device, handle_, static_cast<uint32_t>(command_buffers_.size()), command_buffers_.data());
		command_buffers_.clear();
		next_available_buffer_ = 0;
	}
}


const VkCommandBuffer & render::CommandPool::GetCommandBuffer(size_t index) const
{
	return command_buffers_[index];
}

void render::CommandPool::ExecuteOneTimeCommand(const std::function<void(VkCommandBuffer)>& command) const
{
	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool = handle_;
	alloc_info.commandBufferCount = 1;

	VkCommandBuffer command_buffer;
	VkResult res = vkAllocateCommandBuffers(device_cfg_.logical_device, &alloc_info, &command_buffer);

	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(command_buffer, &begin_info);

	command(command_buffer);

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}

	vkQueueSubmit(pool_queue_, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(pool_queue_);

	vkFreeCommandBuffers(device_cfg_.logical_device, handle_, 1, &command_buffer);
}

render::CommandPool::~CommandPool()
{
	if (handle_ != VK_NULL_HANDLE)
	{
		if (command_buffers_.size() > 0)
		{
			vkFreeCommandBuffers(device_cfg_.logical_device, handle_, static_cast<uint32_t>(command_buffers_.size()), command_buffers_.data());
			command_buffers_.clear();
		}

		vkDestroyCommandPool(device_cfg_.logical_device, handle_, nullptr);
	}
}
