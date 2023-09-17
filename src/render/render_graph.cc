#include "render_graph.h"

#include <stack>
#include <queue>
#include <set>

#include "global.h"

int print_sets = 0;

namespace render
{
	//
	//
	//RenderGraph::RenderGraph(const Global& global, const RenderSetup& render_setup, ModelSceneDescSetHolder& scene) : RenderObjBase(global)
	//{
	//	//TODO fuck! You really have to change this return of push back logic
	//	collection_.images.reserve(16);
	//	collection_.image_views.reserve(16);
	//	collection_.frambuffers.reserve(16);
	//	collection_.render_batches.reserve(16);
	//
	//
	//	auto&& [g_albedo_image, g_albedo_image_view] = collection_.CreateImage(global_, global_.high_range_color_format, global_.presentation_extent);
	//	auto&& [g_position_image, g_position_image_view] = collection_.CreateImage(global_, global_.high_range_color_format, global_.presentation_extent);
	//	auto&& [g_normal_image, g_normal_image_view] = collection_.CreateImage(global_, global_.high_range_color_format, global_.presentation_extent);
	//	auto&& [g_metallic_roughness_image, g_metallic_roughness_image_view] = collection_.CreateImage(global_, global_.high_range_color_format, global_.presentation_extent);
	//
	//	auto&& [g_depth_image, g_depth_image_view] = collection_.CreateImage(global_, global_.depth_map_format, global_.presentation_extent);
	//
	//	auto&& [g_shadowmap_image, g_shadowmap_image_view] = collection_.CreateImage(global_, global_.depth_map_format, global_.shadowmap_extent);
	//
	//
	//	auto&& shadowmap_framebuffer = collection_.CreateFramebuffer(global_, global_.shadowmap_extent, render_setup.GetRenderPass(RenderPassId::kBuildDepthmap));
	//	shadowmap_framebuffer.Attach("shadowmap", g_shadowmap_image_view);
	//
	//
	//	auto&& g_framebuffer = collection_.CreateFramebuffer(global_, global_.presentation_extent, render_setup.GetRenderPass(RenderPassId::kBuildGBuffers));
	//	//auto&& presentation_framebuffer = collection_.CreateFramebuffer(global_, global_.presentation_extent, render_setup.GetRenderPass(RenderPassId::kSimpleRenderToScreen));
	//
	//	g_framebuffer.Attach("g_albedo", g_albedo_image_view);
	//	g_framebuffer.Attach("g_position", g_position_image_view);
	//	g_framebuffer.Attach("g_normal", g_normal_image_view);
	//	g_framebuffer.Attach("g_metal_rough", g_metallic_roughness_image_view);
	//	g_framebuffer.Attach("g_depth", g_depth_image_view);
	//
	//	//presentation_framebuffer.Attach("swapchain_image", presentation_image_view_);
	//
	//
	//	auto&& shadowmap_batch = collection_.CreateBatch();
	//	auto&& g_fill_batch = collection_.CreateBatch();
	//	auto&& g_collect_batch = collection_.CreateBatch();
	//	auto&& ui_batch = collection_.CreateBatch();
	//
	//
	//	shadowmap_batch.render_pass_nodes.push_back(RenderPassNode{ RenderModelCategory::kRenderModel, shadowmap_framebuffer, {render_setup.GetPipeline(PipelineId::kDepth)} });
	//	g_fill_batch.render_pass_nodes.push_back(RenderPassNode{ RenderModelCategory::kRenderModel,g_framebuffer, {render_setup.GetPipeline(PipelineId::kBuildGBuffers)} });
	//	g_collect_batch.render_pass_nodes.push_back(RenderPassNode{ RenderModelCategory::kViewport,{}, {render_setup.GetPipeline(PipelineId::kCollectGBuffers)} });
	//	ui_batch.render_pass_nodes.push_back(RenderPassNode{ RenderModelCategory::kUIShape, {}, {render_setup.GetPipeline(PipelineId::kUI)} });
	//
	//
	//	shadowmap_batch.AddDependencyAsSampled(g_fill_batch, g_shadowmap_image);
	//
	//	g_fill_batch.AddDependencyAsSampled(g_collect_batch, g_albedo_image);
	//	g_fill_batch.AddDependencyAsSampled(g_collect_batch, g_position_image);
	//	g_fill_batch.AddDependencyAsSampled(g_collect_batch, g_normal_image);
	//	g_fill_batch.AddDependencyAsSampled(g_collect_batch, g_metallic_roughness_image);
	//	g_fill_batch.AddDependencyAsSampled(g_collect_batch, g_depth_image);
	//
	//	g_collect_batch.AddSwapchainDependencyAsAttachment(ui_batch);
	//
	//	scene.g_albedo_image = g_albedo_image;
	//	scene.g_position_image = g_position_image;
	//	scene.g_normal_image = g_normal_image;
	//	scene.g_metal_rough_image = g_metallic_roughness_image;
	//
	//	scene.shadowmap_image = g_shadowmap_image;
	//}
	//
	//RenderGraph::~RenderGraph()
	//{
	//}
	//
	//std::pair<Image&, ImageView&> RenderGraph::RenderCollection::CreateImage(const Global& global, VkFormat format, Extent extent)
	//{
	//	images.push_back(Image(global, format, extent));
	//	image_views.push_back(ImageView(global, images.back()));
	//	return { images.back(), image_views.back() };
	//}
	//
	//Framebuffer& RenderGraph::RenderCollection::CreateFramebuffer(const Global& global, Extent extent, const RenderPass& render_pass)
	//{
	//	frambuffers.push_back(Framebuffer(global, extent, render_pass));
	//	return frambuffers.back();
	//}
	//
	//RenderGraph::RenderBatch& RenderGraph::RenderCollection::CreateBatch()
	//{
	//	render_batches.push_back({});
	//	return render_batches.back();
	//}
	//
	//bool RenderGraph::FillCommandBuffer(VkCommandBuffer command_buffer, const Framebuffer& swapchain_framebuffer, const std::map<DescriptorSetType, VkDescriptorSet>& scene_ds, const std::vector<RenderModel>& render_models) const
	//{
	//	VkCommandBufferBeginInfo begin_info{};
	//	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//	begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Optional
	//	begin_info.pInheritanceInfo = nullptr; // Optional
	//
	//	VkResult result;
	//
	//	if (result = vkBeginCommandBuffer(command_buffer, &begin_info); result != VK_SUCCESS) {
	//		throw std::runtime_error("failed to begin recording command buffer!");
	//	}
	//
	//
	//	for (auto&& batch : collection_.render_batches)
	//	{
	//		batch.processed = false;
	//	}
	//
	//	std::queue<std::reference_wrapper<const RenderBatch>> batches_to_process;
	//	batches_to_process.push(collection_.render_batches.front());
	//
	//	while (!batches_to_process.empty())
	//	{
	//		auto&& current_batch = batches_to_process.front().get();
	//		batches_to_process.pop();
	//
	//		bool final_pass = false;
	//		for (auto&& render_pass_node : current_batch.render_pass_nodes)
	//		{
	//			auto frambuffer = render_pass_node.framebuffer;
	//			if (!frambuffer)
	//			{
	//				frambuffer = swapchain_framebuffer;
	//			}
	//
	//			VkRenderPassBeginInfo render_pass_begin_info{};
	//			render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	//			render_pass_begin_info.renderPass = frambuffer->GetRenderPass().GetHandle();
	//			render_pass_begin_info.framebuffer = frambuffer->GetHandle();
	//
	//			render_pass_begin_info.renderArea.offset = { 0, 0 };
	//			render_pass_begin_info.renderArea.extent = frambuffer->GetExtent();
	//
	//			std::vector<VkClearValue> clear_values(frambuffer->GetAttachmentImageViews().size());
	//			int index = 0;
	//			for (auto&& attachment : frambuffer->GetAttachmentImageViews())
	//			{
	//				if (attachment.get().CheckUsageFlag(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
	//				{
	//					clear_values[index].color = VkClearColorValue{ {0.0f, 0.0f, 0.0f, 1.0f} };
	//				}
	//				else
	//				{
	//					clear_values[index].depthStencil = { 1.0f, 0 };
	//				}
	//				index++;
	//			}
	//
	//			render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
	//			render_pass_begin_info.pClearValues = clear_values.data();
	//
	//			vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
	//
	//			for (auto&& pipeline : render_pass_node.pipelines)
	//			{
	//				vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get().GetHandle());
	//
	//				const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets = pipeline.get().GetDescriptorSets();
	//
	//				auto&& pipeline_layout = pipeline.get().GetLayout();
	//
	//				ProcessDescriptorSets(command_buffer, pipeline_layout, pipeline_desc_sets, scene_ds);
	//
	//				for (auto&& render_model : render_models)
	//				{
	//					if (/*true || */render_pass_node.category_flags.Check(render_model.category)) {
	//
	//						ProcessDescriptorSets(command_buffer, pipeline_layout, pipeline_desc_sets, render_model.descriptor_sets);
	//
	//						assert(render_model.vertex_buffers.size() == render_model.vertex_buffers_offsets.size());
	//
	//						vkCmdBindVertexBuffers(command_buffer, 0, u32(render_model.vertex_buffers.size()), render_model.vertex_buffers.data(), render_model.vertex_buffers_offsets.data());
	//
	//						if (render_model.index_buffer_and_offset.has_value())
	//						{
	//							vkCmdBindIndexBuffer(command_buffer, render_model.index_buffer_and_offset->first, render_model.index_buffer_and_offset->second, VK_INDEX_TYPE_UINT16);
	//							vkCmdDrawIndexed(command_buffer, render_model.vertex_count, 1, 0, 0, 0);
	//						}
	//						else
	//						{
	//							vkCmdDraw(command_buffer, render_model.vertex_count, 1, 0, 0);
	//						}
	//					}
	//				}
	//			}
	//
	//			vkCmdEndRenderPass(command_buffer);
	//
	//			final_pass = true;
	//		}
	//
	//		std::vector<VkImageMemoryBarrier2> vk_barriers;
	//		for (auto&& dependency : current_batch.dependencies)
	//		{
	//			if (!dependency.batch.processed)
	//			{
	//				dependency.batch.processed = true;
	//				batches_to_process.push(dependency.batch);
	//			}
	//
	//			VkStructureType            sType;
	//			const void* pNext;
	//			VkPipelineStageFlags2      srcStageMask;
	//			VkAccessFlags2             srcAccessMask;
	//			VkPipelineStageFlags2      dstStageMask;
	//			VkAccessFlags2             dstAccessMask;
	//			VkImageLayout              oldLayout;
	//			VkImageLayout              newLayout;
	//			uint32_t                   srcQueueFamilyIndex;
	//			uint32_t                   dstQueueFamilyIndex;
	//			VkImage                    image;
	//			VkImageSubresourceRange    subresourceRange;
	//
	//			util::NullableRef<const Image> barrier_image = dependency.image;
	//
	//			if (!barrier_image)
	//			{
	//				barrier_image = swapchain_framebuffer.GetAttachment("swapchain_image").GetImage();
	//			}
	//
	//			VkImageMemoryBarrier2 barrier;
	//			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	//			barrier.pNext = nullptr;
	//			barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT; // < ---
	//			barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
	//			barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
	//			barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
	//
	//			if (dependency.as_samped)
	//			{
	//				barrier.oldLayout = barrier_image->CheckUsageFlag(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	//				barrier.newLayout = barrier_image->CheckUsageFlag(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	//			}
	//			else
	//			{
	//				barrier.oldLayout = barrier_image->CheckUsageFlag(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	//				barrier.newLayout = barrier_image->CheckUsageFlag(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	//			}
	//			
	//			barrier.srcQueueFamilyIndex = global_.graphics_queue_index;
	//			barrier.dstQueueFamilyIndex = global_.graphics_queue_index;
	//			barrier.image = barrier_image->GetHandle();
	//			barrier.subresourceRange.aspectMask = barrier_image->CheckUsageFlag(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
	//			barrier.subresourceRange.baseMipLevel = 0;
	//			barrier.subresourceRange.levelCount = 1;
	//			barrier.subresourceRange.baseArrayLayer = 0;
	//			barrier.subresourceRange.layerCount = 1;
	//
	//			vk_barriers.push_back(barrier);
	//		}
	//
	//		if (vk_barriers.size() > 0)
	//		{
	//			VkDependencyInfo vk_dependency_info;
	//			vk_dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	//			vk_dependency_info.pNext = nullptr;
	//			vk_dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	//			vk_dependency_info.memoryBarrierCount = 0;
	//			vk_dependency_info.pMemoryBarriers = nullptr;
	//			vk_dependency_info.bufferMemoryBarrierCount = 0;
	//			vk_dependency_info.pBufferMemoryBarriers = nullptr;
	//			vk_dependency_info.imageMemoryBarrierCount = vk_barriers.size();
	//			vk_dependency_info.pImageMemoryBarriers = vk_barriers.data();
	//
	//			vkCmdPipelineBarrier2(command_buffer, &vk_dependency_info);
	//		}
	//	}
	//	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
	//		throw std::runtime_error("failed to record command buffer!");
	//	}
	//}
	//
	//
	//void RenderGraph::ProcessDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout, const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets, const std::map<DescriptorSetType, VkDescriptorSet>& holder_desc_sets) const
	//{
	//	uint32_t sequence_begin = 0;
	//	std::vector<VkDescriptorSet> desc_sets_to_bind;
	//
	//	for (auto&& [set_id, set_layout] : pipeline_desc_sets)
	//	{
	//		if (auto&& holder_set = holder_desc_sets.find(set_layout.GetType()); holder_set != holder_desc_sets.end())
	//		{
	//			if (desc_sets_to_bind.size() == 0)
	//			{
	//				sequence_begin = set_id;
	//			}
	//
	//			desc_sets_to_bind.push_back(holder_set->second);
	//		}
	//		else
	//		{
	//			if (desc_sets_to_bind.size() != 0)
	//			{
	//				vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, sequence_begin, u32(desc_sets_to_bind.size()), desc_sets_to_bind.data(), 0, nullptr);
	//				desc_sets_to_bind.clear();
	//			}
	//		}
	//	}
	//
	//	if (desc_sets_to_bind.size() != 0)
	//	{
	//		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, sequence_begin, u32(desc_sets_to_bind.size()), desc_sets_to_bind.data(), 0, nullptr);
	//		desc_sets_to_bind.clear();
	//	}
	//}
	//
	//void RenderGraph::RenderBatch::AddDependencyAsSampled(const RenderBatch& batch, const Image& image)
	//{
	//	dependencies.push_back({ batch, image, true});
	//}
	//
	//void RenderGraph::RenderBatch::AddDependencyAsAttachment(const RenderBatch& batch, const Image& image)
	//{
	//	dependencies.push_back({ batch, image, false });
	//}
	//
	//void RenderGraph::RenderBatch::AddSwapchainDependencyAsSampled(const RenderBatch& batch)
	//{
	//	dependencies.push_back({ batch, {}, true });
	//}
	//
	//void RenderGraph::RenderBatch::AddSwapchainDependencyAsAttachment(const RenderBatch& batch)
	//{
	//	dependencies.push_back({ batch, {}, false });
	//}


