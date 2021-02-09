#include "surface.h"

vkvf::Surface::Surface(InitParam init_param, const VkInstance& instance, const VkPhysicalDevice& physical_device, const VkDevice& logical_device) :
	is_valid_(false), logical_device_(logical_device)
{
	window_hande_ = vkvf::platform::CreatePlatformWindow(init_param);

	if (window_hande_)
	{
		VkWin32SurfaceCreateInfoKHR create_info =
		{
			VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR,
			nullptr,
			0,
			nullptr,
			window_hande_
		};

		if (vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &surface_) == VK_SUCCESS)
		{
			VkSurfaceCapabilitiesKHR capabilities;

			if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface_, &capabilities) == VK_SUCCESS)
			{
				VkSwapchainCreateInfoKHR create_info;

				create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
				create_info.pNext = nullptr;
				create_info.flags = 0;
				create_info.surface = surface_;
				create_info.minImageCount = capabilities.minImageCount;
				create_info.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
				create_info.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
				create_info.imageExtent = capabilities.currentExtent;
				create_info.imageArrayLayers = 1;
				create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				create_info.queueFamilyIndexCount = 0;
				create_info.pQueueFamilyIndices = nullptr;
				create_info.preTransform = capabilities.currentTransform;
				create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
				create_info.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
				create_info.clipped = VK_FALSE;
				create_info.oldSwapchain = swapchain_;

				if (vkCreateSwapchainKHR(logical_device, &create_info, nullptr, &swapchain_) == VK_SUCCESS)
				{
					uint32_t images_count;

					if (vkGetSwapchainImagesKHR(logical_device, swapchain_, &images_count, nullptr) == VK_SUCCESS)
					{
						images_.resize(images_count);
						if (vkGetSwapchainImagesKHR(logical_device, swapchain_, &images_count, images_.data()) == VK_SUCCESS)
						{
							is_valid_ = true;
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
		uint32_t image_index;

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
