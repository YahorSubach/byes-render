#include "render_setup.h"

#include <fstream>

#include "render/shader_module.h"

#include "render/descriptor_set.h"

#include "global.h"

namespace render
{
	RenderSetup::RenderSetup(const Global& global) :
		RenderObjBase(global),
		render_graph_(global)
	{


		g_build_node = render_graph_.AddNode("g_build", ExtentType::kPresentation);
		g_collect_node = render_graph_.AddNode("g_collect", ExtentType::kPresentation);
		ui_node = render_graph_.AddNode("ui", ExtentType::kPresentation);

		g_collect_node->use_swapchain_framebuffer = true;
		ui_node->use_swapchain_framebuffer = true;

		g_build_node->Attach("g_albedo", global.high_range_color_format) >> DescriptorSetType::kGBuffers >> 0 >> *g_collect_node;
		g_build_node->Attach("g_position", global.high_range_color_format) >> DescriptorSetType::kGBuffers >> 1 >> *g_collect_node;
		g_build_node->Attach("g_normal", global.high_range_color_format) >> DescriptorSetType::kGBuffers >> 2 >> *g_collect_node;
		g_build_node->Attach("g_metal_rough", global.high_range_color_format) >> DescriptorSetType::kGBuffers >> 3 >> *g_collect_node;
		g_build_node->Attach("g_depth", global.depth_map_format);

		auto&& swapchain_attachment = g_collect_node->AttachSwapchain() >> *ui_node;

		render_graph_.Build();

		swapchain_render_pass_ = g_collect_node->GetRenderPass();

		//render_passes_.emplace(RenderPassId::kSimpleRenderToScreen, RenderPass(global_, RenderPass::SwapchainInteraction::kPresent));
		//render_passes_.at(RenderPassId::kSimpleRenderToScreen).AddColorAttachment("swapchain_image", false);


		//render_passes_.emplace(RenderPassId::kBuildDepthmap, RenderPass(global_));
		//render_passes_.at(RenderPassId::kBuildDepthmap).AddDepthAttachment("shadowmap");

		//render_passes_.emplace(RenderPassId::kBuildGBuffers, RenderPass(global_));
		//render_passes_.at(RenderPassId::kBuildGBuffers).AddColorAttachment("g_albedo");
		//render_passes_.at(RenderPassId::kBuildGBuffers).AddColorAttachment("g_position");
		//render_passes_.at(RenderPassId::kBuildGBuffers).AddColorAttachment("g_normal");
		//render_passes_.at(RenderPassId::kBuildGBuffers).AddColorAttachment("g_metal_rough");
		//render_passes_.at(RenderPassId::kBuildGBuffers).AddDepthAttachment("g_depth");

		//render_passes_.emplace(RenderPassId::kCollectGBuffers, RenderPass(global_, RenderPass::SwapchainInteraction::kAcquire));
		//render_passes_.at(RenderPassId::kCollectGBuffers).AddColorAttachment("swapchain_image");

		//render_passes_.emplace(RenderPassId::kUI, RenderPass(global_, RenderPass::SwapchainInteraction::kPresent));
		//render_passes_.at(RenderPassId::kUI).AddColorAttachment("swapchain_image");


		//{
		//	ShaderModule vert_shader_module(global, "color.vert", descriptor_set_layouts_);
		//	ShaderModule frag_shader_module(global, "color.frag", descriptor_set_layouts_);

		//	pipelines_.emplace(PipelineId::kColor, GraphicsPipeline(global, output_extent, render_passes_.at(RenderPassId::kSimpleRenderToScreen), vert_shader_module, frag_shader_module));
		//}

		//{
		//	ShaderModule vert_shader_module(global, "color_skin.vert", descriptor_set_layouts_);
		//	ShaderModule frag_shader_module(global, "color.frag", descriptor_set_layouts_);

		//	pipelines_.emplace(PipelineId::kColorSkinned, GraphicsPipeline(global, output_extent, render_passes_.at(RenderPassId::kSimpleRenderToScreen), vert_shader_module, frag_shader_module));
		//}


		//{
		//	ShaderModule vert_shader_module(global, "shadow.vert", descriptor_set_layouts_);
		//	ShaderModule frag_shader_module(global, "shadow.frag", descriptor_set_layouts_);

		//	pipelines_.emplace(PipelineId::kDepth, GraphicsPipeline(global, {512, 512}, render_passes_.at(RenderPassId::kBuildDepthmap), vert_shader_module, frag_shader_module));
		//}

		//{
		//	ShaderModule vert_shader_module(global, "shadow_skin.vert", descriptor_set_layouts_);
		//	ShaderModule frag_shader_module(global, "shadow.frag", descriptor_set_layouts_);

		//	pipelines_.emplace(PipelineId::kDepthSkinned, GraphicsPipeline(global, { 512, 512 }, render_passes_.at(RenderPassId::kBuildDepthmap), vert_shader_module, frag_shader_module));
		//}


	}

