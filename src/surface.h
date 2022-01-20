#ifndef RENDER_ENGINE_RENDER_SURFACE_H_
#define RENDER_ENGINE_RENDER_SURFACE_H_


#include "vulkan/vulkan.h"

#include "platform.h"
#include "render/object_base.h"
#include "render/data_types.h"

namespace render
{
	class Surface: public RenderObjBase<VkSwapchainKHR>
	{
	public:
		Surface(platform::Window window_handle, const VkInstance& instance, const DeviceConfiguration& device_cfg);

		Surface(const Surface&) = delete;
		Surface(Surface&&) = default;

		Surface& operator=(const Surface&) = delete;
		Surface& operator=(Surface&&) = default;

		const VkImage* AcquireImage();

		const std::vector<VkImageView>& GetImageViews() const;

		platform::Window GetWindow();

		bool RefreshSwapchain();

		const VkFormat& GetSwapchainFormat();
		const VkExtent2D& GetSwapchainExtent();
		const VkSwapchainKHR& GetSwapchain();

		virtual ~Surface() override;
	private:

		VkSurfaceFormatKHR GetSurfaceFormat(const VkPhysicalDevice& physical_device);
		VkPresentModeKHR GetSurfacePresentMode(const VkPhysicalDevice& physical_device);
		VkExtent2D GetSwapExtend(const VkSurfaceCapabilitiesKHR& capabilities);

		platform::Window window_hande_;

		VkFormat swapchain_image_format_;
		VkExtent2D swapchain_extent_;

		VkSurfaceKHR surface_;
		std::vector<VkImage> images_;
		std::vector<VkImageView> images_views_;

		const VkPhysicalDevice& physical_device_;

		uint32_t queue_index_;
	};
}
#endif  // RENDER_ENGINE_RENDER_SURFACE_H_