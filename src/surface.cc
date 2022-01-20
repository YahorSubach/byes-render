
#include "common.h"
#include "surface.h"

#include "stl_util.h"

#include <functional>




render::Surface::Surface(platform::Window window_hande, const VkInstance& instance, const DeviceConfiguration& device_cfg) :
	RenderObjBase(device_cfg.logical_device), window_hande_(window_hande), physical_device_(device_cfg.physical_device), queue_index_(device_cfg.graphics_queue_index), surface_(VK_NULL_HANDLE)
{
	if (window_hande_)
	{
		if (platform::CreateSurface(instance, window_hande_, surface_))
		{
			valid_ = true;
		}
	}
}

const VkImage* render::Surface::AcquireImage()
{
	if (valid_)
	{
		//uint32_t image_index{};

		//if(vkAcquireNextImageKHR(logical_device_, swapchain_, 0, VkSemaphore)
		return nullptr;
	}
	return nullptr;
}

const std::vector<VkImageView>& render::Surface::GetImageViews() const
{
	return images_views_;
}

render::platform::Window render::Surface::GetWindow()
{
	return window_hande_;
}

bool render::Surface::RefreshSwapchain()
{
	images_.clear();
	VkSurfaceCapabilitiesKHR capabilities;

	if (VkBool32 device_surface_support; vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_, queue_index_, surface_, &device_surface_support) == VK_SUCCESS) // TODO move this upper
	{
		if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &capabilities) == VK_SUCCESS && device_surface_support) // TODO move this to swapchhain class
		{

			VkSurfaceFormatKHR surface_format = GetSurfaceFormat(physical_device_);
			VkPresentModeKHR present_mode = GetSurfacePresentMode(physical_device_);

			VkSwapchainCreateInfoKHR create_info{};
			VkSwapchainKHR old_swapchain = nullptr;


			swapchain_extent_ = GetSwapExtend(capabilities);

			create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			create_info.pNext = nullptr;
			create_info.flags = 0;
			create_info.surface = surface_;
			create_info.minImageCount = capabilities.minImageCount;
			create_info.imageFormat = surface_format.format;
			create_info.imageColorSpace = surface_format.colorSpace;
			create_info.imageExtent = swapchain_extent_; // shoud be fixed for retina
			create_info.imageArrayLayers = 1;
			create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			create_info.queueFamilyIndexCount = 0;
			create_info.pQueueFamilyIndices = nullptr;
			create_info.preTransform = capabilities.currentTransform;
			create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			create_info.presentMode = present_mode;
			create_info.clipped = VK_FALSE;
			create_info.oldSwapchain = handle_;

			old_swapchain = handle_;

			if (vkCreateSwapchainKHR(device_, &create_info, nullptr, &handle_) == VK_SUCCESS)
			{
				images_ = stl_util::GetSizeThenAlocThenGetDataPtrPtr(vkGetSwapchainImagesKHR, device_, handle_);

				for (auto&& image_view : images_views_)
				{
					vkDestroyImageView(device_, image_view, nullptr);
				}

				images_views_.resize(images_.size());

				for (size_t i = 0; i < images_.size(); i++)
				{
					VkImageViewCreateInfo createInfo{};
					createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
					createInfo.image = images_[i];
					createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
					createInfo.format = surface_format.format;

					createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
					createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
					createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
					createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

					createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					createInfo.subresourceRange.baseMipLevel = 0;
					createInfo.subresourceRange.levelCount = 1;
					createInfo.subresourceRange.baseArrayLayer = 0;
					createInfo.subresourceRange.layerCount = 1;

					if (vkCreateImageView(device_, &createInfo, nullptr, &images_views_[i]) != VK_SUCCESS)
					{
						LOG(err, "vkCreateImageView error");
						return VK_NULL_HANDLE;
					}

				}

				swapchain_image_format_ = surface_format.format;
				swapchain_extent_ = swapchain_extent_;
			}

			if (old_swapchain != nullptr)
			{
				vkDestroySwapchainKHR(device_, old_swapchain, nullptr);
			}
		}
	}
	return false;
}

const VkFormat& render::Surface::GetSwapchainFormat()
{
	return swapchain_image_format_;
}

const VkExtent2D& render::Surface::GetSwapchainExtent()
{
	return swapchain_extent_;
}

const VkSwapchainKHR& render::Surface::GetSwapchain()
{
	return handle_;
}

render::Surface::~Surface()
{
	if (handle_ != VK_NULL_HANDLE)
	{
		for (auto&& image_view : images_views_)
		{
			vkDestroyImageView(device_, image_view, nullptr);
		}

		vkDestroySwapchainKHR(device_, handle_, nullptr);
	}
}

VkSurfaceFormatKHR render::Surface::GetSurfaceFormat(const VkPhysicalDevice& physical_device)
{
	auto formats = stl_util::GetSizeThenAlocThenGetDataPtrPtr(vkGetPhysicalDeviceSurfaceFormatsKHR, physical_device, surface_);
	return formats[0];
}

VkPresentModeKHR render::Surface::GetSurfacePresentMode(const VkPhysicalDevice& physical_device)
{
	auto presentat_modes = stl_util::GetSizeThenAlocThenGetDataPtrPtr(vkGetPhysicalDeviceSurfacePresentModesKHR, physical_device, surface_);
	return presentat_modes[3];
}

VkExtent2D render::Surface::GetSwapExtend(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D extent = platform::GetWindowExtent(window_hande_);

		extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return extent;
	}
}
