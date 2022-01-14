#ifndef RENDER_ENGINE_RENDER_SWAPCHAIN_H_
#define RENDER_ENGINE_RENDER_SWAPCHAIN_H_


#include "vulkan/vulkan.h"

#include "render/object_base.h"

namespace render
{
	class Swapchain : public RenderObjBase
	{
	public:
		Swapchain(const VkDevice& device, const VkFormat& format);

		Swapchain(const Swapchain&) = delete;
		Swapchain(Swapchain&&) = default;

		Swapchain& operator=(const Swapchain&) = delete;
		Swapchain& operator=(Swapchain&&) = default;

		const VkSwapchainKHR& GetSwapchainHandle() const;

		~Swapchain();
	private:
		VkSwapchainKHR swapchain_;
	};
}
#endif  // RENDER_ENGINE_RENDER_SWAPCHAIN_H_