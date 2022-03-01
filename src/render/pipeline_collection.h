#ifndef RENDER_ENGINE_RENDER_PIPELINE_COLLECTION_H_
#define RENDER_ENGINE_RENDER_PIPELINE_COLLECTION_H_


#include "vulkan/vulkan.h"

#include "render/graphics_pipeline.h"
#include "render/graphics_pipeline.h"

namespace render
{
	class PipelineCollection: RenderObjBase<bool>
	{
	public:
		PipelineCollection(const DeviceConfiguration& device_cfg, Extent output_extent, uint32_t output_format);

		const GraphicsPipeline& GetPipeline() const;
		const RenderPass& GetRenderPass() const;
	private:

		VkShaderModule CreateShaderModule(const std::vector<char>& code);

		std::vector<GraphicsPipeline> pipelines_;
		std::vector<RenderPass> render_passes_;
	};
}
#endif  // RENDER_ENGINE_RENDER_PIPELINE_COLLECTION_H_