#include "render_setup.h"

#include <fstream>

#include "render/shader_module.h"

render::RenderSetup::RenderSetup(const DeviceConfiguration& device_cfg, Extent output_extent, uint32_t output_format): 
	RenderObjBase(device_cfg),
	descriptor_set_layouts_
	{
#define ENUM_OP(val) DescriptorSetLayout(device_cfg, DescriptorSetType::k##val),
#include "render/descriptor_types.inl"
#undef ENUM_OP
	}
{

	{
		auto render_pass_desc = RenderPass::BuildRenderPassDesc(RenderPass::RenderPassType::kDraw, (VkFormat)output_format, VK_FORMAT_D32_SFLOAT);
		render_passes_.emplace(RenderPassId::kScreen, RenderPass(device_cfg_, render_pass_desc));

		ShaderModule vert_shader_module(device_cfg, "color.vert.spv", descriptor_set_layouts_);
		ShaderModule frag_shader_module(device_cfg, "color.frag.spv", descriptor_set_layouts_);

		pipelines_.emplace(PipelineId::kColor, GraphicsPipeline(device_cfg, output_extent, render_passes_.at(RenderPassId::kScreen), vert_shader_module, frag_shader_module));
	}

	{
		auto depth_render_pass_desc = RenderPass::BuildRenderPassDesc(RenderPass::RenderPassType::kDepth, (VkFormat)output_format, VK_FORMAT_D32_SFLOAT);
		render_passes_.emplace(RenderPassId::kDepth, RenderPass(device_cfg_, depth_render_pass_desc));

		ShaderModule vert_shader_module(device_cfg, "shadow.vert.spv", descriptor_set_layouts_);
		ShaderModule frag_shader_module(device_cfg, "shadow.frag.spv", descriptor_set_layouts_);

		pipelines_.emplace(PipelineId::kDepth, GraphicsPipeline(device_cfg, {512, 512}, render_passes_.at(RenderPassId::kDepth), vert_shader_module, frag_shader_module));
	}
}

const render::RenderPass& render::RenderSetup::GetRenderPass(RenderPassId renderpass_id) const
{
	return render_passes_.at(renderpass_id);
}

const render::DescriptorSetLayout& render::RenderSetup::GetDescriptorSetLayout(DescriptorSetType type) const
{
	return descriptor_set_layouts_[static_cast<int>(type)];
}

const render::GraphicsPipeline& render::RenderSetup::GetPipeline(PipelineId pipeline_id) const
{
	return pipelines_.at(pipeline_id);
}
