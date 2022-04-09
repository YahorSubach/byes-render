#ifndef RENDER_ENGINE_RENDER_PIPELINE_COLLECTION_H_
#define RENDER_ENGINE_RENDER_PIPELINE_COLLECTION_H_


#include "vulkan/vulkan.h"

#include "render/graphics_pipeline.h"
#include "render/descriptor_set_layout.h"

namespace render
{
	class PipelineCollection: RenderObjBase<bool>
	{
	public:

		enum class PipelineId
		{
			kColor,
			kDepth
		};

		enum class RenderPassId
		{
			kScreen,
			kDepth
		};

		enum class DescriptorSetLayoutId : uint32_t
		{
			kCameraMatrices,

			kObjectSet,
			kMaterialSet,

			kDescriptorSetLayoutsIdCount
		};


		PipelineCollection(const DeviceConfiguration& device_cfg, Extent output_extent, uint32_t output_format);

		const GraphicsPipeline& GetPipeline(PipelineId pipeline_id) const;
		const RenderPass& GetRenderPass(RenderPassId renderpass_id) const;

		const DescriptorSetLayout& GetDescriptorSetLayout(DescriptorSetLayoutId descriptor_set_layout_id) const;

	private:

		void InitDescriptorSetLayouts(const DeviceConfiguration& device_cfg);
		VkShaderModule CreateShaderModule(const std::vector<char>& code);

		std::vector<GraphicsPipeline> pipelines_;
		std::vector<RenderPass> render_passes_;
		std::vector<DescriptorSetLayout> descriptor_set_layouts_;
	};
}
#endif  // RENDER_ENGINE_RENDER_PIPELINE_COLLECTION_H_