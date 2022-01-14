#include "swapchain.h"

#include "common.h"

render::Swapchain::Swapchain(const VkDevice& device, const VkFormat& format): RenderObjBase(device)/*, render_pass_(VK_NULL_HANDLE)*/
{

}


render::Swapchain::~Swapchain()
{
	if (swapchain_ != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(device_, swapchain_, nullptr);
	}
}

const VkSwapchainKHR& render::Swapchain::GetSwapchainHandle() const
{
	return swapchain_;
}
