#ifndef RENDER_ENGINE_RENDER_IMAGE_VIEW_H_
#define RENDER_ENGINE_RENDER_IMAGE_VIEW_H_

#include <vector>
#include <array>

#include "vulkan/vulkan.h"

#include "common.h"
#include "render/object_base.h"
#include "render/command_pool.h"
#include "render/image.h"

namespace render
{
	class ImageView : public RenderObjBase<VkImageView>
	{
	public:

		ImageView(const DeviceConfiguration& device_cfg);
		ImageView(const DeviceConfiguration& device_cfg, const Image& image);

		ImageView(const ImageView&) = delete;
		ImageView(ImageView&&) = default;

		ImageView& operator=(const ImageView&) = delete;
		ImageView& operator=(ImageView&&) = default;

		void Assign(const Image& image);
		const ImagePropertiesStorage& GetImageProperties() const;

		virtual ~ImageView() override;

	protected:
		
		ImagePropertiesStorage image_properties_;
	};
}
#endif  // RENDER_ENGINE_RENDER_IMAGE_VIEW_H_