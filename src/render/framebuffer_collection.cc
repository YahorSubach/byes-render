//#include "common.h"
//#include "framebuffer_collection.h"
//
//render::FramebufferCollection::FramebufferCollection(const Global& global, const RenderSetup& render_setup) : RenderObjBase(global),
//
//images_{
//	Image(global_, global_.g_buffer_format, global_.presentation_extent, {ImageProperty::kColorAttachment, ImageProperty::kShaderInput, ImageProperty::kMipMap}),
//	Image(global_, global_.g_buffer_format, global_.presentation_extent, {ImageProperty::kColorAttachment, ImageProperty::kShaderInput}),
//	Image(global_, global_.g_buffer_format, global_.presentation_extent, {ImageProperty::kColorAttachment, ImageProperty::kShaderInput}),
//	Image(global_, global_.g_buffer_format, global_.presentation_extent, {ImageProperty::kColorAttachment, ImageProperty::kShaderInput}),
//	Image(global_, global_.depth_map_format, global_.presentation_extent, {ImageProperty::kDepthAttachment}),
//
//	Image(global_, global_.depth_map_format, global_.depth_map_extent, {ImageProperty::kDepthAttachment, ImageProperty::kShaderInput}),
//},
//
//image_views_{
//	ImageView(global_, images_[static_cast<int>(AttachmentId::kGAlbedo)]),
//	ImageView(global_, images_[static_cast<int>(AttachmentId::kGPosition)]),
//	ImageView(global_, images_[static_cast<int>(AttachmentId::kGNormal)]),
//	ImageView(global_, images_[static_cast<int>(AttachmentId::kGMetallicRoughness)]),
//
//	ImageView(global_, images_[static_cast<int>(AttachmentId::kGDepth)]),
//	ImageView(global_, images_[static_cast<int>(AttachmentId::kDepthMap)]),
//},
//
//
//framebuffers_{
//	Framebuffer(global, global_.presentation_extent, 
//		{ image_views_[u32(AttachmentId::kGAlbedo)], image_views_[u32(AttachmentId::kGPosition)], image_views_[u32(AttachmentId::kGNormal)], image_views_[u32(AttachmentId::kGMetallicRoughness)], image_views_[u32(AttachmentId::kGDepth)] },
//		render_setup.GetRenderPass(RenderPassId::kBuildGBuffers)),
//	Framebuffer(global, global_.depth_map_extent, { image_views_[u32(AttachmentId::kDepthMap)] }, render_setup.GetRenderPass(RenderPassId::kBuildDepthmap)),
//}
//
//{}
//
//const render::Image& render::FramebufferCollection::GetImage(AttachmentId id) const
//{
//	return images_[u32(id)];
//}
//
//const render::Framebuffer& render::FramebufferCollection::GetFramebuffer(FramebufferId id) const
//{
//	return framebuffers_[u32(id)];
//}
