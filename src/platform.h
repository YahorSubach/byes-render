#ifndef VK_VISUAL_FACADE_PLATFORM_H_
#define VK_VISUAL_FACADE_PLATFORM_H_

#include "vulkan/vulkan.h"

#include "vkvf.h"

#include <thread>
#include <vector>

namespace vkvf::platform
{
#ifdef WIN32
	using Window = HWND;
#endif

	Window CreatePlatformWindow(InitParam param);

	void DestroyPlatformWindow(Window window);

	void ShowWindow(Window window);

	bool IsWindowClosed(Window window);

	const std::vector<const char*>& GetRequiredExtensions();

	bool GetPhysicalDevicePresentationSupport(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
}


#endif  // VK_VISUAL_FACADE_PLATFORM_H_