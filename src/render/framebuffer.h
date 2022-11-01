#ifndef RENDER_ENGINE_RENDER_FRAMEBUFFER_H_
#define RENDER_ENGINE_RENDER_FRAMEBUFFER_H_


#include "vulkan/vulkan.h"

#include <variant>
#include <vector>
#include <functional>

#include "render/object_base.h"
#include "render/render_pass.h"
#include "render/image_view.h"

namespace render
{
	struct ColorClearValues
	{
		float r;
		float g;
		float b;
		float a;
	};

	struct DepthStencilClearValues
	{
		float depth;
		uint32_t stencil;
	};

	class Framebuffer : public RenderObjBase<VkFramebuffer>
	{
	public:
		Framebuffer(const DeviceConfiguration& device_cfg, const Extent& extent, const std::vector<std::reference_wrapper<const ImageView>>& attachments);

		Framebuffer(const Framebuffer&) = delete;
		Framebuffer(Framebuffer&&) = default;

		Framebuffer& operator=(const Framebuffer&) = delete;
		Framebuffer& operator=(Framebuffer&&) = default;

		virtual ~Framebuffer() override;
	
		Extent GetExtent() const;

		void Build(const RenderPass2& render_pass);

		struct AttachmentDesc
		{
			const ImageView& image_view;
			const std::variant<ColorClearValues, DepthStencilClearValues> clear_values;
		};

		const std::vector<AttachmentDesc>& GetAttachmentsDescs() const;

	private:

		std::vector<AttachmentDesc> attachments_descs_;
		Extent extent_;
	};
}
#endif  // RENDER_ENGINE_RENDER_FRAMEBUFFER_H_