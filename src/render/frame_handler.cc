#include "frame_handler.h"

#include <utility>

#include "vk_util.h"

render::FrameHandler::FrameHandler(const VkDevice& device, VkQueue graphics_queue, VkSwapchainKHR swapchain, uint32_t image_index, const VkCommandBuffer& command_buffer, VkSemaphore render_finished_semaphore):
	RenderObjBase(device), swapchain_(swapchain), image_index_(image_index), graphics_queue_(graphics_queue), command_buffer_(command_buffer), render_finished_semaphore_(render_finished_semaphore),
	cmd_buffer_fence_(vk_util::CreateFence(device)), present_info_{}, submit_info_{}, wait_stages_(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
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
	submit_info_.pWaitSemaphores = &image_acquire_semaphore;

	vkWaitForFences(device_, 1, &cmd_buffer_fence_, VK_TRUE, UINT64_MAX);
	vkResetFences(device_, 1, &cmd_buffer_fence_);

	if (vkQueueSubmit(graphics_queue_, 1, &submit_info_, cmd_buffer_fence_) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	return vkQueuePresentKHR(graphics_queue_, &present_info_) == VK_SUCCESS;
}

render::FrameHandler::~FrameHandler()
{
	vkDestroyFence(device_, cmd_buffer_fence_, nullptr);
}
