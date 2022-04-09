#include "framebuffer.h"

#include <array>

#include "common.h"

render::Framebuffer::Framebuffer(const DeviceConfiguration& device_cfg, const Extent& extent, const std::vector<std::reference_wrapper<const ImageView>>& attachments, const RenderPass& render_pass):
	RenderObjBase(device_cfg), extent_(extent)
{

	std::vector<VkImageView> vk_attachments;

	for (auto&& attachment : attachments)
	{
		vk_attachments.push_back(attachment.get().GetHandle());
	}

	VkFramebufferCreateInfo framebuffer_info{};
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.renderPass = render_pass.GetHandle();
	framebuffer_info.attachmentCount = vk_attachments.size();
	framebuffer_info.pAttachments = vk_attachments.data();
	framebuffer_info.width = extent.width;
	framebuffer_info.height = extent.height;
	framebuffer_info.layers = 1;

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
