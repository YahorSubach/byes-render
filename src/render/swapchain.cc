#include "swapchain.h"

#include "stl_util.h"

#include "common.h"

#include "surface.h"

render::Swapchain::Swapchain(const DeviceConfiguration& device_cfg, const Surface& surface) : RenderObjBase(device_cfg), extent_(), format_()
{
	VkSurfaceCapabilitiesKHR capabilities;
	VkBool32 device_surface_support;

	valid_ = false;

	if (vkGetPhysicalDeviceSurfaceSupportKHR(device_cfg.physical_device, device_cfg.graphics_queue_index, surface.GetHandle(), &device_surface_support) == VK_SUCCESS &&
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_cfg.physical_device, surface.GetHandle(), &capabilities) == VK_SUCCESS)
	{
		if (device_surface_support)
		{
			VkSurfaceFormatKHR surface_format = surface.GetSurfaceFormat(device_cfg.physical_device);
			VkPresentModeKHR present_mode = surface.GetSurfacePresentMode(device_cfg.physical_device);

			extent_ = surface.GetSwapExtend(capabilities);
			format_ = surface_format.format;

			VkSwapchainCreateInfoKHR create_info{};
			VkSwapchainKHR old_swapchain = nullptr;

			create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			create_info.pNext = nullptr;
			create_info.flags = 0;
			create_info.surface = surface.GetHandle();
			create_info.minImageCount = 3/*capabilities.minImageCount*/;
			create_info.imageFormat = surface_format.format;
			create_info.imageColorSpace = surface_format.colorSpace;
			create_info.imageExtent = extent_; // shoud be fixed for retina
			create_info.imageArrayLayers = 1;
			create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			create_info.queueFamilyIndexCount = 0;
			create_info.pQueueFamilyIndices = nullptr;
			create_info.preTransform = capabilities.currentTransform;
			create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			create_info.presentMode = present_mode;
			create_info.clipped = VK_FALSE;
			create_info.oldSwapchain = old_swapchain;

			if (vkCreateSwapchainKHR(device_cfg_.logical_device, &create_info, nullptr, &handle_) == VK_SUCCESS)
			{
				auto image_handles = util::GetSizeThenAlocThenGetDataPtrPtr(vkGetSwapchainImagesKHR, device_cfg_.logical_device, handle_);

				images_.reserve(16);
				image_views_.reserve(16);

				for (auto&& image_handle : image_handles)
				{
					images_.push_back(Image(device_cfg, surface_format.format, image_handle/*, {ImageProperty::kShouldNotFreeHandle}*/));
					image_views_.emplace_back(device_cfg, images_.back());
				}
			}
		}
	}
}

render::Extent render::Swapchain::GetExtent() const
{
	return extent_;
}

VkFormat render::Swapchain::GetFormat() const
{
	return format_;
}

size_t render::Swapchain::GetImagesCount() const
{
	return images_.size();
}

const render::Image& render::Swapchain::GetImage(size_t index) const
{
	return images_[index];
}

const render::ImageView& render::Swapchain::GetImageView(size_t index) const
{
	return image_views_[index];
}

render::Swapchain::~Swapchain()
{
	image_views_.clear();

	if (handle_ != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(device_cfg_.logical_device, handle_, nullptr);
	}
}