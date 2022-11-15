#include "render_graph.h"

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


render::RenderGraph::RenderGraph(const DeviceConfiguration& device_cfg, const RenderSetup& render_setup, const Image& presentation_image): RenderObjBase(device_cfg), presentation_image_view_(device_cfg, presentation_image)
{
	auto&& [g_albedo_image, g_albedo_image_view] = collection_.CreateImage(device_cfg_, device_cfg_.g_buffer_format, device_cfg_.presentation_extent);
	auto&& [g_position_image, g_position_image_view] = collection_.CreateImage(device_cfg_, device_cfg_.g_buffer_format, device_cfg_.presentation_extent);
	auto&& [g_normal_image, g_normal_image_view] = collection_.CreateImage(device_cfg_, device_cfg_.g_buffer_format, device_cfg_.presentation_extent);
	auto&& [g_metallic_roughness_image, g_metallic_roughness_image_view] = collection_.CreateImage(device_cfg_, device_cfg_.g_buffer_format, device_cfg_.presentation_extent);

	auto&& [g_depth_image, g_depth_image_view] = collection_.CreateImage(device_cfg_, device_cfg_.depth_map_format, device_cfg_.depth_map_extent);

	auto&& g_framebuffer = collection_.CreateFramebuffer(device_cfg_, device_cfg_.presentation_extent, render_setup.GetRenderPass(RenderPassId::kBuildGBuffers));
	auto&& presentation_framebuffer = collection_.CreateFramebuffer(device_cfg_, device_cfg_.presentation_extent, render_setup.GetRenderPass(RenderPassId::kSimpleRenderToScreen));

	g_framebuffer.AddAttachment("g_albedo", g_albedo_image_view);
	g_framebuffer.AddAttachment("g_position", g_position_image_view);
	g_framebuffer.AddAttachment("g_normal", g_normal_image_view);
	g_framebuffer.AddAttachment("g_metal_rough", g_metallic_roughness_image_view);
	g_framebuffer.AddAttachment("g_depth", g_depth_image_view);

	presentation_framebuffer.AddAttachment("swapchain_image", presentation_image_view_);

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

bool render::RenderGraph::FillCommandBuffer(VkCommandBuffer command_buffer) const
{
	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Optional
	begin_info.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	for (auto&& render_pass_info : render_info)
	{
		const Framebuffer& framebuffer = render_pass_info.framebuffer_id == FramebufferId::kScreen ? screen_buffer : framebuffer_collection_.GetFramebuffer(render_pass_info.framebuffer_id);

		VkRenderPassBeginInfo render_pass_begin_info{};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = render_setup_.GetRenderPass(render_pass_info.render_pass_id).GetHandle();
		render_pass_begin_info.framebuffer = framebuffer.GetHandle();

		render_pass_begin_info.renderArea.offset = { 0, 0 };
		render_pass_begin_info.renderArea.extent = framebuffer.GetExtent();

		std::vector<VkClearValue> clear_value(framebuffer.GetClearValues().size());

		for (int i = 0; i < framebuffer.GetClearValues().size(); i++)
		{
			clear_value[i].color.float32[0] = framebuffer.GetClearValues()[i].color[0];
			clear_value[i].color.float32[1] = framebuffer.GetClearValues()[i].color[1];
			clear_value[i].color.float32[2] = framebuffer.GetClearValues()[i].color[2];
			clear_value[i].color.float32[3] = framebuffer.GetClearValues()[i].color[3];

			clear_value[i].depthStencil.depth = framebuffer.GetClearValues()[i].depth;
			clear_value[i].depthStencil.stencil = framebuffer.GetClearValues()[i].stencil;
		}

		render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_value.size());
		render_pass_begin_info.pClearValues = clear_value.data();

		vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		int set_ind = 0;



		for (auto&& pipeline_info : render_pass_info.pipelines)
		{
			auto&& pipeline = render_setup_.GetPipeline(pipeline_info.id);
			auto&& pipeline_layout = pipeline.GetLayout();

			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetHandle());

			const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets = pipeline.GetDescriptorSets();

			ProcessDescriptorSets(command_buffer, pipeline_layout, pipeline_desc_sets, pipeline_info.scene.GetDescriptorSets());

			for (auto&& child : pipeline_info.scene.GetChildren())
			{
				if (child.GetPrimitives().begin()->type != pipeline_info.primitive_type)
					continue;

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

				if (pipeline_info.id == PipelineId::kCollectGBuffers)
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
	}

	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}
