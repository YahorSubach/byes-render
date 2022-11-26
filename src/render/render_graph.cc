#include "render_graph.h"

#include <stack>
#include <queue>
#include <set>
//
//
//render::RenderGraph::RenderGraph(const DeviceConfiguration& device_cfg, const RenderSetup& render_setup, ModelSceneDescSetHolder& scene) : RenderObjBase(device_cfg)
//{
//	//TODO fuck! You really have to change this return of push back logic
//	collection_.images.reserve(16);
//	collection_.image_views.reserve(16);
//	collection_.frambuffers.reserve(16);
//	collection_.render_batches.reserve(16);
//
//
//	auto&& [g_albedo_image, g_albedo_image_view] = collection_.CreateImage(device_cfg_, device_cfg_.high_range_color_format, device_cfg_.presentation_extent);
//	auto&& [g_position_image, g_position_image_view] = collection_.CreateImage(device_cfg_, device_cfg_.high_range_color_format, device_cfg_.presentation_extent);
//	auto&& [g_normal_image, g_normal_image_view] = collection_.CreateImage(device_cfg_, device_cfg_.high_range_color_format, device_cfg_.presentation_extent);
//	auto&& [g_metallic_roughness_image, g_metallic_roughness_image_view] = collection_.CreateImage(device_cfg_, device_cfg_.high_range_color_format, device_cfg_.presentation_extent);
//
//	auto&& [g_depth_image, g_depth_image_view] = collection_.CreateImage(device_cfg_, device_cfg_.depth_map_format, device_cfg_.presentation_extent);
//
//	auto&& [g_shadowmap_image, g_shadowmap_image_view] = collection_.CreateImage(device_cfg_, device_cfg_.depth_map_format, device_cfg_.shadowmap_extent);
//
//
//	auto&& shadowmap_framebuffer = collection_.CreateFramebuffer(device_cfg_, device_cfg_.shadowmap_extent, render_setup.GetRenderPass(RenderPassId::kBuildDepthmap));
//	shadowmap_framebuffer.AddAttachment("shadowmap", g_shadowmap_image_view);
//
//
//	auto&& g_framebuffer = collection_.CreateFramebuffer(device_cfg_, device_cfg_.presentation_extent, render_setup.GetRenderPass(RenderPassId::kBuildGBuffers));
//	//auto&& presentation_framebuffer = collection_.CreateFramebuffer(device_cfg_, device_cfg_.presentation_extent, render_setup.GetRenderPass(RenderPassId::kSimpleRenderToScreen));
//
//	g_framebuffer.AddAttachment("g_albedo", g_albedo_image_view);
//	g_framebuffer.AddAttachment("g_position", g_position_image_view);
//	g_framebuffer.AddAttachment("g_normal", g_normal_image_view);
//	g_framebuffer.AddAttachment("g_metal_rough", g_metallic_roughness_image_view);
//	g_framebuffer.AddAttachment("g_depth", g_depth_image_view);
//
//	//presentation_framebuffer.AddAttachment("swapchain_image", presentation_image_view_);
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
//render::RenderGraph::~RenderGraph()
//{
//}
//
//std::pair<render::Image&, render::ImageView&> render::RenderGraph::RenderCollection::CreateImage(const DeviceConfiguration& device_cfg, VkFormat format, Extent extent)
//{
//	images.push_back(Image(device_cfg, format, extent));
//	image_views.push_back(ImageView(device_cfg, images.back()));
//	return { images.back(), image_views.back() };
//}
//
//render::Framebuffer& render::RenderGraph::RenderCollection::CreateFramebuffer(const DeviceConfiguration& device_cfg, Extent extent, const RenderPass& render_pass)
//{
//	frambuffers.push_back(Framebuffer(device_cfg, extent, render_pass));
//	return frambuffers.back();
//}
//
//render::RenderGraph::RenderBatch& render::RenderGraph::RenderCollection::CreateBatch()
//{
//	render_batches.push_back({});
//	return render_batches.back();
//}
//
//bool render::RenderGraph::FillCommandBuffer(VkCommandBuffer command_buffer, const Framebuffer& swapchain_framebuffer, const std::map<DescriptorSetType, VkDescriptorSet>& scene_ds, const std::vector<RenderModel>& render_models) const
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
//			stl_util::NullableRef<const Image> barrier_image = dependency.image;
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
//			barrier.srcQueueFamilyIndex = device_cfg_.graphics_queue_index;
//			barrier.dstQueueFamilyIndex = device_cfg_.graphics_queue_index;
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
//void render::RenderGraph::ProcessDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout, const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets, const std::map<DescriptorSetType, VkDescriptorSet>& holder_desc_sets) const
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
//void render::RenderGraph::RenderBatch::AddDependencyAsSampled(const RenderBatch& batch, const Image& image)
//{
//	dependencies.push_back({ batch, image, true});
//}
//
//void render::RenderGraph::RenderBatch::AddDependencyAsAttachment(const RenderBatch& batch, const Image& image)
//{
//	dependencies.push_back({ batch, image, false });
//}
//
//void render::RenderGraph::RenderBatch::AddSwapchainDependencyAsSampled(const RenderBatch& batch)
//{
//	dependencies.push_back({ batch, {}, true });
//}
//
//void render::RenderGraph::RenderBatch::AddSwapchainDependencyAsAttachment(const RenderBatch& batch)
//{
//	dependencies.push_back({ batch, {}, false });
//}


