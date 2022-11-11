#include "framebuffer.h"

#include <array>

#include "common.h"

render::Framebuffer::Framebuffer(const DeviceConfiguration& device_cfg, const Extent& extent, const std::vector<std::reference_wrapper<const ImageView>>& attachments):
	RenderObjBase(device_cfg), extent_(extent)
{
	for (auto&& attachment : attachments)
	{
		if (attachment.get().GetImageProperties().Check(ImageProperty::kDepthAttachment))
		{
			DepthStencilClearValues clear_values;
			clear_values.depth = 1.0f;
			clear_values.stencil = 0;
			attachments_descs_.push_back({ attachment, clear_values });
		}
		else
		{
			ColorClearValues clear_values;
			clear_values.r = 0.0f;
			clear_values.g = 0.0f;
			clear_values.b = 0.0f;
			clear_values.a = 0.0f;
			attachments_descs_.push_back({ attachment, clear_values });
		}
	}

	VkFramebufferCreateInfo framebuffer_info{};
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.renderPass = render_pass.GetHandle();
	framebuffer_info.attachmentCount = u32(vk_attachments.size());
	framebuffer_info.pAttachments = vk_attachments.data();
	framebuffer_info.width = extent.width;
	framebuffer_info.height = extent.height;
	framebuffer_info.layers = 1;
		
	framebuffer_info.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;

	if (vkCreateFramebuffer(device_cfg_.logical_device, &framebuffer_info, nullptr, &handle_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer!");
	}
}


render::Framebuffer::~Framebuffer()
{
	if (handle_ != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(device_cfg_.logical_device, handle_, nullptr);
	}
}

render::Extent render::Framebuffer::GetExtent() const
{
	return extent_;
}

void render::Framebuffer::Build(const RenderPass2& render_pass)
{
	if (handle_ != VK_NULL_HANDLE)
		throw std::runtime_error("Framebuffer is already built");

	std::vector<VkImageView> vk_attachments_;

	VkFramebufferCreateInfo framebuffer_info{};
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.renderPass = render_pass.GetHandle();
	framebuffer_info.attachmentCount = u32(attachments_descs_.size());
	framebuffer_info.pAttachments = vk_attachments_.data();
	framebuffer_info.width = extent_.width;
	framebuffer_info.height = extent_.height;
	framebuffer_info.layers = 1;

	if (vkCreateFramebuffer(device_cfg_.logical_device, &framebuffer_info, nullptr, &handle_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer!");
	}
}

const std::vector<render::Framebuffer::AttachmentDesc>& render::Framebuffer::GetAttachmentsDescs() const
{
	return attachments_descs_;
}
