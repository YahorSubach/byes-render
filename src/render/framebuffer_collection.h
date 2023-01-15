//#ifndef RENDER_ENGINE_RENDER_FRAMEBUFFER_COLLECTION_H_
//#define RENDER_ENGINE_RENDER_FRAMEBUFFER_COLLECTION_H_
//
//#include <array>
//
//#include "render/object_base.h"
//#include "render/framebuffer.h"
//#include "render/render_setup.h"
//
//
//namespace render
//{
//	enum class FramebufferId
//	{
//		kGBuffers,
//		kDepth,
//
//		kCount,
//
//		kScreen
//	};
//
//	enum class AttachmentId
//	{
//		kGAlbedo,
//		kGPosition,
//		kGNormal,
//		kGMetallicRoughness,
//		kGDepth,
//
//		kDepthMap,
//
//		kCount
//	};
//
//	constexpr
//		uint32_t kFramebufferIdCount = static_cast<uint32_t>(FramebufferId::kCount);
//
//	constexpr
//		uint32_t kAttachmentIdCount = static_cast<uint32_t>(AttachmentId::kCount);
//
//	class FramebufferCollection : public RenderObjBase<void*>
//	{
//	public:
//
//		FramebufferCollection(const Global& global, const RenderSetup& render_setup);
//
//		FramebufferCollection(const FramebufferCollection&) = delete;
//		FramebufferCollection(FramebufferCollection&&) = default;
//
//		FramebufferCollection& operator=(const FramebufferCollection&) = delete;
//		FramebufferCollection& operator=(FramebufferCollection&&) = default;
//
//		virtual ~FramebufferCollection() = default;
//
//		const Image& GetImage(AttachmentId id) const;
//		const Framebuffer& GetFramebuffer(FramebufferId id) const;
//
//	private:
//
//		std::array<Image, kAttachmentIdCount> images_;
//		std::array<ImageView, kAttachmentIdCount> image_views_;
//		std::array<Framebuffer, kFramebufferIdCount> framebuffers_;
//
//	};
//}
//#endif  // RENDER_ENGINE_RENDER_FRAMEBUFFER_H_