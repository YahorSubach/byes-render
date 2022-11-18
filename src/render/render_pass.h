#ifndef RENDER_ENGINE_RENDER_RENDER_PASS_H_
#define RENDER_ENGINE_RENDER_RENDER_PASS_H_


#include "vulkan/vulkan.h"

#include "render/object_base.h"
#include "render/image_view.h"

namespace render
{
	

	class RenderPass : public LazyRenderObj<VkRenderPass>
	{
	public:

		//struct RenderPassDesc
		//{
		//	struct Attachment
		//	{
		//		std::string name;
		//		bool is_depth_attachment;
		//		VkAttachmentDescription desc;
		//	};

		//	struct Subpass
		//	{
		//		struct AttachmentRef
		//		{
		//			std::string name;
		//			VkImageLayout layout;
		//		};

		//		std::string name;
		//		std::vector<AttachmentRef> attachment_refs;
		//	};

		//	struct Dependency
		//	{
		//		std::string from_name;
		//		std::string to_name;

		//		VkSubpassDependency dependency;
		//	};

		//	std::vector<Attachment> attachments;
		//	std::vector<Subpass> subpasses;
		//	std::vector<Dependency> dependencies;
		//};

		struct Attachment
		{
			const std::string name;
			bool is_depth_attachment;
			VkAttachmentDescription desc;
		};


		RenderPass(const DeviceConfiguration& device_cfg, bool use_swapchain_image = false);

		int AddColorAttachment(const std::string_view& name);
		int AddDepthAttachment(const std::string_view& name);
		int GetAttachmentIndex(const std::string_view& name) const;
		int GetAttachmentsCnt() const;
		const Attachment& GetAttachmentByIndex(int index) const;

		//static RenderPassDesc BuildRenderPassDesc(RenderPassId type, VkFormat color_format, VkFormat depth_format);

		RenderPass(const RenderPass&) = delete;
		RenderPass(RenderPass&&) = default;

		RenderPass& operator=(const RenderPass&) = delete;
		RenderPass& operator=(RenderPass&&) = default;


		virtual ~RenderPass() override;

	private:

		std::vector<Attachment> attachments_;
		std::vector<Attachment> depth_attachments_;
		std::optional<Attachment> depth_attachment_;

		virtual bool InitHandle() const;
		bool use_swapchain_image_;
	};

	/*class RenderPass2 : public RenderObjBase<VkRenderPass>
	{
	public:

		RenderPass2(const DeviceConfiguration& device_cfg);

		RenderPass2(const RenderPass2&) = delete;
		RenderPass2(RenderPass2&&) = default;

		RenderPass2& operator=(const RenderPass2&) = delete;
		RenderPass2& operator=(RenderPass2&&) = default;


		void Attach();


		virtual ~RenderPass2() override;

	private:

		struct VkConstructParams
		{};

		std::unique_ptr<VkConstructParams> params_;

	};*/

}
#endif  // RENDER_ENGINE_RENDER_RENDER_PASS_H_