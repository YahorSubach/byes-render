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
		
		VkFormat GetFormat() const;
		uint32_t GetLayerCount() const;

		virtual ~ImageView() override;

		bool deferred_delete_ = true;

	protected:
		
		VkFormat format_;
		uint32_t layer_cnt_;

	};
}
#endif  // RENDER_ENGINE_RENDER_IMAGE_VIEW_H_