	RenderGraph2::RenderGraph2(const Global& global) : RenderObjBase(global)
	{
	}

	RenderNode& RenderGraph2::AddNode(const std::string& name, ExtentType extent_type)
	{
		auto [it, success] = nodes_.insert({ name, RenderNode{*this, name, extent_type} });
		assert(success);
		return it->second;
	}

	void RenderGraph2::Build()
	{
		for (auto&& [name, node] : nodes_)
		{
			node.Build();
		}
	}

	void RenderGraph2::ClearPipelines()
	{
		for (auto&& [name, node] : nodes_)
		{
			node.ClearPipelines();
		}
	}

	const std::map<std::string, RenderNode>& RenderGraph2::GetNodes() const
	{
		return nodes_;
	}

	RenderNode::RenderNode(const RenderGraph2& render_graph, const std::string& name, const ExtentType& extent_type) :
		name_(name), render_graph_(render_graph), extent_type_(extent_type), order(0), use_swapchain_framebuffer(false)
	{
		attachments_.reserve(16);
	}

	RenderNode::Attachment& RenderNode::Attach(const std::string& name, Format format)
	{
		attachments_.push_back({ name, format, false, *this });
		//assert(success);
		return attachments_.back();
	}

	RenderNode::Attachment& RenderNode::AttachSwapchain()
	{
		attachments_.push_back({ kSwapchainAttachmentName, render_graph_.GetDeviceCfg().presentation_format, true, *this });
		//assert(success);
		return attachments_.back();
	}

