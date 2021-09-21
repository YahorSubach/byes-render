#ifndef VK_VISUAL_FACADE_SURFACE_H_
#define VK_VISUAL_FACADE_SURFACE_H_

#include "platform.h"
#include "vulkan/vulkan.h"


namespace vkvf
{
	class Surface
	{
	public:
		Surface(InitParam init_param, const VkInstance& instance, const VkPhysicalDevice& physical_device, uint32_t queue_index, const VkDevice& logical_device);

		Surface(const Surface&) = delete;
		Surface(Surface&&) = delete;

		Surface& operator=(const Surface&) = delete;
		Surface& operator=(Surface&&) = delete;

		const VkImage* AcquireImage();
		bool Valid();

		platform::Window GetWindow();

		VkFormat swapchain_image_format;
		VkExtent2D swapchain_extent;

		std::vector<VkImageView> images_views_;

		~Surface();
	private:

		VkSurfaceFormatKHR GetSurfaceFormat(const VkPhysicalDevice& physical_device);
		VkPresentModeKHR GetSurfacePresentMode(const VkPhysicalDevice& physical_device);
		VkExtent2D GetSwapExtend(const VkSurfaceCapabilitiesKHR& capabilities);

		platform::Window window_hande_;
		VkSurfaceKHR surface_;
		VkSwapchainKHR swapchain_;
		std::vector<VkImage> images_;



		const VkDevice& logical_device_;

		bool is_valid_;
	};
}
#endif  // VK_VISUAL_FACADE_SURFACE_H_