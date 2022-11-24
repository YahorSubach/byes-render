#ifndef RENDER_ENGINE_RENDER_RENDER_SETUP_H_
#define RENDER_ENGINE_RENDER_RENDER_SETUP_H_

#include <array>
#include <map>
#include <vector>

#include "vulkan/vulkan.h"

#include "render/graphics_pipeline.h"
#include "render/descriptor_set_layout.h"

#include "render/render_graph.h"

namespace render
{
	class RenderSetup: RenderObjBase<bool>
	{
	public:

		RenderSetup(const DeviceConfiguration& device_cfg);

		const GraphicsPipeline& GetPipeline(PipelineId pipeline_id) const;
		const RenderPass& GetRenderPass(RenderPassId renderpass_id) const;

		const DescriptorSetLayout& GetDescriptorSetLayout(DescriptorSetType type) const;

	private:

		void InitDescriptorSetLayouts(const DeviceConfiguration& device_cfg);
		VkShaderModule CreateShaderModule(const std::vector<char>& code);


		RenderGraph2 render_graph_;

		std::map<PipelineId, GraphicsPipeline> pipelines_;
		std::map<RenderPassId, RenderPass> render_passes_;
		
		std::array<DescriptorSetLayout, static_cast<uint32_t>(DescriptorSetType::Count)> descriptor_set_layouts_;
	};
}
#endif  // RENDER_ENGINE_RENDER_RENDER_SETUP_H_