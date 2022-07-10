#include "common.h"
#include "framebuffer_collection.h"

render::FramebufferCollection::FramebufferCollection(const DeviceConfiguration& device_cfg, const RenderSetup& render_setup) : RenderObjBase(device_cfg),

images_{
	Image(device_cfg_, device_cfg_.g_buffer_format, device_cfg_.presentation_extent, ImageType::kColorAttachmentImage),
	Image(device_cfg_, device_cfg_.g_buffer_format, device_cfg_.presentation_extent, ImageType::kColorAttachmentImage),
	Image(device_cfg_, device_cfg_.g_buffer_format, device_cfg_.presentation_extent, ImageType::kColorAttachmentImage),
	Image(device_cfg_, device_cfg_.depth_map_format, device_cfg_.presentation_extent, ImageType::kGDepthImage),

	Image(device_cfg_, device_cfg_.depth_map_format, device_cfg_.depth_map_extent, ImageType::kDepthMapImage),


},

image_views_{
	ImageView(device_cfg_, images_[static_cast<int>(AttachmentId::kGAlbedo)]),
	ImageView(device_cfg_, images_[static_cast<int>(AttachmentId::kGPosition)]),
	ImageView(device_cfg_, images_[static_cast<int>(AttachmentId::kGNormal)]),

	ImageView(device_cfg_, images_[static_cast<int>(AttachmentId::kGDepth)]),
	ImageView(device_cfg_, images_[static_cast<int>(AttachmentId::kDepthMap)]),
},


framebuffers_{
	Framebuffer(device_cfg, device_cfg_.presentation_extent, 
		{ image_views_[u32(AttachmentId::kGAlbedo)], image_views_[u32(AttachmentId::kGPosition)], image_views_[u32(AttachmentId::kGNormal)], image_views_[u32(AttachmentId::kGDepth)] },
		render_setup.GetRenderPass(RenderPassId::kBuildGBuffers)),
	Framebuffer(device_cfg, device_cfg_.depth_map_extent, { image_views_[u32(AttachmentId::kDepthMap)] }, render_setup.GetRenderPass(RenderPassId::kBuildDepthmap)),
}

{}

const render::Image& render::FramebufferCollection::GetImage(AttachmentId id) const
{
	return images_[u32(id)];
}

const render::Framebuffer& render::FramebufferCollection::GetFramebuffer(FramebufferId id) const
{
	return framebuffers_[u32(id)];
}