	const RenderGraph2& RenderSetup::GetRenderGraph() const
	{
		return render_graph_;
	}

	const RenderPass& RenderSetup::GetSwapchainRenderPass() const
	{
		return swapchain_render_pass_.value();
	}

	const std::map<PipelineId, GraphicsPipeline>& RenderSetup::GetPipelines() const
	{
		return pipelines_;
	}

	void RenderSetup::InitPipelines(const DescriptorSetsManager& descriptor_set_manager, const std::array<Extent, kExtentTypeCnt>& extents)
	{
		pipelines_.clear();
		render_graph_.ClearPipelines();

		{
			ShaderModule vert_shader_module(global_, "bitmap.vert", descriptor_set_manager.GetLayouts());
			ShaderModule frag_shader_module(global_, "bitmap.frag", descriptor_set_manager.GetLayouts());

			pipelines_.emplace(PipelineId::kUI, GraphicsPipeline(global_, *ui_node, vert_shader_module, frag_shader_module, extents, PrimitiveProps::kUIShape, GraphicsPipeline::EParams::kDisableDepthTest));
			ui_node->AddPipeline(pipelines_.at(PipelineId::kUI));
		}

		{
			ShaderModule vert_shader_module(global_, "build_g_buffers.vert", descriptor_set_manager.GetLayouts());
			ShaderModule frag_shader_module(global_, "build_g_buffers.frag", descriptor_set_manager.GetLayouts());

			pipelines_.emplace(PipelineId::kBuildGBuffers, GraphicsPipeline(global_, *g_build_node, vert_shader_module, frag_shader_module, extents, PrimitiveProps::kOpaque));
			g_build_node->AddPipeline(pipelines_.at(PipelineId::kBuildGBuffers));
		}

		{
			ShaderModule vert_shader_module(global_, "collect_g_buffers.vert", descriptor_set_manager.GetLayouts());
			ShaderModule frag_shader_module(global_, "collect_g_buffers.frag", descriptor_set_manager.GetLayouts());

			pipelines_.emplace(PipelineId::kCollectGBuffers, GraphicsPipeline(global_, *g_collect_node, vert_shader_module, frag_shader_module, extents, PrimitiveProps::kViewport));
			g_collect_node->AddPipeline(pipelines_.at(PipelineId::kCollectGBuffers));
		}

		{
			ShaderModule vert_shader_module(global_, "pos_color.vert", descriptor_set_manager.GetLayouts());
			ShaderModule frag_shader_module(global_, "pos_color.frag", descriptor_set_manager.GetLayouts());

			pipelines_.emplace(PipelineId::kDebugLines, GraphicsPipeline(global_, *ui_node, vert_shader_module, frag_shader_module, extents, PrimitiveProps::kDebugLines, GraphicsPipeline::EParams::kLineTopology));
			ui_node->AddPipeline(pipelines_.at(PipelineId::kDebugLines));
		}

		//{
		//	ShaderModule vert_shader_module(global_, "pos.vert", descriptor_set_manager.GetLayouts());
		//	ShaderModule frag_shader_module(global_, "pos.frag", descriptor_set_manager.GetLayouts());

		//	pipelines_.emplace(PipelineId::kPos, GraphicsPipeline(global_, *ui_node, vert_shader_module, frag_shader_module, extents, PrimitiveProps::kUIShape));
		//	ui_node->AddPipeline(pipelines_.at(PipelineId::kPos));
		//}

		{
			ShaderModule vert_shader_module(global_, "dbg_color_uni.vert", descriptor_set_manager.GetLayouts());
			ShaderModule frag_shader_module(global_, "dbg_color_uni.frag", descriptor_set_manager.GetLayouts());

			pipelines_.emplace(PipelineId::kDebugPoints, GraphicsPipeline(global_, *ui_node, vert_shader_module, frag_shader_module, extents, PrimitiveProps::kDebugPoints, { GraphicsPipeline::EParams::kPointTopology, GraphicsPipeline::EParams::kDisableDepthTest }));
			ui_node->AddPipeline(pipelines_.at(PipelineId::kDebugPoints));
		}
	}

	const GraphicsPipeline& RenderSetup::GetPipeline(PipelineId pipeline_id) const
	{
		return pipelines_.at(pipeline_id);
	}

}