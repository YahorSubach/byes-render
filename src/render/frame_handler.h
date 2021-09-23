#ifndef RENDER_ENGINE_RENDER_FRAME_HANDLER_H_
#define RENDER_ENGINE_RENDER_FRAME_HANDLER_H_

#include "vulkan/vulkan.h"

#include "validate_object.h"

namespace render
{
	class FrameHandler: public ValidateObject
	{
	public:
		FrameHandler(VkDevice device, VkQueue graphics_queue, VkSwapchainKHR swapchain, uint32_t image_index, VkCommandBuffer& command_buffer,VkSemaphore render_finished_semaphore);
		void Process(VkSemaphore& image_acquire_semaphore);

	private:
		VkDevice device_;
		VkSwapchainKHR swapchain_;
		VkCommandBuffer command_buffer_;
		
		VkPipelineStageFlags wait_stages_;

		uint32_t image_index_;

		VkSemaphore render_finished_semaphore_;
		VkFence cmd_buffer_fence_;

		VkSubmitInfo submit_info_;
		VkPresentInfoKHR present_info_;

		VkQueue graphics_queue_;
	};
}

#endif  // RENDER_ENGINE_RENDER_FRAME_HANDLER_H_