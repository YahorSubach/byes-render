#include "framebuffer.h"

#include <array>

#include "common.h"

render::Framebuffer::Framebuffer(const DeviceConfiguration& device_cfg, const ConstructParams& params):
	LazyRenderObj(device_cfg), extent_(params.extent), render_pass_(params.render_pass)
{
	std::vector<VkImageView> vk_attachments;

	for (auto&& image_view : params.attachments)
	{
		vk_attachments.push_back(image_view.get().GetHandle());
	}

	//int attachments_cnt = render_pass_.GetAttachmentsCnt();

	//for (int att_ind = 0; att_ind < attachments_cnt; att_ind++)
	//{
	//	assert(name_to_image_view_.count(render_pass_.GetAttachmentByIndex(att_ind).name) > 0);
	//}

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

//const std::vector<std::reference_wrapper<const render::ImageView>>& render::Framebuffer::GetAttachmentImageViews() const
//{
//	return image_views_;
//}

const render::RenderPass& render::Framebuffer::GetRenderPass() const
{
	return render_pass_;
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

//int render::Framebuffer::AddAttachment(const std::string_view& name, const ImageView& image_view)
//{
//	assert(name_to_image_view_.count(std::string(name)) == 0);
//
//	int framebuffer_index = render_pass_.GetAttachmentIndex(name);
//	assert(framebuffer_index >= 0);
//
//	auto&& attachment = render_pass_.GetAttachmentByIndex(framebuffer_index);
//
//	assert(image_view.GetFormat() == attachment.desc.format);
//
//	name_to_image_view_.insert({ std::string(name), image_view });
//	
//	if (attachment.is_depth_attachment)
//	{
//		image_view.AddUsageFlag(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
//	}
//	else
//	{
//		image_view.AddUsageFlag(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
//	}
//
//	image_views_.push_back(image_view);
//
//	return image_views_.size() - 1;
//}

//const render::ImageView& render::Framebuffer::GetAttachment(const std::string_view& name) const
//{
//	return name_to_image_view_.at(std::string(name));
//}

bool render::Framebuffer::InitHandle() const
{

	

	return true;
}