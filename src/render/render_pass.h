#ifndef RENDER_ENGINE_RENDER_RENDER_PASS_H_
#define RENDER_ENGINE_RENDER_RENDER_PASS_H_


#include "vulkan/vulkan.h"

#include "render/object_base.h"

namespace render
{
	class RenderPass : public RenderObjBase
	{
	public:
		RenderPass(const VkDevice& device, const VkFormat& format);

		RenderPass(const RenderPass&) = delete;
		RenderPass(RenderPass&&) = default;

		RenderPass& operator=(const RenderPass&) = delete;
		RenderPass& operator=(RenderPass&&) = default;

		const VkRenderPass& GetRenderPassHandle() const;

		~RenderPass();
	private:
		VkRenderPass render_pass_;
	};
}
#endif  // RENDER_ENGINE_RENDER_RENDER_PASS_H_