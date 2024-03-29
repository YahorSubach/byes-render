#ifndef RENDER_ENGINE_RENDER_RENDER_GRAPH_H_
#define RENDER_ENGINE_RENDER_RENDER_GRAPH_H_


#include "vulkan/vulkan.h"

#include <vector>
#include <map>

#include "render/data_types.h"
#include "render/descriptor_sets_manager.h"
#include "render/framebuffer.h"
#include "render/image.h"
#include "render/image_view.h"
#include "render/object_base.h"
#include "render/scene.h"


namespace render
{
	struct FrameInfo
	{
		const Framebuffer& swapchain_framebuffer;
		const Image& swapchain_image;
		uint32_t swapchain_image_index;
		uint32_t frame_index;
	};

	class GraphicsPipeline;

	//struct RenderModel
	//{
	//	RenderModelCategory category;

	//	const GraphicsPipeline& pipeline;

	//	uint32_t vertex_count;

	//	const std::map<DescriptorSetType, VkDescriptorSet>& descriptor_sets;

	//	std::vector<VkBuffer> vertex_buffers;
	//	std::vector<VkDeviceSize> vertex_buffers_offsets;

	//	std::optional<std::pair<VkBuffer, uint32_t>> index_buffer_and_offset;
	//};

	class GraphicsPipeline;
	class RenderGraph2;

	class RenderNode
	{
	public:

		struct Attachment;

		struct Dependency
		{
			const Attachment& from_attachment;
			const RenderNode& to_node;

			DescriptorSetType descriptor_set_type;
			int descriptor_set_binding_index = -1;
		};

		struct Attachment
		{
		private:
			struct DescriptorSetForwarder
			{
				Attachment& attachment;
				DescriptorSetType type;

				int descriptor_set_binding_index = -1;

				DescriptorSetForwarder& operator>>(int binding_index);
				Attachment& operator>>(RenderNode& node_to_forward);
			};

		public:

			std::string name;
			FormatType format_type;
			//const Extent& extent;
			bool is_swapchain_image;

			RenderNode& node;

			uint32_t layers_cnt = 1;

			util::NullableRef<Dependency> depends_on;

			DescriptorSetForwarder operator>>(DescriptorSetType desc_type);
			Attachment& operator>>(render::RenderNode& node_to_forward);

			Attachment& ForwardAsAttachment(render::RenderNode& to_node);
			Attachment& ForwardAsSampled(render::RenderNode& to_node, DescriptorSetType set_type, int binding_index);

			std::vector<Dependency> to_dependencies;
		};


		RenderNode(const RenderGraph2& render_graph, const std::string& name, const ExtentType& extent);

		Attachment& Attach(const std::string& name, FormatType format_type, uint32_t layers_cnt = 1);
		Attachment& AttachSwapchain();
		Attachment& GetAttachment(const std::string& name);

		const std::vector<Attachment>& GetAttachments() const;

		void AddPipeline(const GraphicsPipeline& pipeline);
		const std::vector<std::reference_wrapper<const GraphicsPipeline>>& GetPipelines() const;
		void ClearPipelines();

		const std::string& GetName() const;
		void BuildRenderPass(const Global& global, const Formats& formats);
		/*void AddDependency(Dependency dependency);*/

		const RenderPass& GetRenderPass() const;
		const ExtentType& GetExtentType() const;

		//std::vector<std::reference_wrapper<Dependency>> depends_on;

		bool use_swapchain_framebuffer;

		int order;

		PrimitiveFlags required_primitive_flags;

	private:
		const RenderGraph2& render_graph_;
		ExtentType extent_type_;
		std::string name_;
		std::vector<Attachment> attachments_;
		std::vector<std::reference_wrapper<const GraphicsPipeline>> pipelines_;
		std::optional<RenderPass> render_pass_;

		/*std::vector<Dependency> to_dependencies_;*/
		//std::vector<Dependency> from_dependencies_;
	};

	class RenderGraph2
	{
	public:
		RenderGraph2();

		//TODO 

		RenderNode& AddNode(const std::string& name, ExtentType extent_type);
		void BuildRenderPasses(const Global& global, const Formats& formats);
		void ClearPipelines();

