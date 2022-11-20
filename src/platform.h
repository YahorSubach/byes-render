#ifndef RENDER_ENGINE_RENDER_PLATFORM_H_
#define RENDER_ENGINE_RENDER_PLATFORM_H_

#include "vulkan/vulkan.h"

#include "render/render_engine.h"

#include <thread>
#include <vector>
#include <array>

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

	const std::vector<const char*>& GetRequiredInstanceExtensions();

	bool GetPhysicalDevicePresentationSupport(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);

	bool CreateSurface(const VkInstance& instance, const Window& window, VkSurfaceKHR& surface);

	VkExtent2D GetWindowExtent(const Window& window);

	void GetMouseDelta(int& x_delta, int& y_delta);

	std::array<bool, 'z' - 'a' + 1>& GetButtonState();
}


#endif  // RENDER_ENGINE_RENDER_PLATFORM_H_
