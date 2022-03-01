#ifndef RENDER_ENGINE_RENDER_FRAMEBUFFER_H_
#define RENDER_ENGINE_RENDER_FRAMEBUFFER_H_


#include "vulkan/vulkan.h"

#include "render/object_base.h"
#include "render/render_pass.h"
#include "render/image_view.h"

namespace render
{
	class Framebuffer : public RenderObjBase<VkFramebuffer>
	{
	public:
		Framebuffer(const DeviceConfiguration& device_cfg, const Extent& extent, const ImageView& color_image_view, const ImageView& depth_image_view, const RenderPass& render_pass);

		Framebuffer(const Framebuffer&) = delete;
		Framebuffer(Framebuffer&&) = default;

		Framebuffer& operator=(const Framebuffer&) = delete;
		Framebuffer& operator=(Framebuffer&&) = default;

		virtual ~Framebuffer() override;
	
		Extent GetExtent() const;

	private:

		Extent extent_;
	};
}
#endif  // RENDER_ENGINE_RENDER_FRAMEBUFFER_H_