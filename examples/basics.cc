#include <iostream>

#include "vkvf.h"

void main()
{
	{
		vkvf::VKVisualFacade facade;
		std::cout << "Vulkan initialization on facade creation success: " << facade.VKInitSuccess() << std::endl;
	}
	std::cout << "Destroid" << std::endl;
}