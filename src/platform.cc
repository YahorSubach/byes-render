#include "platform.h"

#include "windowsx.h"

#include<atomic>
#include<iostream>

namespace render::platform
{
#ifdef WIN32
	using Window = HWND;

	bool consume_mouse_input = false;

	InputState input_state = {};

	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (true &&/*uMsg != WM_MOUSEMOVE &&*/ uMsg != WM_SETCURSOR && uMsg != WM_NCHITTEST  && uMsg != WM_GETICON && 
			uMsg != WM_NCMOUSELEAVE && uMsg != WM_NCMOUSEHOVER && 
			!(uMsg >= WM_NCMOUSEMOVE && uMsg <= WM_NCMBUTTONDBLCLK) &&
			!(uMsg >= WM_NCCREATE && uMsg <= WM_SYNCPAINT) &&
			uMsg != WM_SYSCOMMAND && uMsg != WM_GETMINMAXINFO && uMsg != WM_ERASEBKGND && uMsg != WM_PAINT
			)
		{
			if (uMsg == WM_SETFOCUS)
				std::cout << "WM_SETFOCUS" << std::endl;
			else if(uMsg == WM_KILLFOCUS)
				std::cout << "WM_KILLFOCUS" << std::endl;
			else if (uMsg == WM_ACTIVATE)
				std::cout << "WM_ACTIVATE" << std::endl;
			else if (uMsg == WM_WINDOWPOSCHANGING)
				std::cout << "WM_WINDOWPOSCHANGING" << std::endl;
			else if (uMsg == WM_WINDOWPOSCHANGED)
				std::cout << "WM_WINDOWPOSCHANGED" << std::endl;
			else if (uMsg == WM_NCHITTEST)
				std::cout << "WM_NCHITTEST" << std::endl;
			else if (uMsg == WM_SETCURSOR)
				std::cout << "WM_SETCURSOR" << std::endl;
			else if (uMsg == WM_MOVE)
				std::cout << "WM_MOVE" << std::endl;
			else if (uMsg == WM_SIZE)
				std::cout << "WM_SIZE" << std::endl;
			else if (uMsg == WM_SIZING)
				std::cout << "WM_SIZING" << std::endl;
			else if (uMsg == WM_ENTERSIZEMOVE)
				std::cout << "WM_ENTERSIZEMOVE" << std::endl;
			else if (uMsg == WM_EXITSIZEMOVE)
				std::cout << "WM_EXITSIZEMOVE" << std::endl;
			else if (uMsg == WM_IME_SETCONTEXT)
				std::cout << "WM_IME_SETCONTEXT" << std::endl;
			else if (uMsg == WM_IME_NOTIFY)
				std::cout << "WM_IME_NOTIFY" << std::endl;
			else if (uMsg == WM_MOUSEMOVE)
			{}//std::cout << "WM_MOUSEMOVE" << std::endl;
			else std::cout << uMsg << std::endl;
			
		}

		switch (uMsg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_SETFOCUS:
		{
			//SetCapture(hwnd);
			ShowCursor(false);

			consume_mouse_input = true;

			RECT rect;
			GetWindowRect(hwnd, &rect);
			ClipCursor(&rect);
			//std::cout << rect.left << " " << rect.top << " " << rect.right << " " << rect.bottom << std::endl;
			break;
		}

		case WM_KILLFOCUS:
		{
			consume_mouse_input = false;

			for (auto&& button : input_state.button_states)
			{
				button = 0;
			}

			break;
		}

		case WM_WINDOWPOSCHANGED:
		case WM_SIZE:
		{
			RECT rect;
			GetWindowRect(hwnd, &rect);
			ClipCursor(&rect);
			//std::cout << rect.left << " " << rect.top << " " << rect.right << " " << rect.bottom << std::endl;


			break;
		}

		case WM_KEYDOWN:
		{
			if (wParam < 0xFF)
			{
				input_state.button_states[wParam] = 1;
				break;
			}

			switch (wParam)
			{
			case VK_ESCAPE:
				consume_mouse_input = !consume_mouse_input;
				ShowCursor(!consume_mouse_input);

				if (consume_mouse_input)
				{
					RECT rect;
					GetWindowRect(hwnd, &rect);
					ClipCursor(&rect);
				}
				else
				{
					ClipCursor(nullptr);
				}
				// Process the LEFT ARROW key. 

				break;
			}

			break;
		}
		
		case WM_KEYUP:
		{
			if (wParam < 0xFF)
			{
				input_state.button_states[wParam] = 0;
				break;
			}

			break;
		}

		case WM_MOUSEMOVE:
		{
			RECT cursor_rect;

			GetClipCursor(&cursor_rect);
			//std::cout << cursor_rect.left << " " << cursor_rect.top << " " << cursor_rect.right << " " << cursor_rect.bottom << std::endl;
			RECT rect;
			GetWindowRect(hwnd, &rect);
			//std::cout << rect.left << " " << rect.top << " " << rect.right << " " << rect.bottom << std::endl;
			

			int rect_x_center = rect.left + (rect.right - rect.left) / 2;
			int rect_y_center = rect.top + (rect.bottom - rect.top) / 2;

			if (consume_mouse_input)
			{

				POINT point;
				GetCursorPos(&point);

				int x = point.x;
				int y = point.y;

				if (x == rect_x_center && y == rect_y_center)
					break;

				input_state.mouse_position.first += (x - rect_x_center);
				input_state.mouse_position.second+= (y - rect_y_center);

				SetCursorPos(rect_x_center, rect_y_center);
			}

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
				CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800,

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


	const std::vector<const char*>& GetRequiredInstanceExtensions()
	{
		static const std::vector<const char*> extensions{ "VK_KHR_surface", "VK_KHR_win32_surface", "VK_KHR_get_physical_device_properties2"};
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

	const InputState& GetInputState()
	{
		//input_state.mouse_delta = 
		//{	input_state.mouse_position.first - input_state.mouse_position_prev.first, 
		//	input_state.mouse_position.second - input_state.mouse_position_prev.second };

		//input_state.mouse_position_prev = input_state.mouse_position;

		return input_state;
	}

#endif
}
