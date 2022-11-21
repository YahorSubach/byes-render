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
#include "render/scene.h"


namespace render
{
	class RenderGraph : public RenderObjBase<int*>
	{
	public:
		RenderGraph(const DeviceConfiguration& device_cfg, const RenderSetup& render_setup, ModelSceneDescSetHolder& scene);

		RenderGraph(const RenderGraph&) = delete;
		RenderGraph(RenderGraph&&) = default;

		RenderGraph& operator=(const RenderGraph&) = delete;
		RenderGraph& operator=(RenderGraph&&) = default;


		bool FillCommandBuffer(VkCommandBuffer command_buffer, const Framebuffer& swapchain_framebuffer, const SceneRenderNode& scene) const;


		virtual ~RenderGraph() override;
	



	private:

		void ProcessDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout, const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets, const std::map<DescriptorSetType, VkDescriptorSet>& holder_desc_sets) const;

		struct RenderPassNode
		{
			const RenderPass& render_pass;
			stl_util::NullableRef<const Framebuffer> framebuffer;
			std::vector<std::reference_wrapper<const GraphicsPipeline>> pipelines;
		};

		struct RenderBatch
		{
			std::vector<RenderPassNode> render_passes;
			std::vector<std::pair<const RenderBatch&, const Image&>> dependencies;
			mutable bool processed;
		};

		struct RenderCollection
		{
			std::pair<Image&, ImageView&> CreateImage(const DeviceConfiguration& device_cfg, VkFormat format, Extent extent);
			Framebuffer& CreateFramebuffer(const DeviceConfiguration& device_cfg, Extent extent, const RenderPass& render_pass);
			RenderBatch& CreateBatch();
			std::vector<Image> images;
			std::vector<ImageView> image_views;
			std::vector<Framebuffer> frambuffers;
			std::vector<RenderBatch> render_batches;
		};

		RenderCollection collection_;
	};
}
#endif  // RENDER_ENGINE_RENDER_RENDER_GRAPH_H_