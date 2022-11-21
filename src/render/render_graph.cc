#include "render_graph.h"

#include <stack>
#include <queue>

//enum class FramebufferId
//{
//	kGBuffers,
//	kDepth,
//
//	kCount,
//
//	kScreen
//};
//
//enum class AttachmentId
//{
//	kGAlbedo,
//	kGPosition,
//	kGNormal,
//	kGMetallicRoughness,
//	kGDepth,
//
//	kDepthMap,
//
//	kCount
//};
//
//images_{
//	Image(device_cfg_, device_cfg_.g_buffer_format, device_cfg_.presentation_extent, {ImageProperty::kColorAttachment, ImageProperty::kShaderInput, ImageProperty::kMipMap}),
//	Image(device_cfg_, device_cfg_.g_buffer_format, device_cfg_.presentation_extent, {ImageProperty::kColorAttachment, ImageProperty::kShaderInput}),
//	Image(device_cfg_, device_cfg_.g_buffer_format, device_cfg_.presentation_extent, {ImageProperty::kColorAttachment, ImageProperty::kShaderInput}),
//	Image(device_cfg_, device_cfg_.g_buffer_format, device_cfg_.presentation_extent, {ImageProperty::kColorAttachment, ImageProperty::kShaderInput}),
//	Image(device_cfg_, device_cfg_.depth_map_format, device_cfg_.presentation_extent, {ImageProperty::kDepthAttachment}),
//
//	Image(device_cfg_, device_cfg_.depth_map_format, device_cfg_.depth_map_extent, {ImageProperty::kDepthAttachment, ImageProperty::kShaderInput}),
//},
//
//image_views_{
//	ImageView(device_cfg_, images_[static_cast<int>(AttachmentId::kGAlbedo)]),
//	ImageView(device_cfg_, images_[static_cast<int>(AttachmentId::kGPosition)]),
//	ImageView(device_cfg_, images_[static_cast<int>(AttachmentId::kGNormal)]),
//	ImageView(device_cfg_, images_[static_cast<int>(AttachmentId::kGMetallicRoughness)]),
//
//	ImageView(device_cfg_, images_[static_cast<int>(AttachmentId::kGDepth)]),
//	ImageView(device_cfg_, images_[static_cast<int>(AttachmentId::kDepthMap)]),
//},


render::RenderGraph::RenderGraph(const DeviceConfiguration& device_cfg, const RenderSetup& render_setup, ModelSceneDescSetHolder& scene) : RenderObjBase(device_cfg)
{
	//TODO fuck! You really have to change this return of push back logic
	collection_.images.reserve(16);
	collection_.image_views.reserve(16);
	collection_.frambuffers.reserve(16);
	collection_.render_batches.reserve(16);


	auto&& [g_albedo_image, g_albedo_image_view] = collection_.CreateImage(device_cfg_, device_cfg_.high_range_color_format, device_cfg_.presentation_extent);
	auto&& [g_position_image, g_position_image_view] = collection_.CreateImage(device_cfg_, device_cfg_.high_range_color_format, device_cfg_.presentation_extent);
	auto&& [g_normal_image, g_normal_image_view] = collection_.CreateImage(device_cfg_, device_cfg_.high_range_color_format, device_cfg_.presentation_extent);
	auto&& [g_metallic_roughness_image, g_metallic_roughness_image_view] = collection_.CreateImage(device_cfg_, device_cfg_.high_range_color_format, device_cfg_.presentation_extent);

	auto&& [g_depth_image, g_depth_image_view] = collection_.CreateImage(device_cfg_, device_cfg_.depth_map_format, device_cfg_.presentation_extent);

	auto&& [g_shadowmap_image, g_shadowmap_image_view] = collection_.CreateImage(device_cfg_, device_cfg_.depth_map_format, device_cfg_.shadowmap_extent);


	auto&& shadowmap_framebuffer = collection_.CreateFramebuffer(device_cfg_, device_cfg_.shadowmap_extent, render_setup.GetRenderPass(RenderPassId::kBuildDepthmap));
	shadowmap_framebuffer.AddAttachment("shadowmap", g_shadowmap_image_view);


	auto&& g_framebuffer = collection_.CreateFramebuffer(device_cfg_, device_cfg_.presentation_extent, render_setup.GetRenderPass(RenderPassId::kBuildGBuffers));
	//auto&& presentation_framebuffer = collection_.CreateFramebuffer(device_cfg_, device_cfg_.presentation_extent, render_setup.GetRenderPass(RenderPassId::kSimpleRenderToScreen));

	g_framebuffer.AddAttachment("g_albedo", g_albedo_image_view);
	g_framebuffer.AddAttachment("g_position", g_position_image_view);
	g_framebuffer.AddAttachment("g_normal", g_normal_image_view);
	g_framebuffer.AddAttachment("g_metal_rough", g_metallic_roughness_image_view);
	g_framebuffer.AddAttachment("g_depth", g_depth_image_view);

	//presentation_framebuffer.AddAttachment("swapchain_image", presentation_image_view_);


	auto&& shadowmap_batch = collection_.CreateBatch();
	auto&& g_fill_batch = collection_.CreateBatch();
	auto&& g_collect_batch = collection_.CreateBatch();

	shadowmap_batch.render_passes.push_back(RenderPassNode{ render_setup.GetRenderPass(RenderPassId::kBuildDepthmap), shadowmap_framebuffer, {render_setup.GetPipeline(PipelineId::kDepth)} });
	g_fill_batch.render_passes.push_back(RenderPassNode{ render_setup.GetRenderPass(RenderPassId::kBuildGBuffers), g_framebuffer, {render_setup.GetPipeline(PipelineId::kBuildGBuffers)} });
	g_collect_batch.render_passes.push_back(RenderPassNode{ render_setup.GetRenderPass(RenderPassId::kCollectGBuffers), {}, {render_setup.GetPipeline(PipelineId::kCollectGBuffers)} });

	shadowmap_batch.dependencies.push_back({ g_fill_batch, g_shadowmap_image});

	g_fill_batch.dependencies.push_back({ g_collect_batch, g_albedo_image });
	g_fill_batch.dependencies.push_back({ g_collect_batch, g_position_image });
	g_fill_batch.dependencies.push_back({ g_collect_batch, g_normal_image });
	g_fill_batch.dependencies.push_back({ g_collect_batch, g_metallic_roughness_image });
	g_fill_batch.dependencies.push_back({ g_collect_batch, g_depth_image });

	scene.g_albedo_image = g_albedo_image;
	scene.g_position_image = g_position_image;
	scene.g_normal_image = g_normal_image;
	scene.g_metal_rough_image = g_metallic_roughness_image;

	scene.shadowmap_image = g_shadowmap_image;
}

