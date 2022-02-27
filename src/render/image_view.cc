#include "image_view.h"

render::ImageView::ImageView(const DeviceConfiguration& device_cfg, const Image& image): RenderObjBase(device_cfg)
{
	VkImageViewCreateInfo view_info{};

	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image.GetHandle();
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = image.GetFromat();
	
	if		(image.GetImageType() == Image::ImageType::kColorImage) view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	else if (image.GetImageType() == Image::ImageType::kSwapchainImage) view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	else if (image.GetImageType() == Image::ImageType::kDepthImage) view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;

	if (vkCreateImageView(device_cfg_.logical_device, &view_info, nullptr, &handle_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}
}


render::ImageView::~ImageView()
{
	if (handle_ != VK_NULL_HANDLE)
	{
		vkDestroyImageView(device_cfg_.logical_device, handle_, nullptr);
	}
}