		const std::map<std::string, RenderNode>& GetNodes() const;

	private:
		std::map<std::string, RenderNode> nodes_;

	};

	class Scene;

	class RenderGraphHandler : RenderObjBase<int*>
	{
	public:

		RenderGraphHandler(const Global& global, const RenderGraph2& render_graph, const Extents& extents, const Formats& formats, DescriptorSetsManager& desc_set_manager);

		bool FillCommandBuffer(VkCommandBuffer command_buffer, const FrameInfo& frame_info, const Scene& scene) const;

	private:

#ifndef NDEBUG1
		class Marker
		{
			VkCommandBuffer command_buffer_;
		public:
			Marker(VkCommandBuffer command_buffer, const std::string& name);
			~Marker();
		};

		class MarkerObject
		{
			VkCommandBuffer command_buffer_;
		public:
			MarkerObject(VkCommandBuffer command_buffer, const std::string& name);
			~MarkerObject();
		};
#endif

		void ProcessDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout, const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets, const std::map<DescriptorSetType, VkDescriptorSet>& holder_desc_sets) const;

		struct AttachmentImage
		{
			FormatType format_type;
			Image image;
			ImageView image_view;
		};

		struct RenderNodeData
		{
			std::optional<Framebuffer> frambuffer;
			std::map<DescriptorSetType, VkDescriptorSet> descriptor_sets;
		};

		std::map<std::string, AttachmentImage> attachment_images_;
		std::map<std::string, RenderNodeData> node_data_;;
		const RenderGraph2& render_graph_;
		Sampler nearest_sampler_;
	};






	//class RenderGraph : public RenderObjBase<int*>
	//{
	//public:
	//	RenderGraph(const Global& global, const RenderSetup& render_setup, ModelSceneDescSetHolder& scene);

	//	RenderGraph(const RenderGraph&) = delete;
	//	RenderGraph(RenderGraph&&) = default;

	//	RenderGraph& operator=(const RenderGraph&) = delete;
	//	RenderGraph& operator=(RenderGraph&&) = default;


	//	bool FillCommandBuffer(VkCommandBuffer command_buffer, const Framebuffer& swapchain_framebuffer, const std::map<DescriptorSetType, VkDescriptorSet>& scene_ds, const std::vector<RenderModel>& render_models) const;


	//	virtual ~RenderGraph() override;
	//

	//private:

	//	void ProcessDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout, const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets, const std::map<DescriptorSetType, VkDescriptorSet>& holder_desc_sets) const;

	//	struct RenderPassNode
	//	{
	//		RenderModelCategoryFlags category_flags;
	//		util::NullableRef<const Framebuffer> framebuffer;
	//		std::vector<std::reference_wrapper<const GraphicsPipeline>> pipelines;
	//	};


	//	class RenderBatch
	//	{
	//	public:
	//		struct Dependency
	//		{
	//			const RenderBatch& batch;
	//			util::NullableRef<const Image> image;
	//			bool as_samped;
	//		};

	//		std::vector<RenderPassNode> render_pass_nodes;
	//		
	//		void AddDependencyAsSampled(const RenderBatch& batch, const Image& image);
	//		void AddDependencyAsAttachment(const RenderBatch& batch, const Image& image);
	//		void AddSwapchainDependencyAsSampled(const RenderBatch& batch);
	//		void AddSwapchainDependencyAsAttachment(const RenderBatch& batch);

	//		mutable bool processed;

	//		std::vector<Dependency> dependencies;

	//	};

	//	struct RenderCollection
	//	{
	//		std::pair<Image&, ImageView&> CreateImage(const Global& global, VkFormat format, Extent extent);
	//		Framebuffer& CreateFramebuffer(const Global& global, Extent extent, const RenderPass& render_pass);
	//		RenderBatch& CreateBatch();
	//		std::vector<Image> images;
	//		std::vector<ImageView> image_views;
	//		std::vector<Framebuffer> frambuffers;
	//		std::vector<RenderBatch> render_batches;
	//	};

	//	RenderCollection collection_;
	//};
}
#endif  // RENDER_ENGINE_RENDER_RENDER_GRAPH_H_