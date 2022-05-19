#include "render_pass.h"

#include <array>
#include <map>

#include "common.h"

render::RenderPass::RenderPass(const DeviceConfiguration& device_cfg, RenderPassDesc render_pass_desc): RenderObjBase(device_cfg)
{
	std::vector<VkAttachmentDescription> attachments;
	std::map<std::string, uint32_t> attachment_name_to_index;

	for (uint32_t i = 0; i < render_pass_desc.attachments.size(); i++)
	{
		attachments.push_back(render_pass_desc.attachments[i].desc);
		attachment_name_to_index[render_pass_desc.attachments[i].name] = i;
	}


	std::vector<VkSubpassDescription> subpasses;
	std::map<std::string, uint32_t> subpass_name_to_index;

	std::vector<std::vector<VkAttachmentReference>> subpasses_color_refs;
	std::vector<VkAttachmentReference> subpasses_depth_refs;


	subpasses.resize(render_pass_desc.subpasses.size());

	subpasses_color_refs.resize(render_pass_desc.subpasses.size());
	subpasses_depth_refs.resize(render_pass_desc.subpasses.size());

	for (uint32_t subpass_ind = 0; subpass_ind < render_pass_desc.subpasses.size(); subpass_ind++)
	{
		subpass_name_to_index[render_pass_desc.subpasses[subpass_ind].name] = subpass_ind;

		for (uint32_t att_ref_ind = 0; att_ref_ind < render_pass_desc.subpasses[subpass_ind].attachment_refs.size(); att_ref_ind++)
		{
			uint32_t attachment_index = attachment_name_to_index[render_pass_desc.subpasses[subpass_ind].attachment_refs[att_ref_ind].name];

			if (render_pass_desc.attachments[attachment_index].is_depth_attachment)
			{
				subpasses_depth_refs[subpass_ind].attachment = attachment_index;
				subpasses_depth_refs[subpass_ind].layout = render_pass_desc.subpasses[subpass_ind].attachment_refs[att_ref_ind].layout;
				subpasses[subpass_ind].pDepthStencilAttachment = &subpasses_depth_refs[subpass_ind];
			}
			else
			{
				subpasses_color_refs[subpass_ind].push_back({});
				subpasses_color_refs[subpass_ind].back().attachment = attachment_index;
				subpasses_color_refs[subpass_ind].back().layout = render_pass_desc.subpasses[subpass_ind].attachment_refs[att_ref_ind].layout;
			}
		}

		subpasses[subpass_ind].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[subpass_ind].colorAttachmentCount = subpasses_color_refs[subpass_ind].size();
		subpasses[subpass_ind].pColorAttachments = subpasses_color_refs[subpass_ind].data();
	}

	std::vector<VkSubpassDependency> dependencies;

	for (auto&& dependency : render_pass_desc.dependencies)
	{
		VkSubpassDependency vk_dependency = dependency.dependency;
		
		vk_dependency.srcSubpass = dependency.from_name.empty() ? VK_SUBPASS_EXTERNAL : subpass_name_to_index[dependency.from_name];
		vk_dependency.dstSubpass = dependency.to_name.empty() ? VK_SUBPASS_EXTERNAL : subpass_name_to_index[dependency.to_name];
		dependencies.push_back(vk_dependency);
	}

	
	VkRenderPassCreateInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = attachments.size();
	render_pass_info.pAttachments = attachments.data();
	render_pass_info.subpassCount = subpasses.size();
	render_pass_info.pSubpasses = subpasses.data();
	render_pass_info.dependencyCount = dependencies.size();
	render_pass_info.pDependencies = dependencies.data();



	if (vkCreateRenderPass(device_cfg_.logical_device, &render_pass_info, nullptr, &handle_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

}

render::RenderPass::RenderPassDesc render::RenderPass::BuildRenderPassDesc(RenderPassType type, VkFormat color_format, VkFormat depth_format)
{

	RenderPassDesc result;

	switch (type)
	{
	case render::RenderPass::RenderPassType::kDraw:
	{
		{
			RenderPassDesc::Attachment attachment{};
			attachment.name = "color";
			attachment.is_depth_attachment = false;

			attachment.desc.format = color_format;
			attachment.desc.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment.desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment.desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment.desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			result.attachments.push_back(attachment);
		}

		{
			RenderPassDesc::Attachment attachment{};
			attachment.name = "depth";
			attachment.is_depth_attachment = true;

			attachment.desc.format = depth_format; //VK_FORMAT_D32_SFLOAT
			attachment.desc.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment.desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment.desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment.desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			result.attachments.push_back(attachment);
		}


		RenderPassDesc::Subpass subpass{};

		subpass.name = "render";
		subpass.attachment_refs.push_back({ "color", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		subpass.attachment_refs.push_back({ "depth", VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });

		result.subpasses.push_back(subpass);

		RenderPassDesc::Dependency dependency{};

		dependency.from_name = "";
		dependency.to_name = "render";

		dependency.dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT/* | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT*/;
		dependency.dependency.srcAccessMask = 0;

		dependency.dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		result.dependencies.push_back(dependency);
	}
	break;
	case render::RenderPass::RenderPassType::kDepth:
	{
		RenderPassDesc::Attachment attachment{};
		attachment.name = "depth";
		attachment.is_depth_attachment = true;

		attachment.desc.format = depth_format; //VK_FORMAT_D32_SFLOAT
		attachment.desc.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		result.attachments.push_back(attachment);



		RenderPassDesc::Subpass subpass{};

		subpass.name = "render";
		subpass.attachment_refs.push_back({ "depth", VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });

		result.subpasses.push_back(subpass);

		{
			RenderPassDesc::Dependency dependency{};

			dependency.from_name = "";
			dependency.to_name = "render";

			dependency.dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependency.dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;

			dependency.dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			result.dependencies.push_back(dependency);
		}

		{
			RenderPassDesc::Dependency dependency{};

			dependency.from_name = "render";
			dependency.to_name = "";

			dependency.dependency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependency.dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			dependency.dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependency.dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			result.dependencies.push_back(dependency);
		}
	}
	break;
	default:
		break;
	}

	return result;

}

render::RenderPass::~RenderPass()
{
	if (handle_ != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(device_cfg_.logical_device, handle_, nullptr);
	}
}
