#include "image_view.h"

#include "global.h"

render::ImageView::ImageView(const Global& global) :RenderObjBase(global), format_(VK_FORMAT_UNDEFINED)
{
}

render::ImageView::ImageView(const Global& global, const Image& image): RenderObjBase(global), format_(VK_FORMAT_UNDEFINED)
{
	Assign(image);
}


void render::ImageView::Assign(const Image& image)
{
	if (handle_ != VK_NULL_HANDLE)
	{
		//TODO: FIX THIS HACK!
		return;
		throw std::runtime_error("Image already assigned");
	}

//	std::optional<std::reference_wrapper<ReferencedType>>

	VkImageViewCreateInfo view_info{};

	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image.GetHandle();
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = image.GetFormat();

	format_ = image.GetFormat();

	if (image.CheckUsageFlag(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
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
	view_info.subresourceRange.levelCount = image.GetMipMapLevelsCount();
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;

	if (vkCreateImageView(global_.logical_device, &view_info, nullptr, &handle_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}
}

//uint32_t render::ImageView::AddUsageFlag(uint32_t flag) const
//{
//	//assert(image_);
//
//	return image_->AddUsageFlag(flag);
//}
//
//uint32_t render::ImageView::CheckUsageFlag(uint32_t flag) const
//{
//	//assert(image_);
//
//	return image_->CheckUsageFlag(flag);
//}

//VkFormat render::ImageView::GetFormat() const
//{
//	if (image_)
//		return image_->GetFormat();
//	return VK_FORMAT_UNDEFINED;
//}

//bool render::ImageView::InitHandle() const
//{
//	assert(image_);
//
//
//
//}

VkFormat render::ImageView::GetFormat() const
{
	return format_;
}

render::ImageView::~ImageView()
{
	if (handle_ != VK_NULL_HANDLE)
	{
		vkDestroyImageView(global_.logical_device, handle_, nullptr);
	}
}

//render::util::NullableRef<const render::Image> render::ImageView::GetImage() const
//{
//	return image_;
//}