render::RenderGraph2::RenderGraph2(const DeviceConfiguration device_cfg): RenderObjBase(device_cfg)
{
}

render::RenderGraph2::Node& render::RenderGraph2::AddNode(const std::string& name, const Extent& extent, RenderModelCategoryFlags category_flags)
{
	auto [it, success] = nodes_.insert({ name, Node{*this, name, extent, category_flags} });
	assert(success);
	return it->second;
}

void render::RenderGraph2::Build()
{
	for (auto&& [name, node] : nodes_)
	{
		node.Build();
	}
}

const std::map<std::string, render::RenderGraph2::Node>& render::RenderGraph2::GetNodes() const
{
	return nodes_;
}

render::RenderGraph2::Node::Node(const RenderGraph2& render_graph, const std::string& name, const Extent& extent, RenderModelCategoryFlags category_flags) : 
	name_(name), render_graph_(render_graph), extent_(extent), order(0), category_flags(category_flags)
{
}

render::RenderGraph2::Attachment& render::RenderGraph2::Node::AddAttachment(const std::string& name, Format format, Extent extent)
{
	auto [it, success] = attachments_.insert({ name, {name, format, extent, *this} });
	assert(success);
	return it->second;
}

render::RenderGraph2::Attachment& render::RenderGraph2::Node::GetAttachment(const std::string& name)
{
	return attachments_.at(name);
}


const std::map<std::string, render::RenderGraph2::Attachment>& render::RenderGraph2::Node::GetAttachments() const
{
	return attachments_;
}

const std::string& render::RenderGraph2::Node::GetName() const
{
	return name_;
}

void render::RenderGraph2::Node::Build()
{
	render_pass_ = RenderPass(render_graph_.device_cfg_, *this);
}
const render::RenderPass& render::RenderGraph2::Node::GetRenderPass() const
{
	assert(render_pass_);
	return render_pass_.value();
}

render::Extent render::RenderGraph2::Node::GetExtent() const
{
	return extent_;
}
//void render::RenderGraph2::Node::AddDependency(Dependency dependency)
//{
//	to_dependencies_.push_back(dependency);
//}

render::RenderGraph2::Attachment::DescriptorSetForwarder render::RenderGraph2::Attachment::operator>>(DescriptorSetType desc_type)
{
	return {*this, desc_type};
}

render::RenderGraph2::Attachment& render::RenderGraph2::Attachment::operator>>(Node& node_to_forward)
{
	return ForwardAsAttachment(node_to_forward);
}

render::RenderGraph2::Attachment& render::RenderGraph2::Attachment::ForwardAsAttachment(Node& to_node)
{
	//node.AddDependency({ *this, to_node, true });
	to_dependencies.push_back({ *this, to_node, DescriptorSetType::None });

	auto&& new_attachment = to_node.AddAttachment(name, format, extent);
	new_attachment.depends_on = node;
	to_node.order = std::max(to_node.order, node.order + 1);
	//to_node.depends = true;
}

