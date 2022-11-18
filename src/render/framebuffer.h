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

	class Framebuffer : public LazyRenderObj<VkFramebuffer>
	{
	public:
		Framebuffer(const DeviceConfiguration& device_cfg, Extent extent, const RenderPass& render_pass);

		Framebuffer(const Framebuffer&) = delete;
		Framebuffer(Framebuffer&&) = default;

		Framebuffer& operator=(const Framebuffer&) = delete;
		Framebuffer& operator=(Framebuffer&&) = default;

		int AddAttachment(const std::string_view& name, const ImageView& image_view);

		virtual ~Framebuffer() override;
	
		Extent GetExtent() const;

		const std::vector<std::reference_wrapper<const ImageView>>& GetAttachments() const;

		//void Build(const RenderPass2& render_pass);

		//struct AttachmentDesc
		//{
		//	const ImageView& image_view;
		//	const std::variant<ColorClearValues, DepthStencilClearValues> clear_values;
		//};

		//const std::vector<AttachmentDesc>& GetAttachmentsDescs() const;

	private:

		virtual bool InitHandle() const override;

		Extent extent_;
		const RenderPass& render_pass_;

		std::vector<std::reference_wrapper<const ImageView>> image_views_;
	};
}
#endif  // RENDER_ENGINE_RENDER_FRAMEBUFFER_H_