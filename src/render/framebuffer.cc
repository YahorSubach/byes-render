#include "framebuffer.h"

#include <array>

#include "common.h"

render::Framebuffer::Framebuffer(const DeviceConfiguration& device_cfg, Extent extent, const RenderPass& render_pass):
	LazyRenderObj(device_cfg), extent_(extent), render_pass_(render_pass)
{

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

const std::vector<std::reference_wrapper<const render::ImageView>>& render::Framebuffer::GetAttachments() const
{
	return image_views_;
}

//void render::Framebuffer::Build(const RenderPass2& render_pass)
//{
//	if (handle_ != VK_NULL_HANDLE)
//		throw std::runtime_error("Framebuffer is already built");
//
//	std::vector<VkImageView> vk_attachments_;
//
//	VkFramebufferCreateInfo framebuffer_info{};
//	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//	framebuffer_info.renderPass = render_pass.GetHandle();
//	framebuffer_info.attachmentCount = u32(attachments_descs_.size());
//	framebuffer_info.pAttachments = vk_attachments_.data();
//	framebuffer_info.width = extent_.width;
//	framebuffer_info.height = extent_.height;
//	framebuffer_info.layers = 1;
//
//	if (vkCreateFramebuffer(device_cfg_.logical_device, &framebuffer_info, nullptr, &handle_) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create framebuffer!");
//	}
//}

//const std::vector<render::Framebuffer::AttachmentDesc>& render::Framebuffer::GetAttachmentsDescs() const
//{
//	return attachments_descs_;
//}
//

int render::Framebuffer::AddAttachment(const std::string_view& name, const ImageView& image_view)
{
	assert(attached_.count(std::string(name)) == 0);
	attached_.insert(std::string(name));
	
	int framebuffer_index = render_pass_.GetAttachmentIndex(name);
	assert(framebuffer_index >= 0);

	auto&& attachment = render_pass_.GetAttachmentByIndex(framebuffer_index);

	if (attachment.is_depth_attachment)
	{
		image_view.AddUsageFlag(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
	else
	{
		image_view.AddUsageFlag(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	}

	image_views_.push_back(image_view);

	return image_views_.size() - 1;
}

bool render::Framebuffer::InitHandle() const
{

	std::vector<VkImageView> vk_attachments;
	
	for (auto&& image_view : image_views_)
	{
		vk_attachments.push_back(image_view.get().GetHandle());
	}

	VkFramebufferCreateInfo framebuffer_info{};
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.renderPass = render_pass_.GetHandle();
	framebuffer_info.attachmentCount = u32(vk_attachments.size());
	framebuffer_info.pAttachments = vk_attachments.data();
	framebuffer_info.width = extent_.width;
	framebuffer_info.height = extent_.height;
	framebuffer_info.layers = 1;

	framebuffer_info.flags = 0;

	if (vkCreateFramebuffer(device_cfg_.logical_device, &framebuffer_info, nullptr, &handle_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer!");
	}

	return true;
}