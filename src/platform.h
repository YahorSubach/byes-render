#ifndef VK_VISUAL_FACADE_PLATFORM_H_
#define VK_VISUAL_FACADE_PLATFORM_H_

#include "vkvf.h"
#include <vector>


namespace vkvf::platform
{
#ifdef WIN32
	using Window = HWND;
#endif

	Window CreatePlatformWindow(InitParam param);

	void DestroyPlatformWindow(Window window);

	void ShowWindow(Window window);

	const std::vector<const char*>& GetRequiredExtensions();


}


#endif  // VK_VISUAL_FACADE_PLATFORM_H_