render::RenderGraph::~RenderGraph()
{
}

std::pair<render::Image&, render::ImageView&> render::RenderGraph::RenderCollection::CreateImage(const DeviceConfiguration& device_cfg, VkFormat format, Extent extent)
{
	images.push_back(Image(device_cfg, format, extent));
	image_views.push_back(ImageView(device_cfg, images.back()));
	return { images.back(), image_views.back() };
}

render::Framebuffer& render::RenderGraph::RenderCollection::CreateFramebuffer(const DeviceConfiguration& device_cfg, Extent extent, const RenderPass& render_pass)
{
	frambuffers.push_back(Framebuffer(device_cfg, extent, render_pass));
	return frambuffers.back();
}

render::RenderGraph::RenderBatch& render::RenderGraph::RenderCollection::CreateBatch()
{
	render_batches.push_back({});
	return render_batches.back();
}

bool render::RenderGraph::FillCommandBuffer(VkCommandBuffer command_buffer, const Framebuffer& swapchain_framebuffer, const SceneRenderNode& scene) const
{
	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Optional
	begin_info.pInheritanceInfo = nullptr; // Optional

	VkResult result;

	if (result = vkBeginCommandBuffer(command_buffer, &begin_info); result != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}


	for (auto&& batch : collection_.render_batches)
	{
		batch.processed = false;
	}

	std::queue<std::reference_wrapper<const RenderBatch>> batches_to_process;
	batches_to_process.push(collection_.render_batches.front());

	while (!batches_to_process.empty())
	{
		auto&& current_batch = batches_to_process.front().get();
		batches_to_process.pop();

		bool final_pass = false;
		for (auto&& render_pass_node : current_batch.render_passes)
		{
			auto frambuffer = render_pass_node.framebuffer;
			if (!frambuffer)
			{
				frambuffer = swapchain_framebuffer;
			}

			VkRenderPassBeginInfo render_pass_begin_info{};
			render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_begin_info.renderPass = render_pass_node.render_pass.GetHandle();
			render_pass_begin_info.framebuffer = frambuffer->GetHandle();

			render_pass_begin_info.renderArea.offset = { 0, 0 };
			render_pass_begin_info.renderArea.extent = frambuffer->GetExtent();

			std::vector<VkClearValue> clear_values(frambuffer->GetAttachments().size());
			int index = 0;
			for (auto&& attachment : frambuffer->GetAttachments())
			{
				if (attachment.get().CheckUsageFlag(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
				{
					clear_values[index].color = VkClearColorValue{ {0.0f, 0.0f, 0.0f, 1.0f} };
				}
				else
				{
					clear_values[index].depthStencil = { 1.0f, 0 };
				}
				index++;
			}

			render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
			render_pass_begin_info.pClearValues = clear_values.data();

			vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

			for (auto&& pipeline : render_pass_node.pipelines)
			{
				vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get().GetHandle());

				const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets = pipeline.get().GetDescriptorSets();

				auto&& pipeline_layout = pipeline.get().GetLayout();

				ProcessDescriptorSets(command_buffer, pipeline_layout, pipeline_desc_sets, scene.GetDescriptorSets());

				for (auto&& child : scene.GetChildren())
				{
					//if (child.GetPrimitives().begin()->type != pipeline_info.primitive_type)
					//	continue;

					ProcessDescriptorSets(command_buffer, pipeline_layout, pipeline_desc_sets, child.GetDescriptorSets());

					std::vector<VkBuffer> vert_bufs;
					std::vector<VkDeviceSize> offsetes;

					stl_util::size<uint32_t>(offsetes);

					for (auto&& buf : child.GetPrimitives().begin()->vertex_buffers)
					{
						if (buf.buffer)
						{
							vert_bufs.push_back(buf.buffer->GetHandle());
							offsetes.push_back(buf.offset);
						}
					}


					vkCmdBindVertexBuffers(command_buffer, 0, u32(vert_bufs.size()), vert_bufs.data(), offsetes.data());
					vkCmdBindIndexBuffer(command_buffer, child.GetPrimitives().begin()->indices.buffer->GetHandle(), child.GetPrimitives().begin()->indices.offset, VK_INDEX_TYPE_UINT16);

					if (final_pass)
					{
						vkCmdDraw(command_buffer, 6, 1, 0, 0);
					}
					else
					{
						vkCmdDrawIndexed(command_buffer, u32(child.GetPrimitives().begin()->indices.count), 1, 0, 0, 0);
					}
				}
			}

			vkCmdEndRenderPass(command_buffer);

			final_pass = true;
		}

		std::vector<VkImageMemoryBarrier2> vk_barriers;
		for (auto&& dependency : current_batch.dependencies)
		{
			if (!dependency.first.processed)
			{
				dependency.first.processed = true;
				batches_to_process.push(dependency.first);
			}

			VkStructureType            sType;
			const void* pNext;
			VkPipelineStageFlags2      srcStageMask;
			VkAccessFlags2             srcAccessMask;
			VkPipelineStageFlags2      dstStageMask;
			VkAccessFlags2             dstAccessMask;
			VkImageLayout              oldLayout;
			VkImageLayout              newLayout;
			uint32_t                   srcQueueFamilyIndex;
			uint32_t                   dstQueueFamilyIndex;
			VkImage                    image;
			VkImageSubresourceRange    subresourceRange;

			VkImageMemoryBarrier2 barrier;
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			barrier.pNext = nullptr;
			barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
			barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
			barrier.oldLayout = dependency.second.CheckUsageFlag(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			barrier.newLayout = dependency.second.CheckUsageFlag(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			barrier.srcQueueFamilyIndex = device_cfg_.graphics_queue_index;
			barrier.dstQueueFamilyIndex = device_cfg_.graphics_queue_index;
			barrier.image = dependency.second.GetHandle();
			barrier.subresourceRange.aspectMask = dependency.second.CheckUsageFlag(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
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







//for (auto&& render_pass_info : render_info)
//{
//	const Framebuffer& framebuffer = render_pass_info.framebuffer_id == FramebufferId::kScreen ? screen_buffer : framebuffer_collection_.GetFramebuffer(render_pass_info.framebuffer_id);

//	

//	vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

//	int set_ind = 0;



//	for (auto&& pipeline_info : render_pass_info.pipelines)
//	{
//		auto&& pipeline = render_setup_.GetPipeline(pipeline_info.id);
//		auto&& pipeline_layout = pipeline.GetLayout();

//		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetHandle());

//		const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets = pipeline.GetDescriptorSets();

//		ProcessDescriptorSets(command_buffer, pipeline_layout, pipeline_desc_sets, pipeline_info.scene.GetDescriptorSets());

//		for (auto&& child : pipeline_info.scene.GetChildren())
//		{
//			if (child.GetPrimitives().begin()->type != pipeline_info.primitive_type)
//				continue;

//			ProcessDescriptorSets(command_buffer, pipeline_layout, pipeline_desc_sets, child.GetDescriptorSets());

//			std::vector<VkBuffer> vert_bufs;
//			std::vector<VkDeviceSize> offsetes;

//			stl_util::size<uint32_t>(offsetes);

//			for (auto&& buf : child.GetPrimitives().begin()->vertex_buffers)
//			{
//				if (buf.buffer)
//				{
//					vert_bufs.push_back(buf.buffer->GetHandle());
//					offsetes.push_back(buf.offset);
//				}
//			}


//			vkCmdBindVertexBuffers(command_buffer, 0, u32(vert_bufs.size()), vert_bufs.data(), offsetes.data());
//			vkCmdBindIndexBuffer(command_buffer, child.GetPrimitives().begin()->indices.buffer->GetHandle(), child.GetPrimitives().begin()->indices.offset, VK_INDEX_TYPE_UINT16);

//			if (pipeline_info.id == PipelineId::kCollectGBuffers)
//			{
//				vkCmdDraw(command_buffer, 6, 1, 0, 0);
//			}
//			else
//			{
//				vkCmdDrawIndexed(command_buffer, u32(child.GetPrimitives().begin()->indices.count), 1, 0, 0, 0);
//			}
//		}
//	}

//	vkCmdEndRenderPass(command_buffer);
//}




void render::RenderGraph::ProcessDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout, const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets, const std::map<DescriptorSetType, VkDescriptorSet>& holder_desc_sets) const
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