	RenderNode::Attachment& RenderNode::GetAttachment(const std::string& name)
	{
		for (auto&& attachment : attachments_)
		{
			if (attachment.name == name)
				return attachment;
		}

		return attachments_[0];
	}


	const std::vector<RenderNode::Attachment>& RenderNode::GetAttachments() const
	{
		return attachments_;
	}

	void RenderNode::AddPipeline(const GraphicsPipeline& pipeline)
	{
		pipelines_.push_back(pipeline);
		required_primitive_flags.Set(pipeline.GetRequiredPrimitiveFlags());
	}

	const std::vector<std::reference_wrapper<const GraphicsPipeline>>& RenderNode::GetPipelines() const
	{
		return pipelines_;
	}

	void RenderNode::ClearPipelines()
	{
		pipelines_.clear();
	}

	const std::string& RenderNode::GetName() const
	{
		return name_;
	}

	void RenderNode::Build()
	{
		render_pass_.emplace(RenderPass(render_graph_.GetDeviceCfg(), *this));
	}
	const RenderPass& RenderNode::GetRenderPass() const
	{
		assert(render_pass_);
		return render_pass_.value();
	}

	const ExtentType& RenderNode::GetExtentType() const
	{
		return extent_type_;
	}
	//void RenderNode::AddDependency(Dependency dependency)
	//{
	//	to_dependencies_.push_back(dependency);
	//}

