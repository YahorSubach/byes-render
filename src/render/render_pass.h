#ifndef RENDER_ENGINE_RENDER_RENDER_PASS_H_
#define RENDER_ENGINE_RENDER_RENDER_PASS_H_


#include "vulkan/vulkan.h"

#include "render/object_base.h"
#include "render/object_base.h"

namespace render
{
	

	class RenderPass : public RenderObjBase<VkRenderPass>
	{
	public:

		struct RenderPassDesc
		{
			struct Attachment
			{
				std::string name;
				bool is_depth_attachment;
				VkAttachmentDescription desc;
			};

			struct Subpass
			{
				struct AttachmentRef
				{
					std::string name;
					VkImageLayout layout;
				};

				std::string name;
				std::vector<AttachmentRef> attachment_refs;
			};

			struct Dependency
			{
				std::string from_name;
				std::string to_name;

				VkSubpassDependency dependency;
			};

			std::vector<Attachment> attachments;
			std::vector<Subpass> subpasses;
			std::vector<Dependency> dependencies;
		};

		RenderPass(const DeviceConfiguration& device_cfg, RenderPassDesc render_pass_desc);
		static RenderPassDesc BuildRenderPassDesc(RenderPassId type, VkFormat color_format, VkFormat depth_format);

		RenderPass(const RenderPass&) = delete;
		RenderPass(RenderPass&&) = default;

		RenderPass& operator=(const RenderPass&) = delete;
		RenderPass& operator=(RenderPass&&) = default;

		int attachments_cnt = 0;

		virtual ~RenderPass() override;

	private:



	};

	class RenderPass2 : public RenderObjBase<VkRenderPass>
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

	};

}
#endif  // RENDER_ENGINE_RENDER_RENDER_PASS_H_