render::RenderGraph2::Attachment& render::RenderGraph2::Attachment::ForwardAsSampled(Node& to_node, DescriptorSetType set_type, int binding_index)
{
	//node.AddDependency({ *this, to_node, false });
	to_dependencies.push_back({ *this, to_node, set_type, binding_index });
	to_node.order = std::max(to_node.order, node.order + 1);
	//to_node.depends = true;
	return *this;
}

render::RenderGraph2::Attachment::DescriptorSetForwarder& render::RenderGraph2::Attachment::DescriptorSetForwarder::operator>>(int binding_index)
{
	descriptor_set_binding_index = binding_index;
}

render::RenderGraph2::Attachment& render::RenderGraph2::Attachment::DescriptorSetForwarder::operator>>(Node& node_to_forward)
{
	return attachment.ForwardAsSampled(node_to_forward, type, descriptor_set_binding_index);
}

render::RenderGraphHandler::RenderGraphHandler(const DeviceConfiguration& device_cfg, const RenderGraph2& render_graph, DescriptorSetsManager& desc_set_manager): 
	RenderObjBase(device_cfg), render_graph_(render_graph)
{
	std::map<std::string, std::map<DescriptorSetType, std::map<int, const AttachmentImage&>>> desc_set_images;

	for (auto&& [node_name, node] : render_graph.GetNodes())
	{
		Framebuffer::ConstructParams framebuffer_params{node.GetRenderPass(), node.GetExtent()};
		bool has_dependency = false;
		for (auto&& [attachment_name, attachment] : node.GetAttachments())
		{
			auto&& it = attachment_images_.find(attachment_name);

			if (it != attachment_images_.end())
			{
				framebuffer_params.attachments.push_back(it->second.image_view);
				continue;
			}
			
			Image image(device_cfg, attachment.format, attachment.extent);
			
			if (attachment.format == device_cfg.depth_map_format)
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

			ImageView image_view(device_cfg, image);

			auto res = attachment_images_.insert({ attachment_name, AttachmentImage{attachment.format, std::move(image), std::move(image_view)}});
			framebuffer_params.attachments.push_back(res.first->second.image_view);

			for (auto&& dependency : attachment.to_dependencies)
			{
				has_dependency = true;
				if (dependency.descriptor_set_type != DescriptorSetType::None)
				{
					desc_set_images[dependency.to_node.GetName()][dependency.descriptor_set_type].emplace(dependency.descriptor_set_binding_index, res.first->second);
				}
			}
		}

		if (has_dependency)
		{
			Framebuffer framebuffer(device_cfg, framebuffer_params);
			node_data_.emplace(node_name, RenderNodeData{ std::move(framebuffer) });
		}
	}

	for (auto&& [node_name, descriptors] : desc_set_images)
	{
		auto&& node_data = node_data_[node_name];

		for (auto&& [desc_type, desc_images] : descriptors)
		{
			VkDescriptorSet vk_descriptor_set = desc_set_manager.GetFreeDescriptor(desc_type);

			std::vector<VkWriteDescriptorSet> writes(desc_images.size());

			for (auto&& [binding_index, binding_att_image] : desc_images)
			{
				VkDescriptorImageInfo image_info;
				image_info.sampler = device_cfg.texture_sampler->GetHandle();
				image_info.imageLayout = binding_att_image.format == device_cfg.depth_map_format ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				image_info.imageView = binding_att_image.image_view.GetHandle();

				writes[binding_index].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writes[binding_index].pNext = nullptr;
				writes[binding_index].dstSet = vk_descriptor_set;
				writes[binding_index].dstBinding = binding_index;
				writes[binding_index].dstArrayElement = 0;
				writes[binding_index].descriptorCount = 1;
				writes[binding_index].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				writes[binding_index].pImageInfo = &image_info;
				writes[binding_index].pBufferInfo = nullptr;
				writes[binding_index].pTexelBufferView = nullptr;
			}

			vkUpdateDescriptorSets(device_cfg.logical_device, writes.size(), writes.data(), 0, nullptr);
			node_data.descriptor_sets.emplace(desc_type, vk_descriptor_set);
		}
	}
}

