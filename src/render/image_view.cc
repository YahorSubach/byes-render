#include "image_view.h"

render::ImageView::ImageView(const DeviceConfiguration& device_cfg):RenderObjBase(device_cfg), image_type_(Image::ImageType::kUndefined)
{
}

render::ImageView::ImageView(const DeviceConfiguration& device_cfg, const Image& image): RenderObjBase(device_cfg), image_type_(Image::ImageType::kUndefined)
{
	Assign(image);
}


void render::ImageView::Assign(const Image& image)
{
	image_type_ = image.GetImageType();

	if (handle_ != VK_NULL_HANDLE)
	{
		//TODO: FIX THIS HACK!
		return;
		throw std::runtime_error("Image already assigned");
	}

	VkImageViewCreateInfo view_info{};

	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image.GetHandle();
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = image.GetFormat();

	if (image_type_ == Image::ImageType::kColorImage) view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	else if (image_type_ == Image::ImageType::kSwapchainImage) view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	else if (image_type_ == Image::ImageType::kDepthImage) view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	else if (image_type_ == Image::ImageType::kBitmapImage) view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	view_info.components.r = VK_COMPONENT_SWIZZLE_R;
	view_info.components.g = VK_COMPONENT_SWIZZLE_G;
	view_info.components.b = VK_COMPONENT_SWIZZLE_B;
	view_info.components.a = VK_COMPONENT_SWIZZLE_A;

	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = image.GetMipMapLevelsCount();
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;

	if (vkCreateImageView(device_cfg_.logical_device, &view_info, nullptr, &handle_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

	image_type_ = image.GetImageType();
}

render::Image::ImageType render::ImageView::GetImageType() const
{
	return image_type_;
}

render::ImageView::~ImageView()
{
	if (handle_ != VK_NULL_HANDLE)
	{
		vkDestroyImageView(device_cfg_.logical_device, handle_, nullptr);
	}
}

