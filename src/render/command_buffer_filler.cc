//#include "command_buffer_filler.h"
//
//#include <stl_util.h>
//
//render::CommandBufferFiller::CommandBufferFiller(const RenderSetup& render_setup, const FramebufferCollection& framebuffer_collection) : render_setup_(render_setup), framebuffer_collection_(framebuffer_collection)
//{
//}
//
//void render::CommandBufferFiller::Fill(VkCommandBuffer command_buffer, std::vector<RenderPassInfo> render_info, const Framebuffer& screen_buffer)
//{
//	VkCommandBufferBeginInfo begin_info{};
//	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Optional
//	begin_info.pInheritanceInfo = nullptr; // Optional
//
//	if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
//		throw std::runtime_error("failed to begin recording command buffer!");
//	}
//
//	for (auto&& render_pass_info : render_info)
//	{
//		const Framebuffer& framebuffer = render_pass_info.framebuffer_id == FramebufferId::kScreen ? screen_buffer : framebuffer_collection_.GetFramebuffer(render_pass_info.framebuffer_id);
//
//		VkRenderPassBeginInfo render_pass_begin_info{};
//		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//		render_pass_begin_info.renderPass = render_setup_.GetRenderPass(render_pass_info.render_pass_id).GetHandle();
//		render_pass_begin_info.framebuffer = framebuffer.GetHandle();
//
//		render_pass_begin_info.renderArea.offset = { 0, 0 };
//		render_pass_begin_info.renderArea.extent = framebuffer.GetExtent();
//
//		std::vector<VkClearValue> clear_value(framebuffer.GetClearValues().size());
//
//		for (int i = 0; i < framebuffer.GetClearValues().size(); i++)
//		{
//			clear_value[i].color.float32[0] = framebuffer.GetClearValues()[i].color[0];
//			clear_value[i].color.float32[1] = framebuffer.GetClearValues()[i].color[1];
//			clear_value[i].color.float32[2] = framebuffer.GetClearValues()[i].color[2];
//			clear_value[i].color.float32[3] = framebuffer.GetClearValues()[i].color[3];
//
//			clear_value[i].depthStencil.depth= framebuffer.GetClearValues()[i].depth;
//			clear_value[i].depthStencil.stencil = framebuffer.GetClearValues()[i].stencil;
//		}
//
//		render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_value.size());
//		render_pass_begin_info.pClearValues = clear_value.data();
//
//		vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
//
//		int set_ind = 0;
//
//
//
//		for (auto&& pipeline_info: render_pass_info.pipelines)
//		{
//			auto&& pipeline = render_setup_.GetPipeline(pipeline_info.id);
//			auto&& pipeline_layout = pipeline.GetLayout();
//
//			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetHandle());
//
//			const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets = pipeline.GetDescriptorSets();
//
//			ProcessDescriptorSets(command_buffer, pipeline_layout, pipeline_desc_sets, pipeline_info.scene.GetDescriptorSets());
//
//			for (auto&& child : pipeline_info.scene.GetChildren())
//			{
//				if (child.GetPrimitives().begin()->type != pipeline_info.primitive_type)
//					continue;
//
//				ProcessDescriptorSets(command_buffer, pipeline_layout, pipeline_desc_sets, child.GetDescriptorSets());
//
//				std::vector<VkBuffer> vert_bufs;
//				std::vector<VkDeviceSize> offsets;
//
//				stl_util::size<uint32_t>(offsets);
//
//				for (auto&& buf : child.GetPrimitives().begin()->vertex_buffers)
//				{
//					if (buf.buffer)
//					{
//						vert_bufs.push_back(buf.buffer->GetHandle());
//						offsets.push_back(buf.offset);
//					}
//				}
//
//
//				vkCmdBindVertexBuffers(command_buffer, 0, u32(vert_bufs.size()), vert_bufs.data(), offsets.data());
//				vkCmdBindIndexBuffer(command_buffer, child.GetPrimitives().begin()->indices.buffer->GetHandle(), child.GetPrimitives().begin()->indices.offset, VK_INDEX_TYPE_UINT16);
//
//				if (pipeline_info.id == PipelineId::kCollectGBuffers)
//				{
//					vkCmdDraw(command_buffer, 6, 1, 0, 0);
//				}
//				else
//				{
//					vkCmdDrawIndexed(command_buffer, u32(child.GetPrimitives().begin()->indices.count), 1, 0, 0, 0);
//				}
//			}
//		}
//
//		vkCmdEndRenderPass(command_buffer);
//	}
//
//	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
//		throw std::runtime_error("failed to record command buffer!");
//	}
//}
//
//void render::CommandBufferFiller::ProcessDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout, const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets, const std::map<DescriptorSetType, VkDescriptorSet>& holder_desc_sets) const
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
