#ifndef RENDER_ENGINE_RENDER_FRAME_HANDLER_H_
#define RENDER_ENGINE_RENDER_FRAME_HANDLER_H_

#include "vulkan/vulkan.h"

#include "object_base.h"

#include "render/buffer.h"
#include "render/framebuffer.h"
#include "render/swapchain.h"
#include "render/pipeline_collection.h"
#include "render/batches_manager.h"
#include "render/sampler.h"

namespace render
{
	class FrameHandler: public RenderObjBase<void*>
	{
	public:
		FrameHandler(const DeviceConfiguration& device_cfg, const Swapchain& swapchain);
		
		FrameHandler(const FrameHandler&) = delete;
		FrameHandler(FrameHandler&&) = default;

		FrameHandler& operator=(const FrameHandler&) = delete;
		FrameHandler& operator=(FrameHandler&&) = default;
		
		bool FillCommandBuffer(const Framebuffer& swapchain_framebuffer, uint32_t swapchain_image_index, const PipelineCollection& pipeline_collection, const BatchesManager& batches_manager);
		bool Draw(const Framebuffer& swapchain_framebuffer, uint32_t image_index, const PipelineCollection& pipeline_collection, const BatchesManager& batches_manager, glm::vec3 pos, glm::vec3 look);

		VkSemaphore GetImageAvailableSemaphore() const;

		virtual ~FrameHandler() override;


	private:
		VkSwapchainKHR swapchain_;
		VkCommandBuffer command_buffer_;
		
		VkPipelineStageFlags wait_stages_;
		
		VkSemaphore render_finished_semaphore_;
		VkSemaphore image_available_semaphore_;

		VkFence cmd_buffer_fence_;

		VkSubmitInfo submit_info_;
		VkPresentInfoKHR present_info_;

		VkQueue graphics_queue_;

		//VkSemaphore acquire_semaphore = VK_NULL_HANDLE;
		//VkSemaphore acquire_semaphore_prev = VK_NULL_HANDLE;

		std::vector<UniformBuffer> uniform_buffers_;
		std::vector<VkDescriptorSet> descriptor_sets_;

		Sampler color_sampler_;

		void BuildDescriptorSet(uint32_t batch_index, const ImageView& color_image_view, const ImageView& env_image_view);
	};
}

#endif  // RENDER_ENGINE_RENDER_FRAME_HANDLER_H_