#ifndef RENDER_ENGINE_RENDER_FRAME_HANDLER_H_
#define RENDER_ENGINE_RENDER_FRAME_HANDLER_H_

#include "vulkan/vulkan.h"

#include "object_base.h"
#include "buffer.h"

namespace render
{
	class FrameHandler: public RenderObjBase<void*>
	{
	public:
		FrameHandler(const DeviceConfiguration& device_cfg, VkSwapchainKHR swapchain, uint32_t image_index, const VkCommandBuffer& command_buffer,VkSemaphore render_finished_semaphore);
		
		FrameHandler(const FrameHandler&) = delete;
		FrameHandler(FrameHandler&&) = default;

		FrameHandler& operator=(const FrameHandler&) = delete;
		FrameHandler& operator=(FrameHandler&&) = default;
		
		bool Process(VkSemaphore& image_acquire_semaphore);
		const Buffer& GetUniformBuffer();
		virtual ~FrameHandler() override;
	private:
		VkSwapchainKHR swapchain_;
		VkCommandBuffer command_buffer_;
		
		VkPipelineStageFlags wait_stages_;

		uint32_t image_index_;

		Buffer uniform_buffer_;

		VkSemaphore render_finished_semaphore_;
		VkFence cmd_buffer_fence_;

		VkSubmitInfo submit_info_;
		VkPresentInfoKHR present_info_;

		VkQueue graphics_queue_;
	};
}

#endif  // RENDER_ENGINE_RENDER_FRAME_HANDLER_H_