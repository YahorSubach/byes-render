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

		ImageView(const Global& global);
		ImageView(const Global& global, const Image& image);

		ImageView(const ImageView&) = delete;
		ImageView(ImageView&&) = default;

		ImageView& operator=(const ImageView&) = delete;
		ImageView& operator=(ImageView&&) = default;

		void Assign(const Image& image);
		
		//uint32_t AddUsageFlag(uint32_t flag) const;
		//uint32_t CheckUsageFlag(uint32_t flag) const;

		VkFormat GetFormat() const;

		virtual ~ImageView() override;

		//util::NullableRef<const Image> GetImage() const;

		bool deferred_delete_ = true;

	protected:
		
		VkFormat format_;

		//virtual bool InitHandle() const override;

		//util::NullableRef<const Image> image_;
	};
}
#endif  // RENDER_ENGINE_RENDER_IMAGE_VIEW_H_