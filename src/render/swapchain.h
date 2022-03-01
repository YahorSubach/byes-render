#ifndef RENDER_ENGINE_RENDER_SWAPCHAIN_H_
#define RENDER_ENGINE_RENDER_SWAPCHAIN_H_


#include "vulkan/vulkan.h"

#include "render/data_types.h"
#include "render/object_base.h"
#include <render/image.h>
#include <render/image_view.h>

#include "surface.h"

namespace render
{
	class Swapchain : public RenderObjBase<VkSwapchainKHR>
	{
	public:
		Swapchain(const DeviceConfiguration& device, const Surface& surface);

		Swapchain(const Swapchain&) = delete;
		Swapchain(Swapchain&&) = default;

		Swapchain& operator=(const Swapchain&) = delete;
		Swapchain& operator=(Swapchain&&) = default;

		Extent GetExtent() const;
		VkFormat GetFormat() const;

		size_t GetImagesCount() const;
		const ImageView& GetImageView(size_t index) const;

		virtual ~Swapchain() override;
	private:

		Extent extent_;
		VkFormat format_;
		
		std::vector<Image> images_;
		std::vector<ImageView> image_views_;
	};
}
#endif  // RENDER_ENGINE_RENDER_SWAPCHAIN_H_