	RenderNode::Attachment::DescriptorSetForwarder RenderNode::Attachment::operator>>(DescriptorSetType desc_type)
	{
		return { *this, desc_type };
	}

	RenderNode::Attachment& RenderNode::Attachment::operator>>(RenderNode& node_to_forward)
	{
		return ForwardAsAttachment(node_to_forward);
	}

	RenderNode::Attachment& RenderNode::Attachment::ForwardAsAttachment(RenderNode& to_node)
	{
		assert(node.extent_type_ == to_node.extent_type_);
		//RenderNode.AddDependency({ *this, to_node, true });
		to_dependencies.push_back({ *this, to_node, DescriptorSetType::None });

		auto&& new_attachment = to_node.Attach(name, format);
		new_attachment.is_swapchain_image = is_swapchain_image;
		new_attachment.depends_on = to_dependencies.back();
		to_node.order = std::max(to_node.order, node.order + 1);
		//to_node.depends = true;

		return new_attachment;
	}

	RenderNode::Attachment& RenderNode::Attachment::ForwardAsSampled(RenderNode& to_node, DescriptorSetType set_type, int binding_index)
	{
		//RenderNode.AddDependency({ *this, to_node, false });
		to_dependencies.push_back({ *this, to_node, set_type, binding_index });
		to_node.order = std::max(to_node.order, node.order + 1);
		//to_node.depends = true;
		return *this;
	}

