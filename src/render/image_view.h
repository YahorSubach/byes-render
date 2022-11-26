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
	class ImageView : public LazyRenderObj<VkImageView>
	{
	public:

		ImageView(const DeviceConfiguration& device_cfg);
		ImageView(const DeviceConfiguration& device_cfg, const Image& image);

		ImageView(const ImageView&) = delete;
		ImageView(ImageView&&) = default;

		ImageView& operator=(const ImageView&) = delete;
		ImageView& operator=(ImageView&&) = default;

		void Assign(const Image& image);
		
		uint32_t AddUsageFlag(uint32_t flag) const;
		uint32_t CheckUsageFlag(uint32_t flag) const;

		VkFormat GetFormat() const;

		virtual ~ImageView() override;

		//stl_util::NullableRef<const Image> GetImage() const;

	protected:
		
		virtual bool InitHandle() const override;

		//stl_util::NullableRef<const Image> image_;
	};
}
#endif  // RENDER_ENGINE_RENDER_IMAGE_VIEW_H_