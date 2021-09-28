#include "framebuffer.h"

#include "common.h"

render::Framebuffer::Framebuffer(const VkDevice& device, const VkExtent2D& extent, const VkImageView& image_view, const RenderPass& render_pass): 
	RenderObjBase(device), framebuffer_(VK_NULL_HANDLE)
{
	VkImageView attachments[] = {
		image_view
	};

	VkFramebufferCreateInfo framebuffer_info{};
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.renderPass = render_pass.GetRenderPassHandle();
	framebuffer_info.attachmentCount = 1;
	framebuffer_info.pAttachments = attachments;
	framebuffer_info.width = extent.width;
	framebuffer_info.height = extent.height;
	framebuffer_info.layers = 1;

	if (vkCreateFramebuffer(device_, &framebuffer_info, nullptr, &framebuffer_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer!");
	}
}

const VkFramebuffer& render::Framebuffer::GetFramebufferHandle() const
{
	return framebuffer_;
}

render::Framebuffer::~Framebuffer()
{
	if (framebuffer_ != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(device_, framebuffer_, nullptr);
	}
}
