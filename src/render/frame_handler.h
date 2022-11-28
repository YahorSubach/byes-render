#ifndef RENDER_ENGINE_RENDER_FRAME_HANDLER_H_
#define RENDER_ENGINE_RENDER_FRAME_HANDLER_H_

#include "vulkan/vulkan.h"

#include "object_base.h"

#include "render/buffer.h"
#include "render/swapchain.h"
#include "render/render_setup.h"
#include "render/batches_manager.h"
#include "render/render_graph.h"
#include "render/sampler.h"
#include "render/scene.h"

namespace render
{

	class FrameHandler: public RenderObjBase<void*>
	{
	public:
		FrameHandler(const DeviceConfiguration& device_cfg, const Swapchain& swapchain, const RenderSetup& render_setup, const std::array<Extent, kExtentTypeCnt>& extents, DescriptorSetsManager& descriptor_set_manager, const BatchesManager& batches_manager, const ui::UI& ui);
		
		FrameHandler(const FrameHandler&) = delete;
		FrameHandler(FrameHandler&&) = default;

		FrameHandler& operator=(const FrameHandler&) = delete;
		FrameHandler& operator=(FrameHandler&&) = default;
		
		bool Draw(const Framebuffer& swapchain_framebuffer, const Image& swapchain_image, uint32_t image_index, glm::vec3 pos, glm::vec3 look);

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

		const ui::UI& ui_;

		const RenderSetup& render_setup_;

		ModelSceneDescSetHolder model_scene_;
		RenderGraphHandler render_graph_handeler_;
		UIScene ui_scene_;
		Buffer viewport_vertex_buffer_;
	};
}

#endif  // RENDER_ENGINE_RENDER_FRAME_HANDLER_H_