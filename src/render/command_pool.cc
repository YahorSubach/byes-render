#include "command_pool.h"

render::CommandPool::CommandPool(const VkDevice & device, uint32_t queue_index): RenderObjBase(device), command_pool_(VK_NULL_HANDLE)
{
	VkCommandPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = queue_index;
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

bool render::CommandPool::FillCommandBuffer(size_t index, const GraphicsPipeline& pipeline, const VkExtent2D& extent, const RenderPass& render_pass, const Framebuffer& framebuffer)
{
	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Optional
	begin_info.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(command_buffers_[index], &begin_info) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = render_pass.GetRenderPassHandle();
	render_pass_info.framebuffer = framebuffer.GetFramebufferHandle();

	render_pass_info.renderArea.offset = { 0, 0 };
	render_pass_info.renderArea.extent = extent;

	VkClearValue clear_color = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	render_pass_info.clearValueCount = 1;
	render_pass_info.pClearValues = &clear_color;

	vkCmdBeginRenderPass(command_buffers_[index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(command_buffers_[index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipelineHandle());
	vkCmdDraw(command_buffers_[index], 3, 1, 0, 0);

	vkCmdEndRenderPass(command_buffers_[index]);

	if (vkEndCommandBuffer(command_buffers_[index]) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}

	return false;
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
