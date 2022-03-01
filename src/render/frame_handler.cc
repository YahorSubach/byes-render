#include "frame_handler.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <utility>

#include "vk_util.h"
#include <render/data_types.h>

render::FrameHandler::FrameHandler(const DeviceConfiguration& device_cfg, const Swapchain& swapchain) :
	RenderObjBase(device_cfg), swapchain_(swapchain.GetHandle()), graphics_queue_(device_cfg.graphics_queue),
	command_buffer_(device_cfg.graphics_cmd_pool->GetCommandBuffer()),
	image_available_semaphore_(vk_util::CreateSemaphore(device_cfg.logical_device)),
	render_finished_semaphore_(vk_util::CreateSemaphore(device_cfg.logical_device)),
	cmd_buffer_fence_(vk_util::CreateFence(device_cfg.logical_device)), present_info_{}, submit_info_{}, wait_stages_(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT),
	color_sampler_(device_cfg)
{

	handle_ = (void*)(1);
}

bool render::FrameHandler::FillCommandBuffer(const Framebuffer& swapchain_framebuffer, uint32_t swapchain_image_index, const PipelineCollection& pipeline_collection, const BatchesManager& batches_manager)
{
	vkResetCommandBuffer(command_buffer_, 0);

	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Optional
	begin_info.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(command_buffer_, &begin_info) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = pipeline_collection.GetRenderPass().GetHandle();
	render_pass_info.framebuffer = swapchain_framebuffer.GetHandle();

	render_pass_info.renderArea.offset = { 0, 0 };
	render_pass_info.renderArea.extent = swapchain_framebuffer.GetExtent();

	std::array<VkClearValue, 2> clear_color;
	clear_color[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clear_color[1].depthStencil = { 1.0f, 0 };
	render_pass_info.clearValueCount = clear_color.size();
	render_pass_info.pClearValues = clear_color.data();

	vkCmdBeginRenderPass(command_buffer_, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);


	auto&& render_batches = batches_manager.GetBatches();

	int set_ind = 0;
	for (auto&& batch : render_batches)
	{
		vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_collection.GetPipeline().GetHandle());

		VkDescriptorSet desc_sets[] = { descriptor_sets_[set_ind]};
		set_ind++;

		std::vector<VkBuffer> vert_bufs;
		std::vector<VkDeviceSize> offsetes;

		for (auto&& buf : batch.GetVertexBuffers())
		{
			vert_bufs.push_back(buf.buffer_.GetHandle());
			offsetes.push_back(buf.offset_);
		}

		vkCmdBindVertexBuffers(command_buffer_, 0, vert_bufs.size(), vert_bufs.data(), offsetes.data());
		vkCmdBindIndexBuffer(command_buffer_, batch.GetIndexBuffer().buffer_.GetHandle(), batch.GetIndexBuffer().offset_, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_collection.GetPipeline().GetLayout(), 0, 1, desc_sets, 0, nullptr);
		vkCmdDrawIndexed(command_buffer_, batch.GetDrawSize(), 1, 0, 0, 0);
	}

	vkCmdEndRenderPass(command_buffer_);

	if (vkEndCommandBuffer(command_buffer_) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}

	return false;

	return true;
}

