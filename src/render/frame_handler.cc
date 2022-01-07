#include "frame_handler.h"

#include <chrono>
#include <utility>

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include "vk_util.h"
#include <render/data_types.h>

render::FrameHandler::FrameHandler(const VkDevice& device, const VkPhysicalDevice& physical_device, VkQueue graphics_queue, VkSwapchainKHR swapchain, uint32_t image_index, const VkCommandBuffer& command_buffer, VkSemaphore render_finished_semaphore):
	RenderObjBase(device), swapchain_(swapchain), image_index_(image_index), graphics_queue_(graphics_queue), command_buffer_(command_buffer), render_finished_semaphore_(render_finished_semaphore),
	cmd_buffer_fence_(vk_util::CreateFence(device)), present_info_{}, submit_info_{}, wait_stages_(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT),
	uniform_buffer_(device, physical_device, sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {})
{

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
	present_info_.pImageIndices = &image_index_;

	present_info_.pResults = nullptr; // Optional
}

bool render::FrameHandler::Process(VkSemaphore& image_acquire_semaphore)
{

	static auto start_time = std::chrono::high_resolution_clock::now();

	auto current_time = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

	UniformBufferObject ubo{};
	
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), 16.0f / 9.f, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;

	void* data;
	vkMapMemory(device_, uniform_buffer_.GetBufferMemory(), 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device_, uniform_buffer_.GetBufferMemory());

	submit_info_.pWaitSemaphores = &image_acquire_semaphore;

	vkWaitForFences(device_, 1, &cmd_buffer_fence_, VK_TRUE, UINT64_MAX);
	vkResetFences(device_, 1, &cmd_buffer_fence_);

	if (vkQueueSubmit(graphics_queue_, 1, &submit_info_, cmd_buffer_fence_) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	return vkQueuePresentKHR(graphics_queue_, &present_info_) == VK_SUCCESS;
}

const render::Buffer& render::FrameHandler::GetUniformBuffer()
{
	return uniform_buffer_;
}

render::FrameHandler::~FrameHandler()
{
	vkDestroyFence(device_, cmd_buffer_fence_, nullptr);
}
