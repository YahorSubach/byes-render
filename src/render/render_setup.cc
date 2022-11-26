#include "render_setup.h"

#include <fstream>

#include "render/shader_module.h"

#include "render/descriptor_set.h"

render::RenderSetup::RenderSetup(const DeviceConfiguration& device_cfg): 
	RenderObjBase(device_cfg),
	render_graph_(device_cfg),
	descriptor_set_layouts_
	{
#define ENUM_OP(val) DescriptorSetLayout(device_cfg, DescriptorSetType::k##val),
#include "render/descriptor_types.inl"
#undef ENUM_OP
	}
{


	auto&& g_build_node = render_graph_.AddNode("g_build");
	auto&& g_collect_node = render_graph_.AddNode("g_build");
	auto&& ui_node = render_graph_.AddNode("g_build");

	g_build_node.AddAttachment("g_albedo", device_cfg.high_range_color_format, device_cfg.presentation_extent) >> DescriptorSetType::kGBuffers >> 0 >> g_collect_node;
	g_build_node.AddAttachment("g_position", device_cfg.high_range_color_format, device_cfg.presentation_extent) >> DescriptorSetType::kGBuffers >> 1 >> g_collect_node;
	g_build_node.AddAttachment("g_normal", device_cfg.high_range_color_format, device_cfg.presentation_extent) >> DescriptorSetType::kGBuffers >> 2 >> g_collect_node;
	g_build_node.AddAttachment("g_metal_rough", device_cfg.high_range_color_format, device_cfg.presentation_extent) >> DescriptorSetType::kGBuffers >> 3 >> g_collect_node;
	g_build_node.AddAttachment("g_depth", device_cfg.depth_map_format, device_cfg.presentation_extent);

	g_collect_node.AddAttachment("swapchain", device_cfg.presentation_format, device_cfg.presentation_extent) >> ui_node;






	Extent output_extent = device_cfg_.presentation_extent;
	VkFormat output_format = device_cfg_.presentation_format;

	render_passes_.emplace(RenderPassId::kSimpleRenderToScreen, RenderPass(device_cfg_, RenderPass::SwapchainInteraction::kPresent));
	render_passes_.at(RenderPassId::kSimpleRenderToScreen).AddColorAttachment("swapchain_image", false);


	render_passes_.emplace(RenderPassId::kBuildDepthmap, RenderPass(device_cfg_));
	render_passes_.at(RenderPassId::kBuildDepthmap).AddDepthAttachment("shadowmap");

	render_passes_.emplace(RenderPassId::kBuildGBuffers, RenderPass(device_cfg_));
	render_passes_.at(RenderPassId::kBuildGBuffers).AddColorAttachment("g_albedo");
	render_passes_.at(RenderPassId::kBuildGBuffers).AddColorAttachment("g_position");
	render_passes_.at(RenderPassId::kBuildGBuffers).AddColorAttachment("g_normal");
	render_passes_.at(RenderPassId::kBuildGBuffers).AddColorAttachment("g_metal_rough");
	render_passes_.at(RenderPassId::kBuildGBuffers).AddDepthAttachment("g_depth");

	render_passes_.emplace(RenderPassId::kCollectGBuffers, RenderPass(device_cfg_, RenderPass::SwapchainInteraction::kAcquire));
	render_passes_.at(RenderPassId::kCollectGBuffers).AddColorAttachment("swapchain_image");

	render_passes_.emplace(RenderPassId::kUI, RenderPass(device_cfg_, RenderPass::SwapchainInteraction::kPresent));
	render_passes_.at(RenderPassId::kUI).AddColorAttachment("swapchain_image");


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
