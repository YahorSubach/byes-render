#ifndef RENDER_ENGINE_RENDER_SURFACE_H_
#define RENDER_ENGINE_RENDER_SURFACE_H_


#include "vulkan/vulkan.h"

#include "platform.h"
#include "render/object_base.h"
#include "render/data_types.h"

namespace render
{
	class Surface: public RenderObjBase<VkSurfaceKHR>
	{
	public:
		Surface(platform::Window window_handle, const VkInstance& instance, const Global& global);

		Surface(const Surface&) = delete;
		Surface(Surface&&) = default;

		Surface& operator=(const Surface&) = delete;
		Surface& operator=(Surface&&) = default;

		platform::Window GetWindow();

		VkSurfaceFormatKHR GetSurfaceFormat(const VkPhysicalDevice& physical_device) const;
		VkPresentModeKHR GetSurfacePresentMode(const VkPhysicalDevice& physical_device) const;
		Extent GetSwapExtend(const VkSurfaceCapabilitiesKHR& capabilities) const;

		virtual ~Surface() override;
	private:

		platform::Window window_hande_;
	};
}
#endif  // RENDER_ENGINE_RENDER_SURFACE_H_