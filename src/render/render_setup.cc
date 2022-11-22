#include "render_setup.h"

#include <fstream>

#include "render/shader_module.h"

render::RenderSetup::RenderSetup(const DeviceConfiguration& device_cfg): 
	RenderObjBase(device_cfg),
	descriptor_set_layouts_
	{
#define ENUM_OP(val) DescriptorSetLayout(device_cfg, DescriptorSetType::k##val),
#include "render/descriptor_types.inl"
#undef ENUM_OP
	}
{

	Extent output_extent = device_cfg_.presentation_extent;
	VkFormat output_format = device_cfg_.presentation_format;

	render_passes_.emplace(RenderPassId::kSimpleRenderToScreen, RenderPass(device_cfg_, true));
	render_passes_.at(RenderPassId::kSimpleRenderToScreen).AddColorAttachment("swapchain_image", false);
	render_passes_.at(RenderPassId::kSimpleRenderToScreen).AddDepthAttachment("depth_image");


	render_passes_.emplace(RenderPassId::kBuildDepthmap, RenderPass(device_cfg_));
	render_passes_.at(RenderPassId::kBuildDepthmap).AddDepthAttachment("shadowmap");

	render_passes_.emplace(RenderPassId::kBuildGBuffers, RenderPass(device_cfg_));
	render_passes_.at(RenderPassId::kBuildGBuffers).AddColorAttachment("g_albedo");
	render_passes_.at(RenderPassId::kBuildGBuffers).AddColorAttachment("g_position");
	render_passes_.at(RenderPassId::kBuildGBuffers).AddColorAttachment("g_normal");
	render_passes_.at(RenderPassId::kBuildGBuffers).AddColorAttachment("g_metal_rough");
	render_passes_.at(RenderPassId::kBuildGBuffers).AddDepthAttachment("g_depth");

	render_passes_.emplace(RenderPassId::kCollectGBuffers, RenderPass(device_cfg_, true));
	render_passes_.at(RenderPassId::kCollectGBuffers).AddColorAttachment("swapchain_image");


	{
		ShaderModule vert_shader_module(device_cfg, "color.vert", descriptor_set_layouts_);
		ShaderModule frag_shader_module(device_cfg, "color.frag", descriptor_set_layouts_);

		pipelines_.emplace(PipelineId::kColor, GraphicsPipeline(device_cfg, output_extent, render_passes_.at(RenderPassId::kSimpleRenderToScreen), vert_shader_module, frag_shader_module));
	}

	{
		ShaderModule vert_shader_module(device_cfg, "color_skin.vert", descriptor_set_layouts_);
		ShaderModule frag_shader_module(device_cfg, "color.frag", descriptor_set_layouts_);

		pipelines_.emplace(PipelineId::kColorSkinned, GraphicsPipeline(device_cfg, output_extent, render_passes_.at(RenderPassId::kSimpleRenderToScreen), vert_shader_module, frag_shader_module));
	}


	{
		ShaderModule vert_shader_module(device_cfg, "shadow.vert", descriptor_set_layouts_);
		ShaderModule frag_shader_module(device_cfg, "shadow.frag", descriptor_set_layouts_);

		pipelines_.emplace(PipelineId::kDepth, GraphicsPipeline(device_cfg, {512, 512}, render_passes_.at(RenderPassId::kBuildDepthmap), vert_shader_module, frag_shader_module));
	}

	{
		ShaderModule vert_shader_module(device_cfg, "shadow_skin.vert", descriptor_set_layouts_);
		ShaderModule frag_shader_module(device_cfg, "shadow.frag", descriptor_set_layouts_);

		pipelines_.emplace(PipelineId::kDepthSkinned, GraphicsPipeline(device_cfg, { 512, 512 }, render_passes_.at(RenderPassId::kBuildDepthmap), vert_shader_module, frag_shader_module));
	}

	{
		ShaderModule vert_shader_module(device_cfg, "ui.vert", descriptor_set_layouts_);
		ShaderModule frag_shader_module(device_cfg, "ui.frag", descriptor_set_layouts_);

		pipelines_.emplace(PipelineId::kUI, GraphicsPipeline(device_cfg, output_extent, render_passes_.at(RenderPassId::kSimpleRenderToScreen), vert_shader_module, frag_shader_module, false));
	}

	{
		ShaderModule vert_shader_module(device_cfg, "build_g_buffers.vert", descriptor_set_layouts_);
		ShaderModule frag_shader_module(device_cfg, "build_g_buffers.frag", descriptor_set_layouts_);

		pipelines_.emplace(PipelineId::kBuildGBuffers, GraphicsPipeline(device_cfg, output_extent, render_passes_.at(RenderPassId::kBuildGBuffers), vert_shader_module, frag_shader_module));
	}

	{
		ShaderModule vert_shader_module(device_cfg, "collect_g_buffers.vert", descriptor_set_layouts_);
		ShaderModule frag_shader_module(device_cfg, "collect_g_buffers.frag", descriptor_set_layouts_);

		pipelines_.emplace(PipelineId::kCollectGBuffers, GraphicsPipeline(device_cfg, output_extent, render_passes_.at(RenderPassId::kCollectGBuffers), vert_shader_module, frag_shader_module));
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
