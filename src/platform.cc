#include "platform.h"

#include "windowsx.h"

#include<atomic>
#include<iostream>

namespace render::platform
{
#ifdef WIN32
	using Window = HWND;

	int mouse_x = 0;
	int mouse_y = 0;

	std::array<bool, 'z' - 'a' + 1> buttons;

	constexpr int kVkCharStart = 0x41;


	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_SETFOCUS:
		{

			ShowCursor(false);

			RECT rect;
			GetWindowRect(hwnd, &rect);
			ClipCursor(&rect);


			break;
		}

		case WM_KILLFOCUS:
		{
			for (auto&& button : buttons)
			{
				button = false;
			}

			break;
		}

		case WM_KEYDOWN:
		{
			if (wParam >= kVkCharStart && wParam < (kVkCharStart + 'z' - 'a' + 1))
			{
				buttons[wParam - kVkCharStart] = true;
				break;
			}

			switch (wParam)
			{
			case VK_ESCAPE:
				ShowCursor(true);
				ClipCursor(nullptr);
				// Process the LEFT ARROW key. 

				break;
			}

			break;
		}
		
		case WM_KEYUP:
		{
			if (wParam >= kVkCharStart && wParam < (kVkCharStart + 'z' - 'a' + 1))
			{
				buttons[wParam - kVkCharStart] = false;
				break;
			}

			break;
		}

		case WM_MOUSEMOVE:
		{
			RECT cursor_rect;

			GetClipCursor(&cursor_rect);

			RECT rect;
			GetWindowRect(hwnd, &rect);

			int mr = memcmp(&cursor_rect, &rect, sizeof(RECT));

			if (mr != 0)
				break;

			int rect_x_center = rect.left + (rect.right - rect.left) / 2;
			int rect_y_center = rect.top + (rect.bottom - rect.top) / 2;

			POINT point;
			GetCursorPos(&point);

			int x = point.x;
			int y = point.y;

			if (x == rect_x_center && y == rect_y_center)
				break;

			mouse_x += (x - rect_x_center);
			mouse_y += (y - rect_y_center);

			SetCursorPos(rect_x_center, rect_y_center);

			break;
		}
 

		//case WM_PAINT:
		//{
		//	//PAINTSTRUCT ps;
		//	//HDC hdc = BeginPaint(hwnd, &ps);



		//	//FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

		//	//EndPaint(hwnd, &ps);
		//}
		return 0;
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	std::thread window_thread;
	std::atomic_bool window_created;
	Window window;
	bool window_closed = false;
	
	Window CreatePlatformWindow(InitParam param)
	{
		window_created = false;

		window_thread = (std::thread([param]() {

			const wchar_t CLASS_NAME[] = L"render WINDOW CLASS";

			WNDCLASSW wc = { };

			wc.lpfnWndProc = WindowProc;
			wc.hInstance = param;
			wc.lpszClassName = CLASS_NAME;


			RegisterClassW(&wc);

			window = CreateWindowExW(
				0,                              // Optional window styles.
				CLASS_NAME,                     // Window class
				L"Vulkan Visual Facade",    // Window text
				WS_OVERLAPPEDWINDOW,            // Window style

				// Size and position
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

				NULL,       // Parent window    
				NULL,       // Menu
				param,  // Instance handle
				NULL        // Additional application data
			);

			window_created = true;

			if (window)
			{
				ShowWindow(window, 1);
			}

			MSG msg = {};

			while (GetMessage(&msg, NULL, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			window_closed = true;

			}));

		while (!window_created)
		{
			Sleep(100);
		}

		return window;
	}

	void DestroyPlatformWindow(Window window)
	{
		DestroyWindow(window);
	}



	void ShowWindow(Window window)
	{

	}

	void JoinWindowThread(Window window)
	{
		window_thread.join();
	}

	bool IsWindowClosed(Window window)
	{
		return window_closed;
	}


	const std::vector<const char*>& GetRequiredExtensions()
	{
		static const std::vector<const char*> extensions{ "VK_KHR_surface", "VK_KHR_win32_surface"};
		return extensions;
	}

	bool GetPhysicalDevicePresentationSupport(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex)
	{
		return vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
	}

	bool CreateSurface(const VkInstance& instance, const Window& window, VkSurfaceKHR& surface)
	{
		VkWin32SurfaceCreateInfoKHR create_info =
		{
			VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			nullptr,
			0,
			GetModuleHandle(NULL),
			window
		};

		return vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &surface) == VK_SUCCESS;
	}

	VkExtent2D GetWindowExtent(const Window& window)
	{
		VkExtent2D extent{};

		RECT rect;
		if (GetWindowRect(window, &rect))
		{
			extent.width = rect.right - rect.left;
			extent.height = rect.bottom - rect.top;
		}

		return extent;
	}

	int last_mouse_x = 0;
	int last_mouse_y = 0;

	void GetMouseDelta(int& x_delta, int& y_delta)
	{
		x_delta = mouse_x - last_mouse_x;
		y_delta = mouse_y - last_mouse_y;

		last_mouse_x = mouse_x;
		last_mouse_y = mouse_y;
	}



	std::array<bool, 'z' - 'a' + 1>& GetButtonState()
	{
		return buttons;
	}




#endif
}
