
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

	if (formats.size() == 0)
		throw std::runtime_error("No surface formats detected. Epic fail:(");


	for (auto format : formats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB)
			return format;
	}

	return formats[0]; //TODO fuck
}

VkPresentModeKHR render::Surface::GetSurfacePresentMode(const VkPhysicalDevice& physical_device) const
{
	auto presentat_modes = stl_util::GetSizeThenAlocThenGetDataPtrPtr(vkGetPhysicalDeviceSurfacePresentModesKHR, physical_device, handle_);
	
	for (auto mode : presentat_modes)
	{
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			return mode;
	}

	for (auto mode : presentat_modes)
	{
		if (mode == VK_PRESENT_MODE_FIFO_KHR)
			return mode;
	}

	for (auto mode : presentat_modes)
	{
		return mode;
	}
	
	
	return presentat_modes[2]; //TODO fuck
}

render::Extent render::Surface::GetSwapExtend(const VkSurfaceCapabilitiesKHR& capabilities) const
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

