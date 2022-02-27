#ifndef RENDER_ENGINE_RENDER_BATCHES_MANAGER_H_
#define RENDER_ENGINE_RENDER_BATCHES_MANAGER_H_

#include "vulkan/vulkan.h"

#include <vector>

#include "common.h"
#include "render/object_base.h"
#include "render/data_types.h"
#include "render/batch.h"
#include "render/descriptor_pool.h"
#include "render/image.h"
#include "render/image_view.h"
#include "render/render_pass.h"
#include "render/sampler.h"
#include "render/swapchain.h"

#include "gltf_wrapper.h"


namespace render
{
	class BatchesManager: RenderObjBase<void*>
	{
	public:

		BatchesManager(const DeviceConfiguration& device_cfg, uint32_t frames_cnt, const Swapchain& swapchain, const RenderPass& render_pass, DescriptorPool& descriptor_pool);

		BatchesManager(const BatchesManager&) = delete;
		BatchesManager(BatchesManager&&) = default;

		BatchesManager& operator=(const BatchesManager&) = delete;
		BatchesManager& operator=(BatchesManager&&) = default;

		std::vector<std::reference_wrapper<render::Batch>> GetBatches();

		uint32_t GetUniformSetCnt();
		uint32_t GetSamplerSetCnt();

		struct DescSetsAndBuffers
		{
			std::vector<VkDescriptorSet> descriptor_sets;
			std::vector<UniformBuffer> uniform_buffers;
		};

	private:

		DescSetsAndBuffers BuildDescriptorSets(uint32_t frames_cnt, const ImageView& image_view, const VkDescriptorSetLayout& layout);

		VkShaderModule CreateShaderModule(const std::vector<char>& code);

		std::unique_ptr<DescriptorPool> descriptor_pool_ptr_;

		std::vector<Batch> batches_;

		uint32_t uniform_set_cnt_;
		uint32_t sampler_set_cnt_;


		std::vector<GraphicsPipeline> pipelines_;
		std::vector<GPULocalBuffer> buffers_;
		std::vector<Image> images_;
		std::vector<ImageView> image_views_;
		
		std::vector<GLTFWrapper> gltf_wrappers_;

		Sampler color_sampler_;

	};
}

#endif  // RENDER_ENGINE_RENDER_BATCHES_MANAGER_H_