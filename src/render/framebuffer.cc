#include "framebuffer.h"

#include <array>

#include "common.h"

render::Framebuffer::Framebuffer(const DeviceConfiguration& device_cfg, const Extent& extent, const ImageView& color_image_view, const ImageView& depth_image_view, const RenderPass& render_pass):
	RenderObjBase(device_cfg), extent_(extent)
{
	std::array<VkImageView, 2> attachments = {
		color_image_view.GetHandle(),
		depth_image_view.GetHandle()
	};

	VkFramebufferCreateInfo framebuffer_info{};
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.renderPass = render_pass.GetHandle();
	framebuffer_info.attachmentCount = attachments.size();
	framebuffer_info.pAttachments = attachments.data();
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
