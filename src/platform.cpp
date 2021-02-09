#include "platform.h"

namespace vkvf::platform
{
#ifdef WIN32
	using Window = HWND;


	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		//switch (uMsg)
		//{
		//case WM_DESTROY:
		//	PostQuitMessage(0);
		//	return 0;
		//case WM_PAINT:
		//{
		//	//PAINTSTRUCT ps;
		//	//HDC hdc = BeginPaint(hwnd, &ps);



		//	//FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

		//	//EndPaint(hwnd, &ps);
		//}
		//return 0;
		//}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}


	Window CreatePlatformWindow(InitParam param)
	{
		const wchar_t CLASS_NAME[] = L"VKVF WINDOW CLASS";

		WNDCLASSW wc = { };

		wc.lpfnWndProc = WindowProc;
		wc.hInstance = param;
		wc.lpszClassName = CLASS_NAME;


		RegisterClassW(&wc);

		return CreateWindowExW(
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
	}

	void DestroyPlatformWindow(Window window)
	{
		DestroyWindow(window);
	}

	void ShowWindow(Window window)
	{
		if (window)
		{
			ShowWindow(window, 1);
		}

		MSG msg = { };

		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}


	const std::vector<const char*>& GetRequiredExtensionsNames()
	{
		static const std::vector<const char*> extensions{ "VK_KHR_surface", "VK_KHR_win32_surface"};
		return extensions;
	}

#endif
}
