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
#include "render/graphics_pipeline.h"
#include "render/render_setup.h"
#include "render/scene.h"


namespace render
{
	class RenderGraph2 : public RenderObjBase<int*>
	{
	public:

		class Node;
		struct Attachment;

		struct Dependency
		{
			const Attachment& from_attachment;
			const Node& to_node;

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
				Attachment& operator>>(Node& node_to_forward);
			};

		public:

			std::string name;
			Format format;
			Extent extent;
			
			Node& node;

			stl_util::NullableRef<Dependency> depends_on;

			DescriptorSetForwarder operator>>(DescriptorSetType desc_type);
			Attachment& operator>>(Node& node_to_forward);

			Attachment& ForwardAsAttachment(Node& to_node);
			Attachment& ForwardAsSampled(Node& to_node, DescriptorSetType set_type, int binding_index);

			std::vector<Dependency> to_dependencies;
		};


		class Node
		{
		public:

			Node(const RenderGraph2& render_graph, const std::string& name, const Extent& extent, RenderModelCategoryFlags category_flags);

			Attachment& AddAttachment(const std::string& name, Format format, Extent extent);
			Attachment& GetAttachment(const std::string& name);

			const std::map<std::string, Attachment>& GetAttachments() const;

			const std::string& GetName() const;
			void Build();
			/*void AddDependency(Dependency dependency);*/

			const RenderPass& GetRenderPass() const;
			Extent GetExtent() const;

			//std::vector<std::reference_wrapper<Dependency>> depends_on;

			int order;
			RenderModelCategoryFlags category_flags;
		private:
			const RenderGraph2& render_graph_;
			const Extent& extent_;
			std::string name_;
			std::map<std::string, Attachment> attachments_;
			std::optional<RenderPass> render_pass_;

			/*std::vector<Dependency> to_dependencies_;*/
			//std::vector<Dependency> from_dependencies_;
		};

		RenderGraph2(const DeviceConfiguration device_cfg);

		Node& AddNode(const std::string& name, const Extent& extent, RenderModelCategoryFlags category_flags);
		void Build();


		
		const std::map<std::string, Node>& GetNodes() const;

	private:
		std::map<std::string, Node> nodes_;

	};

	class RenderGraphHandler: RenderObjBase<int*>
	{
	public:

		RenderGraphHandler(const DeviceConfiguration& device_cfg, const RenderGraph2& render_graph, DescriptorSetsManager& desc_set_manager);

		bool FillCommandBuffer(VkCommandBuffer command_buffer, const Framebuffer& swapchain_framebuffer, const std::map<DescriptorSetType, VkDescriptorSet>& scene_ds, const std::vector<RenderModel>& render_models) const;

	private:

		void ProcessDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout, const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets, const std::map<DescriptorSetType, VkDescriptorSet>& holder_desc_sets) const;

		struct AttachmentImage
		{
			VkFormat format;
			Image image;
			ImageView image_view;
		};

		struct RenderNodeData
		{
			Framebuffer frambuffer;
			std::map<DescriptorSetType, VkDescriptorSet> descriptor_sets;
		};

		std::map<std::string, AttachmentImage> attachment_images_;
		std::map<std::string, RenderNodeData> node_data_;;
		const RenderGraph2& render_graph_;
	};



	enum class RenderModelCategory
	{
		kRenderModel,
		kViewport,
		kUIShape
	};

	using RenderModelCategoryFlags = stl_util::EnumFlags<RenderModelCategory>;


	struct RenderModel
	{
		RenderModelCategory category;

		const GraphicsPipeline& pipeline;

		const std::map<DescriptorSetType, VkDescriptorSet>& descriptor_sets;
		
		std::vector<VkBuffer> vertex_buffers;
		std::vector<VkDeviceSize> vertex_buffers_offsets;

		std::optional<std::pair<VkBuffer, uint32_t>> index_buffer_and_offset;

		uint32_t vertex_count;
	};


	//class RenderGraph : public RenderObjBase<int*>
	//{
	//public:
	//	RenderGraph(const DeviceConfiguration& device_cfg, const RenderSetup& render_setup, ModelSceneDescSetHolder& scene);

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
	//		stl_util::NullableRef<const Framebuffer> framebuffer;
	//		std::vector<std::reference_wrapper<const GraphicsPipeline>> pipelines;
	//	};


	//	class RenderBatch
	//	{
	//	public:
	//		struct Dependency
	//		{
	//			const RenderBatch& batch;
	//			stl_util::NullableRef<const Image> image;
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
	//		std::pair<Image&, ImageView&> CreateImage(const DeviceConfiguration& device_cfg, VkFormat format, Extent extent);
	//		Framebuffer& CreateFramebuffer(const DeviceConfiguration& device_cfg, Extent extent, const RenderPass& render_pass);
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