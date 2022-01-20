#include "image_view.h"

render::ImageView::ImageView(const DeviceConfiguration& device_cfg, const Image& image): RenderObjBase(device_cfg.logical_device)
{
	VkImageViewCreateInfo view_info{};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image.GetImageHandle();
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = image.GetFromat();
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;

	if (vkCreateImageView(device_, &view_info, nullptr, &handle_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

}

render::ImageView::~ImageView()
{
	if (handle_ != VK_NULL_HANDLE)
	{
		vkDestroyImageView(device_, handle_, nullptr);
	}
}
