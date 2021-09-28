#ifndef RENDER_ENGINE_RENDER_PLATFORM_H_
#define RENDER_ENGINE_RENDER_PLATFORM_H_

#include "vulkan/vulkan.h"

#include "render/render_engine.h"

#include <thread>
#include <vector>

namespace render::platform
{
#ifdef WIN32
	using Window = HWND;
#endif

	Window CreatePlatformWindow(InitParam param);

	void DestroyPlatformWindow(Window window);

	void ShowWindow(Window window);

	void JoinWindowThread(Window window);

	bool IsWindowClosed(Window window);

	const std::vector<const char*>& GetRequiredExtensions();

	bool GetPhysicalDevicePresentationSupport(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);

	bool CreateSurface(const VkInstance& instance, const Window& window, VkSurfaceKHR& surface);

	VkExtent2D GetWindowExtent(const Window& window);
}


#endif  // RENDER_ENGINE_RENDER_PLATFORM_H_
