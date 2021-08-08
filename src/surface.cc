
#include "common.h"
#include "surface.h"

#include "stl_util.h"

#include <functional>

template<class T>
struct TT
{
	void F(T t)
	{

	}

	template<class C>
	TT(C c)
	{
	}
};

template<class C>
TT(C c)->TT<C>;

vkvf::Surface::Surface(InitParam init_param, const VkInstance& instance, const VkPhysicalDevice& physical_device, uint32_t queue_index, const VkDevice& logical_device) :
	is_valid_(false), logical_device_(logical_device), surface_(nullptr), swapchain_(nullptr)
{
	TT t = 1;

	window_hande_ = vkvf::platform::CreatePlatformWindow(init_param);

	if (window_hande_)
	{
		VkWin32SurfaceCreateInfoKHR create_info =
		{
			VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			nullptr,
			0,
			init_param,
			window_hande_
		};

		if (vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &surface_) == VK_SUCCESS)
		{
			VkSurfaceCapabilitiesKHR capabilities;

			
			if (VkBool32 device_surface_support; vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_index, surface_, &device_surface_support) == VK_SUCCESS)
			{
				if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface_, &capabilities) == VK_SUCCESS && device_surface_support)
				{
					
					VkSurfaceFormatKHR surface_format = GetSurfaceFormat(physical_device);
					VkPresentModeKHR present_mode = GetSurfacePresentMode(physical_device);

					VkSwapchainCreateInfoKHR create_info{};

					create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
					create_info.pNext = nullptr;
					create_info.flags = 0;
					create_info.surface = surface_;
					create_info.minImageCount = capabilities.minImageCount;
					create_info.imageFormat = surface_format.format;
					create_info.imageColorSpace = surface_format.colorSpace;
					create_info.imageExtent = capabilities.currentExtent;
					create_info.imageArrayLayers = 1;
					create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
					create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
					create_info.queueFamilyIndexCount = 0;
					create_info.pQueueFamilyIndices = nullptr;
					create_info.preTransform = capabilities.currentTransform;
					create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
					create_info.presentMode = present_mode;
					create_info.clipped = VK_FALSE;
					create_info.oldSwapchain = nullptr;

					//vkGetPhysicalDeviceSurfaceSupportKHR()

					if (vkCreateSwapchainKHR(logical_device, &create_info, nullptr, &swapchain_) == VK_SUCCESS)
					{
						uint32_t images_count;

						images_ = stl_util::GetSizeThenAlocThenGetDataPtrPtr(vkGetSwapchainImagesKHR, logical_device, swapchain_);

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

							if (vkCreateImageView(logical_device, &createInfo, nullptr, &images_views_[i]) != VK_SUCCESS)
							{
								LOG(err, "vkCreateImageView error");
							}

						}

					}

				}
			}
		}
	}
}

const VkImage* vkvf::Surface::AcquireImage()
{
	if (is_valid_)
	{
		//uint32_t image_index{};

		//if(vkAcquireNextImageKHR(logical_device_, swapchain_, 0, VkSemaphore)
		return nullptr;
	}
	return nullptr;
}

bool vkvf::Surface::Valid()
{
	return is_valid_;
}

vkvf::platform::Window vkvf::Surface::GetWindow()
{
	return window_hande_;
}

vkvf::Surface::~Surface()
{
	if (window_hande_)
		vkvf::platform::DestroyPlatformWindow(window_hande_);
}

VkSurfaceFormatKHR vkvf::Surface::GetSurfaceFormat(const VkPhysicalDevice& physical_device)
{
	auto formats = stl_util::GetSizeThenAlocThenGetDataPtrPtr(vkGetPhysicalDeviceSurfaceFormatsKHR, physical_device, surface_);
	return formats[0];
}

VkPresentModeKHR vkvf::Surface::GetSurfacePresentMode(const VkPhysicalDevice& physical_device)
{
	auto presentat_modes = stl_util::GetSizeThenAlocThenGetDataPtrPtr(vkGetPhysicalDeviceSurfacePresentModesKHR, physical_device, surface_);
	return presentat_modes[3];
}
