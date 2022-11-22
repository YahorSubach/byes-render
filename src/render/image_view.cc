#include "image_view.h"

render::ImageView::ImageView(const DeviceConfiguration& device_cfg) :LazyRenderObj(device_cfg)
{
}

render::ImageView::ImageView(const DeviceConfiguration& device_cfg, const Image& image): LazyRenderObj(device_cfg)
{
	Assign(image);
}


void render::ImageView::Assign(const Image& image)
{
	if (image_)
	{
		//TODO: FIX THIS HACK!
		return;
		throw std::runtime_error("Image already assigned");
	}

//	std::optional<std::reference_wrapper<ReferencedType>>

	image_ = image;
}

uint32_t render::ImageView::AddUsageFlag(uint32_t flag) const
{
	assert(image_);

	return image_->AddUsageFlag(flag);
}

uint32_t render::ImageView::CheckUsageFlag(uint32_t flag) const
{
	assert(image_);

	return image_->CheckUsageFlag(flag);
}

VkFormat render::ImageView::GetFormat() const
{
	if (image_)
		return image_->GetFormat();
	return VK_FORMAT_UNDEFINED;
}

bool render::ImageView::InitHandle() const
{
	assert(image_);


	VkImageViewCreateInfo view_info{};

	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image_->GetHandle();
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = image_->GetFormat();

	if (image_->CheckUsageFlag(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
	{
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else
	{
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	view_info.components.r = VK_COMPONENT_SWIZZLE_R;
	view_info.components.g = VK_COMPONENT_SWIZZLE_G;
	view_info.components.b = VK_COMPONENT_SWIZZLE_B;
	view_info.components.a = VK_COMPONENT_SWIZZLE_A;

	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = image_->GetMipMapLevelsCount();
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

render::stl_util::NullableRef<const render::Image> render::ImageView::GetImage() const
{
	return image_;
}

