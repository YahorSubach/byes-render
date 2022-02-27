
#include "common.h"
#include "surface.h"

#include "stl_util.h"

#include <functional>

render::Surface::Surface(platform::Window window_hande, const VkInstance& instance, const DeviceConfiguration& device_cfg) :
	RenderObjBase(device_cfg), window_hande_(window_hande)
{
	if (window_hande_)
	{
		if (platform::CreateSurface(instance, window_hande_, handle_))
		{
			valid_ = true;
		}
	}
}

render::platform::Window render::Surface::GetWindow()
{
	return window_hande_;
}

render::Surface::~Surface()
{
	if (handle_ != VK_NULL_HANDLE)
	{
	}
}

VkSurfaceFormatKHR render::Surface::GetSurfaceFormat(const VkPhysicalDevice& physical_device) const
{
	auto formats = stl_util::GetSizeThenAlocThenGetDataPtrPtr(vkGetPhysicalDeviceSurfaceFormatsKHR, physical_device, handle_);
	return formats[0]; //TODO fuck
}

VkPresentModeKHR render::Surface::GetSurfacePresentMode(const VkPhysicalDevice& physical_device) const
{
	auto presentat_modes = stl_util::GetSizeThenAlocThenGetDataPtrPtr(vkGetPhysicalDeviceSurfacePresentModesKHR, physical_device, handle_);
	return presentat_modes[2]; //TODO fuck
}

VkExtent2D render::Surface::GetSwapExtend(const VkSurfaceCapabilitiesKHR& capabilities) const
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