bool render::RenderGraphHandler::FillCommandBuffer(VkCommandBuffer command_buffer, const Framebuffer& swapchain_framebuffer, const std::map<DescriptorSetType, VkDescriptorSet>& scene_ds, const std::vector<RenderModel>& render_models) const
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
		std::vector<std::reference_wrapper<const RenderGraph2::Dependency>> dependencies;
		for (auto&& [node_name, node] : render_graph_.GetNodes())
		{
			if (node.order != order)
				continue;

			stop = false;

			stl_util::NullableRef<const Framebuffer> framebuffer = swapchain_framebuffer;
			std::map<DescriptorSetType, VkDescriptorSet> node_desc_set;
			if (auto&& it = node_data_.find(node_name); it != node_data_.end())
			{
				framebuffer = it->second.frambuffer;
				node_desc_set = it->second.descriptor_sets;
			}

			VkRenderPassBeginInfo render_pass_begin_info{};
			render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_begin_info.renderPass = node.GetRenderPass().GetHandle();
			render_pass_begin_info.framebuffer = framebuffer->GetHandle();

			render_pass_begin_info.renderArea.offset = { 0, 0 };
			render_pass_begin_info.renderArea.extent = framebuffer->GetExtent();

			for (auto&& attachment : node.GetAttachments())
			{
				for (auto&& dependency : attachment.second.to_dependencies)
				{
					dependencies.push_back(dependency);
				}
			}

			std::vector<VkClearValue> clear_values(framebuffer->GetFormats().size());
			for (int att_ind = 0; att_ind < framebuffer->GetFormats().size(); att_ind++)
			{
				if (framebuffer->GetFormats()[att_ind] == device_cfg_.depth_map_format)
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
			for (auto&& render_model : render_models)
			{
				if (current_pipeline != &render_model.pipeline)
				{
					current_pipeline = &render_model.pipeline;

					vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_model.pipeline.GetHandle());
					const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets = render_model.pipeline.GetDescriptorSets();

					pipeline_layout = render_model.pipeline.GetLayout();

					ProcessDescriptorSets(command_buffer, pipeline_layout, pipeline_desc_sets, node_desc_set);
					ProcessDescriptorSets(command_buffer, pipeline_layout, pipeline_desc_sets, scene_ds);
				}

				if (/*true || */node.category_flags.Check(render_model.category)) {
					const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets = render_model.pipeline.GetDescriptorSets();

					ProcessDescriptorSets(command_buffer, pipeline_layout, pipeline_desc_sets, render_model.descriptor_sets);

					assert(render_model.vertex_buffers.size() == render_model.vertex_buffers_offsets.size());

					vkCmdBindVertexBuffers(command_buffer, 0, u32(render_model.vertex_buffers.size()), render_model.vertex_buffers.data(), render_model.vertex_buffers_offsets.data());

					if (render_model.index_buffer_and_offset.has_value())
					{
						vkCmdBindIndexBuffer(command_buffer, render_model.index_buffer_and_offset->first, render_model.index_buffer_and_offset->second, VK_INDEX_TYPE_UINT16);
						vkCmdDrawIndexed(command_buffer, render_model.vertex_count, 1, 0, 0, 0);
					}
					else
					{
						vkCmdDraw(command_buffer, render_model.vertex_count, 1, 0, 0);
					}
				}
			}



			//for (auto&& pipeline : render_pass_node.pipelines)
			//{
			//	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get().GetHandle());

			//	const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets = pipeline.get().GetDescriptorSets();

			//	auto&& pipeline_layout = pipeline.get().GetLayout();

			//	ProcessDescriptorSets(command_buffer, pipeline_layout, pipeline_desc_sets, scene_ds);

			//	for (auto&& render_model : render_models)
			//	{
			//		
			//	}
			//}

			vkCmdEndRenderPass(command_buffer);
		}

		std::vector<VkImageMemoryBarrier2> vk_barriers;
		for (auto&& dependency : dependencies)
		{

			stl_util::NullableRef<const Image> barrier_image = attachment_images_.at(dependency.get().from_attachment.name);

			if (!barrier_image)
			{
				barrier_image = swapchain_framebuffer.GetAttachment("swapchain_image").GetImage();
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
			vk_dependency_info.imageMemoryBarrierCount = vk_barriers.size();
			vk_dependency_info.pImageMemoryBarriers = vk_barriers.data();

			vkCmdPipelineBarrier2(command_buffer, &vk_dependency_info);
		}
	}
	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void render::RenderGraphHandler::ProcessDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout, const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets, const std::map<DescriptorSetType, VkDescriptorSet>& holder_desc_sets) const
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
