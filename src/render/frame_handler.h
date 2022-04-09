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
		
		bool FillCommandBuffer(const Framebuffer& swapchain_framebuffer, const PipelineCollection& pipeline_collection, const BatchesManager& batches_manager);
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

		std::map<std::string, DescriptorSetLayout> descriptor_sets_layouts;

		UniformBuffer camera_uniform_buffer_;
		VkDescriptorSet camera_descriptor_set_;

		std::vector<UniformBuffer> model_uniform_buffers_;
		std::vector<VkDescriptorSet> model_descriptor_sets_;

		std::vector<VkDescriptorSet> material_descriptor_sets_;

		Sampler color_sampler_;


		void UpdateCameraDescriptorSet(glm::vec3 pos, glm::vec3 look);
		void UpdateModelDescriptorSet(uint32_t model_descriptor_set_index, const Batch& batch);
		void UpdateMaterialDescriptorSet(uint32_t material_descriptor_set_index, const Batch& batch, const ImageView& env_image_view);
	};
}

#endif  // RENDER_ENGINE_RENDER_FRAME_HANDLER_H_