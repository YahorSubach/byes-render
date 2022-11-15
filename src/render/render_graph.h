#ifndef RENDER_ENGINE_RENDER_RENDER_GRAPH_H_
#define RENDER_ENGINE_RENDER_RENDER_GRAPH_H_


#include "vulkan/vulkan.h"

#include <vector>

#include "render/framebuffer.h"
#include "render/image.h"
#include "render/image_view.h"
#include "render/object_base.h"
#include "render/graphics_pipeline.h"
#include "render/render_setup.h"


namespace render
{
	class RenderGraph : public RenderObjBase<int>
	{
	public:
		RenderGraph(const DeviceConfiguration& device_cfg, const RenderSetup& render_setup, const Image& presentation_image);

		RenderGraph(const RenderGraph&) = delete;
		RenderGraph(RenderGraph&&) = default;

		RenderGraph& operator=(const RenderGraph&) = delete;
		RenderGraph& operator=(RenderGraph&&) = default;


		bool FillCommandBuffer(VkCommandBuffer command_buffer) const;


		virtual ~RenderGraph() override;
	
		struct RenderCollection
		{
			std::pair<Image&, ImageView&> CreateImage(const DeviceConfiguration& device_cfg, VkFormat format, Extent extent);
			Framebuffer& CreateFramebuffer(const DeviceConfiguration& device_cfg, Extent extent, const RenderPass& render_pass);
			std::vector<Image> images;
			std::vector<ImageView> image_views;
			std::vector<Framebuffer> frambuffers;
		};


	private:

		struct RenderPassNode
		{
			const RenderPass& render_pass;
			std::vector<std::reference_wrapper<GraphicsPipeline>> pipelines;
		};

		struct RenderNode
		{
			std::vector<RenderPassNode> render_passes;
			std::vector<std::reference_wrapper<GraphicsPipeline>> pipelines;
		};

		struct RenderNodeAttachmetnDependency
		{
			const ImageView& attachment;
			const RenderNode& depend_node;
		};

		

		ImageView presentation_image_view_;
		RenderCollection collection_;
	};
}
#endif  // RENDER_ENGINE_RENDER_RENDER_GRAPH_H_