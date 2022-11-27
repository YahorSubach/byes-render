#include "render_pass.h"

#include <algorithm>
#include <array>
#include <map>

#include "common.h"

#include "render/render_graph.h"

render::RenderPass::RenderPass(const DeviceConfiguration& device_cfg, const RenderNode& render_node): RenderObjBase(device_cfg), contains_depth_attachment_(false)
{
	
	std::vector<VkAttachmentDescription> vk_attachments;
	std::map<std::string, uint32_t> attachment_name_to_index;

	int attachment_index = 0;
	for (auto&& node_attachment : render_node.GetAttachments())
	{
		VkAttachmentDescription attachment_description = {};

		attachment_description.format = node_attachment.format;
		attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment_description.loadOp = node_attachment.depends_on ? VK_ATTACHMENT_LOAD_OP_LOAD : node_attachment.format == device_cfg.depth_map_format ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;

		if(node_attachment.format == device_cfg.presentation_format || !node_attachment.to_dependencies.empty())
			attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		else attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		
		attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		if (!node_attachment.depends_on)
		{
			attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
		else
		{
			attachment_description.initialLayout = node_attachment.format == device_cfg.depth_map_format ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		if (node_attachment.is_swapchain_image && node_attachment.to_dependencies.empty())
		{
			attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}
		else
		{
			attachment_description.finalLayout = node_attachment.format == device_cfg.depth_map_format ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		vk_attachments.push_back(attachment_description);
		attachment_name_to_index[node_attachment.name] = attachment_index;
		attachment_index++;
	}

	std::vector<VkSubpassDescription> subpasses;
	std::map<std::string, uint32_t> subpass_name_to_index;

	std::vector<std::vector<VkAttachmentReference>> subpasses_color_refs;
	std::vector<VkAttachmentReference> subpasses_depth_refs;

	int subpass_cnt = 1;


	subpasses.resize(subpass_cnt);

	subpasses_color_refs.resize(subpass_cnt);
	subpasses_depth_refs.resize(subpass_cnt);

	for (uint32_t subpass_ind = 0; subpass_ind < subpass_cnt; subpass_ind++)
	{
		int attachment_index = 0;
		for (auto&& node_attachment : render_node.GetAttachments())
		{
			if (node_attachment.format == device_cfg.depth_map_format)
			{
				subpasses_depth_refs[subpass_ind].attachment = attachment_index;
				subpasses_depth_refs[subpass_ind].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				subpasses[subpass_ind].pDepthStencilAttachment = &subpasses_depth_refs[subpass_ind];
			}
			else
			{
				subpasses_color_refs[subpass_ind].push_back({});
				subpasses_color_refs[subpass_ind].back().attachment = attachment_index;
				subpasses_color_refs[subpass_ind].back().layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}

			attachment_index++;
		}

		subpasses[subpass_ind].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[subpass_ind].colorAttachmentCount = subpasses_color_refs[subpass_ind].size();
		subpasses[subpass_ind].pColorAttachments = subpasses_color_refs[subpass_ind].data();
	}

	std::vector<VkSubpassDependency> dependencies;

	bool acquire_depend = false;
	for (auto&& attachment : render_node.GetAttachments())
	{
		if (attachment.is_swapchain_image)
		{
			acquire_depend = true;
			break;
		}
	}

	if (acquire_depend)
	{
		//if (!it->second.depends_on)
		{
			VkSubpassDependency vk_dependency;

			vk_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			vk_dependency.dstSubpass = 0;

			vk_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT/* | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT*/;
			vk_dependency.srcAccessMask = 0;

			vk_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			vk_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			vk_dependency.dependencyFlags = 0;

			dependencies.push_back(vk_dependency);
		}
	}


	VkRenderPassCreateInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = vk_attachments.size();
	render_pass_info.pAttachments = vk_attachments.data();
	render_pass_info.subpassCount = subpasses.size();
	render_pass_info.pSubpasses = subpasses.data();
	render_pass_info.dependencyCount = dependencies.size();
	render_pass_info.pDependencies = dependencies.data();



	if (vkCreateRenderPass(device_cfg_.logical_device, &render_pass_info, nullptr, &handle_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}


//int render::RenderPass::GetAttachmentIndex(const std::string_view& name) const
//{
//	auto&& it = std::find_if(attachments_.begin(), attachments_.end(), [&name](const Attachment& a) {return a.name == name; });
//
//	if (it != attachments_.end())
//		return (it - attachments_.begin());
//
//	return -1;
//}
//
//int render::RenderPass::GetAttachmentsCnt() const
//{
//	return attachments_.size();
//}
//
//int render::RenderPass::GetColorAttachmentsCnt() const
//{
//	if(contains_depth_attachment_)
//		return attachments_.size() - 1;
//
//	return attachments_.size();
//}
//
//const render::RenderPass::Attachment& render::RenderPass::GetAttachmentByIndex(int index) const
//{
//	return attachments_[index];
//}

render::RenderPass::~RenderPass()
{
	if (handle_ != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(device_cfg_.logical_device, handle_, nullptr);
	}
}


//render::RenderPass2::RenderPass2(const DeviceConfiguration& device_cfg) : RenderObjBase(device_cfg)
//{
//}
