#ifndef RENDER_ENGINE_RENDER_RENDER_SETUP_H_
#define RENDER_ENGINE_RENDER_RENDER_SETUP_H_

#include <array>
#include <map>
#include <vector>

#include "vulkan/vulkan.h"

#include "render/data_types.h"
#include "render/graphics_pipeline.h"
#include "render/descriptor_sets_manager.h"

#include "render/render_graph.h"

namespace render
{
	class GraphicsPipeline;

	class RenderSetup: RenderObjBase<bool>
	{
	public:

		RenderSetup(const Global& global);

		void BuildRenderPasses(const Formats& formats);

		void InitPipelines(const DescriptorSetsManager& descriptor_set_manager, const std::array<Extent, kExtentTypeCnt>& extents);

		const RenderGraph2& GetRenderGraph() const;
		const RenderPass& GetSwapchainRenderPass() const;

		const std::vector<GraphicsPipeline>& GetPipelines() const;
	private:

		RenderGraph2 render_graph_;

		util::NullableRef<const RenderPass> swapchain_render_pass_;

		std::vector<GraphicsPipeline> pipelines_;

		util::NullableRef<render::RenderNode> g_build_node;
		util::NullableRef<render::RenderNode> cube_shadow_map_node;
		util::NullableRef<render::RenderNode> g_collect_node;
		util::NullableRef<render::RenderNode> ui_node;

		//std::map<RenderPassId, RenderPass> render_passes_;
	};
}
#endif  // RENDER_ENGINE_RENDER_RENDER_SETUP_H_