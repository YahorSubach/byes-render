#ifndef RENDER_ENGINE_RENDER_FRAMEBUFFER_H_
#define RENDER_ENGINE_RENDER_FRAMEBUFFER_H_


#include "vulkan/vulkan.h"

#include "render/object_base.h"
#include "render/render_pass.h"

namespace render
{
	class Framebuffer : public RenderObjBase<VkFramebuffer>
	{
	public:
		Framebuffer(const VkDevice& device, const VkExtent2D& extent, const VkImageView& image_view, const RenderPass& render_pass);

		Framebuffer(const Framebuffer&) = delete;
		Framebuffer(Framebuffer&&) = default;

		Framebuffer& operator=(const Framebuffer&) = delete;
		Framebuffer& operator=(Framebuffer&&) = default;

		const VkFramebuffer& GetFramebufferHandle() const;

		virtual ~Framebuffer() override;
	};
}
#endif  // RENDER_ENGINE_RENDER_FRAMEBUFFER_H_