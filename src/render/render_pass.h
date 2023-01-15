#ifndef RENDER_ENGINE_RENDER_RENDER_PASS_H_
#define RENDER_ENGINE_RENDER_RENDER_PASS_H_


#include "vulkan/vulkan.h"

#include "render/object_base.h"
#include "render/image_view.h"

namespace render
{
	const std::string kSwapchainAttachmentName = "swapchain";

	class RenderNode;

	class RenderPass : public RenderObjBase<VkRenderPass>
	{
	public:

		//enum class SwapchainInteraction
		//{
		//	kNone,
		//	kAcquire,
		//	kPresent
		//};

		//using SwapchainInteractionFlags = util::EnumFlags<SwapchainInteraction>;

		struct Attachment
		{
			const std::string name;
			bool is_depth_attachment;
			VkAttachmentDescription desc;
		};


		RenderPass(const Global& global, const RenderNode& render_node/*, SwapchainInteractionFlags interaction = {}*/);

		//int AddColorAttachment(const std::string_view& name, bool high_range = true);
		//int AddDepthAttachment(const std::string_view& name);
		//int GetAttachmentIndex(const std::string_view& name) const;
		//int GetAttachmentsCnt() const;
		//int GetColorAttachmentsCnt() const;
		//const Attachment& GetAttachmentByIndex(int index) const;

		//static RenderPassDesc BuildRenderPassDesc(RenderPassId type, VkFormat color_format, VkFormat depth_format);

		RenderPass(const RenderPass&) = delete;
		RenderPass(RenderPass&&) = default;

		RenderPass& operator=(const RenderPass&) = delete;
		RenderPass& operator=(RenderPass&&) = default;


		virtual ~RenderPass() override;

	private:

		bool contains_depth_attachment_;


		//std::vector<Attachment> attachments_;
		//std::vector<Attachment> depth_attachments_;
		//std::optional<Attachment> depth_attachment_;

		//virtual bool InitHandle() const;
		//SwapchainInteractionFlags swapchain_interaction_flags;
	};

}
#endif  // RENDER_ENGINE_RENDER_RENDER_PASS_H_