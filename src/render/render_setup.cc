#include "render_setup.h"

#include <fstream>

#include "render/shader_module.h"

#include "render/descriptor_set.h"

#include "global.h"

namespace render
{
	RenderSetup::RenderSetup(const Global& global) :
		RenderObjBase(global)
	{
		pipelines_.reserve(32);

		g_build_node = render_graph_.AddNode("g_build", ExtentType::kPresentation);
		cube_shadow_map_node = render_graph_.AddNode("cube_shadow_map", ExtentType::kShadowMap);



		g_collect_node = render_graph_.AddNode("g_collect", ExtentType::kPresentation);
		ui_node = render_graph_.AddNode("ui", ExtentType::kPresentation);

		g_collect_node->use_swapchain_framebuffer = true;
		ui_node->use_swapchain_framebuffer = true;

		g_build_node->Attach("g_albedo", FormatType::kHighRangeColor) >> DescriptorSetType::kGBuffers >> 0 >> *g_collect_node;
		g_build_node->Attach("g_position", FormatType::kHighRangeColor) >> DescriptorSetType::kGBuffers >> 1 >> *g_collect_node;
		g_build_node->Attach("g_normal", FormatType::kHighRangeColor) >> DescriptorSetType::kGBuffers >> 2 >> *g_collect_node;
		g_build_node->Attach("g_metal_rough", FormatType::kHighRangeColor) >> DescriptorSetType::kGBuffers >> 3 >> *g_collect_node;
		g_build_node->Attach("g_depth", FormatType::kDepth);

		cube_shadow_map_node->Attach("cube_depth", FormatType::kDepth, 6 * 10) >> DescriptorSetType::kShadowCubeMaps >> 0 >> *g_collect_node;

		auto&& swapchain_attachment = g_collect_node->AttachSwapchain() >> *ui_node;

	}

	void RenderSetup::BuildRenderPasses(const Formats& formats)
	{
		render_graph_.BuildRenderPasses(global_, formats);

		swapchain_render_pass_ = g_collect_node->GetRenderPass();
	}

	const RenderGraph2& RenderSetup::GetRenderGraph() const
	{
		return render_graph_;
	}

	const RenderPass& RenderSetup::GetSwapchainRenderPass() const
	{
		return swapchain_render_pass_.value();
	}

	const std::vector<GraphicsPipeline>& RenderSetup::GetPipelines() const
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

			pipelines_.push_back(GraphicsPipeline(global_, *ui_node, vert_shader_module, frag_shader_module, extents, PrimitiveProps::kUIShape, GraphicsPipeline::EParams::kDisableDepthTest));
			ui_node->AddPipeline(pipelines_.back());
		}

		{
			ShaderModule vert_shader_module(global_, "build_g_buffers.vert", descriptor_set_manager.GetLayouts());
			ShaderModule frag_shader_module(global_, "build_g_buffers.frag", descriptor_set_manager.GetLayouts());

			pipelines_.push_back(GraphicsPipeline(global_, *g_build_node, vert_shader_module, frag_shader_module, extents, PrimitiveProps::kOpaque));
			g_build_node->AddPipeline(pipelines_.back());
		}

		{
			ShaderModule vert_shader_module(global_, "collect_g_buffers.vert", descriptor_set_manager.GetLayouts());
			ShaderModule frag_shader_module(global_, "collect_g_buffers.frag", descriptor_set_manager.GetLayouts());

			pipelines_.push_back(GraphicsPipeline(global_, *g_collect_node, vert_shader_module, frag_shader_module, extents, PrimitiveProps::kViewport));
			g_collect_node->AddPipeline(pipelines_.back());
		}

		{
			ShaderModule vert_shader_module(global_, "dbg_color_uni.vert", descriptor_set_manager.GetLayouts());
			ShaderModule frag_shader_module(global_, "dbg_color_uni.frag", descriptor_set_manager.GetLayouts());

			pipelines_.push_back(GraphicsPipeline(global_, *ui_node, vert_shader_module, frag_shader_module, extents, PrimitiveProps::kDebugPoints, { GraphicsPipeline::EParams::kPointTopology, GraphicsPipeline::EParams::kDisableDepthTest }));
			ui_node->AddPipeline(pipelines_.back());
		}

		{
			ShaderModule vert_shader_module(global_, "pos_color.vert", descriptor_set_manager.GetLayouts());
			ShaderModule frag_shader_module(global_, "pos_color.frag", descriptor_set_manager.GetLayouts());

			pipelines_.push_back(GraphicsPipeline(global_, *ui_node, vert_shader_module, frag_shader_module, extents, PrimitiveProps::kDebugLines, GraphicsPipeline::EParams::kLineTopology));
			ui_node->AddPipeline(pipelines_.back());
		}

		//{
		//	ShaderModule vert_shader_module(global_, "pos.vert", descriptor_set_manager.GetLayouts());
		//	ShaderModule frag_shader_module(global_, "pos.frag", descriptor_set_manager.GetLayouts());

		//	pipelines_.push_back(GraphicsPipeline(global_, *ui_node, vert_shader_module, frag_shader_module, extents, PrimitiveProps::kUIShape));
		//	ui_node->AddPipeline(pipelines_.back));
		//}

		{
			ShaderModule vert_shader_module(global_, "cube_depth.vert", descriptor_set_manager.GetLayouts());
			ShaderModule geom_shader_module(global_, "cube_depth.geom", descriptor_set_manager.GetLayouts());
			ShaderModule frag_shader_module(global_, "cube_depth.frag", descriptor_set_manager.GetLayouts());

			pipelines_.push_back(GraphicsPipeline(global_, *cube_shadow_map_node, vert_shader_module, geom_shader_module, frag_shader_module, extents, PrimitiveProps::kOpaque, GraphicsPipeline::EParams::kDepthBias));
			cube_shadow_map_node->AddPipeline(pipelines_.back());
		}
	}
}