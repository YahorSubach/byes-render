#include <iostream>

#include "render/render_engine.h"

//int CALLBACK WinMain(
//	_In_ HINSTANCE hInstance,
//	_In_opt_ HINSTANCE hPrevInstance,
//	_In_ LPSTR     lpCmdLine,
//	_In_ int       nCmdShow
//)
int main()
{
	{
		render::RenderEngine facade(nullptr);
		std::cout << "Vulkan initialization on facade creation success: " << facade.VKInitSuccess() << std::endl;

		if (facade.VKInitSuccess())
		{
			facade.ShowWindow();
		}
	}
	std::cout << "Destroid" << std::endl;
	return 0;
}