bool render::FrameHandler::Draw(const Framebuffer& swapchain_framebuffer, uint32_t image_index, const PipelineCollection& pipeline_collection, const BatchesManager& batches_manager, glm::vec3 pos, glm::vec3 look)
{
	if (descriptor_sets_.size() == 0)
	{
		descriptor_sets_.resize(batches_manager.GetBatches().size());
		device_cfg_.descriptor_pool->AllocateSet(pipeline_collection.GetPipeline().GetDescriptorSetLayout(), batches_manager.GetBatches().size(), descriptor_sets_);
		for (int i = 0; i < batches_manager.GetBatches().size(); i++)
		{
			BuildDescriptorSet(i, batches_manager.GetBatches()[i].GetColorImageView(), batches_manager.GetEnvImageView());
		}
	}

	int uni_ind = 0;
	for (auto&& batch : batches_manager.GetBatches())
	{
		UniformBufferObject ubo{};

		ubo.model = batch.GetModelMatrix();
		ubo.view = glm::lookAt(pos, pos + look, glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), 16.0f / 9.f, 0.1f, 100.0f);
		ubo.proj[1][1] *= -1;

		void* data;
		vkMapMemory(device_cfg_.logical_device, uniform_buffers_[uni_ind].GetBufferMemory(), 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device_cfg_.logical_device, uniform_buffers_[uni_ind].GetBufferMemory());
		uni_ind++;
	}

	submit_info_.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	submit_info_.waitSemaphoreCount = 1;

	submit_info_.pWaitDstStageMask = &wait_stages_;

	submit_info_.commandBufferCount = 1;
	submit_info_.pCommandBuffers = &command_buffer_;

	submit_info_.signalSemaphoreCount = 1;

	submit_info_.pSignalSemaphores = &render_finished_semaphore_;


	present_info_.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	present_info_.waitSemaphoreCount = 1;
	present_info_.pWaitSemaphores = &render_finished_semaphore_;

	present_info_.swapchainCount = 1;
	present_info_.pSwapchains = &swapchain_;

	present_info_.pResults = nullptr; // Optional


	submit_info_.pWaitSemaphores = &image_available_semaphore_;

	vkWaitForFences(device_cfg_.logical_device, 1, &cmd_buffer_fence_, VK_TRUE, UINT64_MAX);
	vkResetFences(device_cfg_.logical_device, 1, &cmd_buffer_fence_);

	FillCommandBuffer(swapchain_framebuffer, image_index, pipeline_collection, batches_manager);

	//if (acquire_semaphore_prev != VK_NULL_HANDLE)
	//{
	//	vkDestroySemaphore(device_cfg_.logical_device, acquire_semaphore_prev, nullptr);
	//}

	//acquire_semaphore_prev = image_acquire_semaphore;

	present_info_.pImageIndices = &image_index;

	if (vkQueueSubmit(graphics_queue_, 1, &submit_info_, cmd_buffer_fence_) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	return vkQueuePresentKHR(graphics_queue_, &present_info_) == VK_SUCCESS;
}

VkSemaphore render::FrameHandler::GetImageAvailableSemaphore() const
{
	return image_available_semaphore_;
}


render::FrameHandler::~FrameHandler()
{
	if (handle_ != nullptr)
	{
		vkDestroyFence(device_cfg_.logical_device, cmd_buffer_fence_, nullptr);
		vkDestroySemaphore(device_cfg_.logical_device, render_finished_semaphore_, nullptr);
	}
}

void render::FrameHandler::BuildDescriptorSet(uint32_t batch_index, const render::ImageView& color_image_view, const render::ImageView& env_image_view)
{
	uniform_buffers_.push_back(UniformBuffer(device_cfg_, sizeof(UniformBufferObject)));

	VkDescriptorBufferInfo buffer_info{};
	buffer_info.buffer = uniform_buffers_.back().GetHandle();
	buffer_info.offset = 0;
	buffer_info.range = sizeof(UniformBufferObject);

	VkDescriptorImageInfo color_image_info{};
	color_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	color_image_info.imageView = color_image_view.GetHandle();
	color_image_info.sampler = color_sampler_.GetHandle();

	VkDescriptorImageInfo env_image_info{};
	env_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	env_image_info.imageView = env_image_view.GetHandle();
	env_image_info.sampler = color_sampler_.GetHandle();

	std::array<VkWriteDescriptorSet, 3> descriptor_writes{};

	descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_writes[0].dstSet = descriptor_sets_[batch_index];
	descriptor_writes[0].dstBinding = 0;
	descriptor_writes[0].dstArrayElement = 0;
	descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor_writes[0].descriptorCount = 1;
	descriptor_writes[0].pBufferInfo = &buffer_info;

	descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_writes[1].dstSet = descriptor_sets_[batch_index];
	descriptor_writes[1].dstBinding = 1;
	descriptor_writes[1].dstArrayElement = 0;
	descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptor_writes[1].descriptorCount = 1;
	descriptor_writes[1].pImageInfo = &color_image_info;

	descriptor_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_writes[2].dstSet = descriptor_sets_[batch_index];
	descriptor_writes[2].dstBinding = 2;
	descriptor_writes[2].dstArrayElement = 0;
	descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptor_writes[2].descriptorCount = 1;
	descriptor_writes[2].pImageInfo = &env_image_info;

	vkUpdateDescriptorSets(device_cfg_.logical_device, descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);

}