	RenderNode::Attachment::DescriptorSetForwarder& RenderNode::Attachment::DescriptorSetForwarder::operator>>(int binding_index)
	{
		descriptor_set_binding_index = binding_index;
		return *this;
	}

	RenderNode::Attachment& RenderNode::Attachment::DescriptorSetForwarder::operator>>(RenderNode& node_to_forward)
	{
		return attachment.ForwardAsSampled(node_to_forward, type, descriptor_set_binding_index);
	}

	RenderGraphHandler::RenderGraphHandler(const Global& global, const RenderGraph2& render_graph, const std::array<Extent, kExtentTypeCnt>& extents, DescriptorSetsManager& desc_set_manager) :
		RenderObjBase(global), render_graph_(render_graph), nearest_sampler_(global, 0, Sampler::AddressMode::kRepeat, true)
	{
		std::map<std::string, std::map<DescriptorSetType, std::map<int, const AttachmentImage&>>> desc_set_images;

		for (auto&& [node_name, RenderNode] : render_graph.GetNodes())
		{
			Framebuffer::ConstructParams framebuffer_params{ RenderNode.GetRenderPass(), extents[u32(RenderNode.GetExtentType())] };

			for (auto&& attachment : RenderNode.GetAttachments())
			{
				auto&& it = attachment_images_.find(attachment.name);

				if (it != attachment_images_.end())
				{
					framebuffer_params.attachments.push_back(it->second.image_view);
					continue;
				}

				Image image(global, attachment.format, extents[u32(RenderNode.GetExtentType())]);

				if (attachment.format == global.depth_map_format)
				{
					image.AddUsageFlag(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
				}
				else
				{
					image.AddUsageFlag(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
				}

				for (auto&& dependency : attachment.to_dependencies)
				{
					if (dependency.descriptor_set_type != DescriptorSetType::None)
					{
						image.AddUsageFlag(VK_IMAGE_USAGE_SAMPLED_BIT);
					}
				}

				ImageView image_view(global, image);

				auto res = attachment_images_.insert({ attachment.name, AttachmentImage{attachment.format, std::move(image), std::move(image_view)} });
				framebuffer_params.attachments.push_back(res.first->second.image_view);

				for (auto&& dependency : attachment.to_dependencies)
				{
					if (dependency.descriptor_set_type != DescriptorSetType::None)
					{
						desc_set_images[dependency.to_node.GetName()][dependency.descriptor_set_type].emplace(dependency.descriptor_set_binding_index, res.first->second);
					}
				}
			}

			auto&& [it, success] = node_data_.emplace(node_name, RenderNodeData{ {} });
			assert(success);

			if (!RenderNode.use_swapchain_framebuffer)
			{
				it->second.frambuffer.emplace(global, framebuffer_params);
			}
		}

		for (auto&& [node_name, descriptors] : desc_set_images)
		{
			auto&& node_data = node_data_.at(node_name);

			for (auto&& [desc_type, desc_images] : descriptors)
			{
				VkDescriptorSet vk_descriptor_set = desc_set_manager.GetFreeDescriptor(desc_type);

				std::vector<VkWriteDescriptorSet> writes(desc_images.size());
				std::vector<VkDescriptorImageInfo> image_infos(desc_images.size());

				for (auto&& [binding_index, binding_att_image] : desc_images)
				{
					image_infos[binding_index].sampler = nearest_sampler_.GetHandle();
					image_infos[binding_index].imageLayout = binding_att_image.format == global.depth_map_format ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					image_infos[binding_index].imageView = binding_att_image.image_view.GetHandle();

					writes[binding_index].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writes[binding_index].pNext = nullptr;
					writes[binding_index].dstSet = vk_descriptor_set;
					writes[binding_index].dstBinding = binding_index;
					writes[binding_index].dstArrayElement = 0;
					writes[binding_index].descriptorCount = 1;
					writes[binding_index].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					writes[binding_index].pImageInfo = &image_infos[binding_index];
					writes[binding_index].pBufferInfo = nullptr;
					writes[binding_index].pTexelBufferView = nullptr;
				}

				vkUpdateDescriptorSets(global.logical_device, u32(writes.size()), writes.data(), 0, nullptr);

				node_data.descriptor_sets.emplace(desc_type, vk_descriptor_set);
			}
		}
	}

	bool RenderGraphHandler::FillCommandBuffer(VkCommandBuffer command_buffer, const FrameInfo& frame_info, /*Scene::*/Scene& scene) const
	{
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Optional
		begin_info.pInheritanceInfo = nullptr; // Optional

		VkResult result;

		if (result = vkBeginCommandBuffer(command_buffer, &begin_info); result != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		std::set<std::string> processed_nodes;

		int order = 0;

		while (true)
		{
			bool stop = true;
			std::vector<std::reference_wrapper<const RenderNode::Dependency>> dependencies;
			for (auto&& [node_name, render_node] : render_graph_.GetNodes())
			{
				if (render_node.order != order)
					continue;

				stop = false;

				util::NullableRef<const Framebuffer> framebuffer = frame_info.swapchain_framebuffer;
				std::map<DescriptorSetType, VkDescriptorSet> node_desc_set;
				if (auto&& it = node_data_.find(node_name); it != node_data_.end())
				{
					if (it->second.frambuffer)
					{
						framebuffer = it->second.frambuffer;
					}
					node_desc_set = it->second.descriptor_sets;
				}

				VkRenderPassBeginInfo render_pass_begin_info{};
				render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				render_pass_begin_info.renderPass = render_node.GetRenderPass().GetHandle();
				render_pass_begin_info.framebuffer = framebuffer->GetHandle();

				render_pass_begin_info.renderArea.offset = { 0, 0 };
				render_pass_begin_info.renderArea.extent = framebuffer->GetExtent();

				for (auto&& attachment : render_node.GetAttachments())
				{
					for (auto&& dependency : attachment.to_dependencies)
					{
						dependencies.push_back(dependency);
					}
				}

				std::vector<VkClearValue> clear_values(framebuffer->GetFormats().size());
				for (int att_ind = 0; att_ind < framebuffer->GetFormats().size(); att_ind++)
				{
					if (framebuffer->GetFormats()[att_ind] == global_.depth_map_format)
					{
						clear_values[att_ind].depthStencil = { 1.0f, 0 };
					}
					else
					{
						clear_values[att_ind].color = VkClearColorValue{ {0.0f, 0.0f, 0.0f, 1.0f} };
					}
				}

				render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
				render_pass_begin_info.pClearValues = clear_values.data();

				vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

				const GraphicsPipeline* current_pipeline = nullptr;
				VkPipelineLayout pipeline_layout;
				int ind = 0;
				for (auto&& render_model_ref : scene.models_)
				{
					auto&& model = render_model_ref;
					Mesh& mesh = model.mesh;
					ind++;
					for (auto&& primitive : mesh.primitives)
					{
						auto [flags, primitive_vertex_buffers, primitive_indices] = std::visit([](auto&& primitive) { return std::tie(primitive.flags, primitive.vertex_buffers, primitive.indices); }, primitive);

						if (render_node.required_primitive_flags.Check(flags))
						{
							for (auto&& primitive_pipeline_ref : render_node.GetPipelines())
							{
								auto&& primitive_pipeline = primitive_pipeline_ref.get();

								if (!primitive_pipeline.GetRequiredPrimitiveFlags().Check(flags))
									continue;

								if (current_pipeline != &primitive_pipeline)
								{
									current_pipeline = &primitive_pipeline;

									vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, primitive_pipeline.GetHandle());
									const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets = primitive_pipeline.GetDescriptorSetLayouts();

									pipeline_layout = primitive_pipeline.GetLayout();


									ProcessDescriptorSets(command_buffer, pipeline_layout, pipeline_desc_sets, scene.GetDescriptorSets(frame_info.frame_index));
								}


								const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets = primitive_pipeline.GetDescriptorSetLayouts();
								ProcessDescriptorSets(command_buffer, pipeline_layout, pipeline_desc_sets, node_desc_set);
								ProcessDescriptorSets(command_buffer, pipeline_layout, pipeline_desc_sets, model.GetDescriptorSets(frame_info.frame_index));

								std::visit(
									[&](auto&& primitive)
									{
										ProcessDescriptorSets(command_buffer, pipeline_layout, pipeline_desc_sets, primitive.GetDescriptorSets(frame_info.frame_index));
									},
									primitive
								);

								std::array<VkBuffer, kVertexBufferTypesCount> vertex_buffers;
								std::array<VkDeviceSize, kVertexBufferTypesCount> vertex_buffer_offsets;
								uint32_t vertex_buffers_cnt = 0;

								bool valid = true;
								for (auto&& [vertex_binding_index, vertex_binding] : primitive_pipeline.GetVertexBindingsDescs())
								{
									for (auto&& [attr_location, attr] : vertex_binding.attributes)
									{
										if (!primitive_vertex_buffers[u32(attr.type)])
										{
											valid = false;
											break;
										}

										vertex_buffers[vertex_binding_index] = primitive_vertex_buffers[u32(attr.type)]->buffer->GetHandle();
										vertex_buffer_offsets[vertex_binding_index] = primitive_vertex_buffers[u32(attr.type)]->offset;
										vertex_buffers_cnt = std::max(vertex_buffers_cnt, vertex_binding_index + 1);
									}

									if (!valid)
										break;
								}


								if (!valid)
									continue;

								vkCmdBindVertexBuffers(command_buffer, 0, vertex_buffers_cnt, vertex_buffers.data(), vertex_buffer_offsets.data());

								if (primitive_indices)
								{
									vkCmdBindIndexBuffer(command_buffer, primitive_indices->buffer->GetHandle(), primitive_indices->offset, VK_INDEX_TYPE_UINT16);
									vkCmdDrawIndexed(command_buffer, u32(primitive_indices->count), 1, 0, 0, 0);
								}
								else
								{
									vkCmdDraw(command_buffer, u32(primitive_vertex_buffers[u32(VertexBufferType::kPOSITION)]->count), 1, 0, 0);
									int a = 1;
								}
							}
						}
					}
				}

				vkCmdEndRenderPass(command_buffer);
			}

			std::vector<VkImageMemoryBarrier2> vk_barriers;
			for (auto&& dependency : dependencies)
			{

				util::NullableRef<const Image> barrier_image = attachment_images_.at(dependency.get().from_attachment.name).image;

				if (!barrier_image)
				{
					barrier_image = frame_info.swapchain_image;
				}

				VkImageMemoryBarrier2 barrier;
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
				barrier.pNext = nullptr;
				barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT; // < ---
				barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
				barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
				barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

				if (dependency.get().descriptor_set_type != DescriptorSetType::None)
				{
					barrier.oldLayout = barrier_image->CheckUsageFlag(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					barrier.newLayout = barrier_image->CheckUsageFlag(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				}
				else
				{
					barrier.oldLayout = barrier_image->CheckUsageFlag(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					barrier.newLayout = barrier_image->CheckUsageFlag(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				}

				barrier.srcQueueFamilyIndex = 0;
				barrier.dstQueueFamilyIndex = 0;
				barrier.image = barrier_image->GetHandle();
				barrier.subresourceRange.aspectMask = barrier_image->CheckUsageFlag(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
				barrier.subresourceRange.baseMipLevel = 0;
				barrier.subresourceRange.levelCount = 1;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount = 1;

				vk_barriers.push_back(barrier);
			}

			if (vk_barriers.size() > 0)
			{
				VkDependencyInfo vk_dependency_info;
				vk_dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
				vk_dependency_info.pNext = nullptr;
				vk_dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
				vk_dependency_info.memoryBarrierCount = 0;
				vk_dependency_info.pMemoryBarriers = nullptr;
				vk_dependency_info.bufferMemoryBarrierCount = 0;
				vk_dependency_info.pBufferMemoryBarriers = nullptr;
				vk_dependency_info.imageMemoryBarrierCount = u32(vk_barriers.size());
				vk_dependency_info.pImageMemoryBarriers = vk_barriers.data();

				vkCmdPipelineBarrier2(command_buffer, &vk_dependency_info);
			}

			if (stop)
				break;

			order++;

		}
		if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

		return true;
	}

	bool RenderGraphHandler::FillCommandBufferPrimitive(VkCommandBuffer command_buffer, const FrameInfo& frame_info, const std::map<PipelineId, GraphicsPipeline>& pipelines, const Primitive& scene) const
	{
		return false;
	}

	void RenderGraphHandler::ProcessDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout, const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets, const std::map<DescriptorSetType, VkDescriptorSet>& holder_desc_sets) const
	{
		uint32_t sequence_begin = 0;
		std::vector<VkDescriptorSet> desc_sets_to_bind;

		for (auto&& [set_id, set_layout] : pipeline_desc_sets)
		{
			if (auto&& holder_set = holder_desc_sets.find(set_layout.GetType()); holder_set != holder_desc_sets.end())
			{
				if (desc_sets_to_bind.size() == 0)
				{
					sequence_begin = set_id;
				}

				desc_sets_to_bind.push_back(holder_set->second);
			}
			else
			{
				if (desc_sets_to_bind.size() != 0)
				{
					vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, sequence_begin, u32(desc_sets_to_bind.size()), desc_sets_to_bind.data(), 0, nullptr);
					desc_sets_to_bind.clear();
				}
			}
		}

		if (desc_sets_to_bind.size() != 0)
		{
			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, sequence_begin, u32(desc_sets_to_bind.size()), desc_sets_to_bind.data(), 0, nullptr);
			desc_sets_to_bind.clear();
		}
	}
}