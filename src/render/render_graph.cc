#include "render_graph.h"

enum class FramebufferId
{
	kGBuffers,
	kDepth,

	kCount,

	kScreen
};

enum class AttachmentId
{
	kGAlbedo,
	kGPosition,
	kGNormal,
	kGMetallicRoughness,
	kGDepth,

	kDepthMap,

	kCount
};

images_{
	Image(device_cfg_, device_cfg_.g_buffer_format, device_cfg_.presentation_extent, {ImageProperty::kColorAttachment, ImageProperty::kShaderInput, ImageProperty::kMipMap}),
	Image(device_cfg_, device_cfg_.g_buffer_format, device_cfg_.presentation_extent, {ImageProperty::kColorAttachment, ImageProperty::kShaderInput}),
	Image(device_cfg_, device_cfg_.g_buffer_format, device_cfg_.presentation_extent, {ImageProperty::kColorAttachment, ImageProperty::kShaderInput}),
	Image(device_cfg_, device_cfg_.g_buffer_format, device_cfg_.presentation_extent, {ImageProperty::kColorAttachment, ImageProperty::kShaderInput}),
	Image(device_cfg_, device_cfg_.depth_map_format, device_cfg_.presentation_extent, {ImageProperty::kDepthAttachment}),

	Image(device_cfg_, device_cfg_.depth_map_format, device_cfg_.depth_map_extent, {ImageProperty::kDepthAttachment, ImageProperty::kShaderInput}),
},

image_views_{
	ImageView(device_cfg_, images_[static_cast<int>(AttachmentId::kGAlbedo)]),
	ImageView(device_cfg_, images_[static_cast<int>(AttachmentId::kGPosition)]),
	ImageView(device_cfg_, images_[static_cast<int>(AttachmentId::kGNormal)]),
	ImageView(device_cfg_, images_[static_cast<int>(AttachmentId::kGMetallicRoughness)]),

	ImageView(device_cfg_, images_[static_cast<int>(AttachmentId::kGDepth)]),
	ImageView(device_cfg_, images_[static_cast<int>(AttachmentId::kDepthMap)]),
},


render::RenderGraph::RenderGraph(const DeviceConfiguration& device_cfg, const Image& presentation_image): RenderObjBase(device_cfg)
{
	auto&& [g_albedo_image, g_albedo_image_view] = image_collection_.Add(device_cfg_, device_cfg_.g_buffer_format, device_cfg_.presentation_extent);
	auto&& [g_position_image, g_position_image_view] = image_collection_.Add(device_cfg_, device_cfg_.g_buffer_format, device_cfg_.presentation_extent);
	auto&& [g_normal_image, g_normal_image_view] = image_collection_.Add(device_cfg_, device_cfg_.g_buffer_format, device_cfg_.presentation_extent);
	auto&& [g_metallic_roughness_image, g_metallic_roughness_image_view] = image_collection_.Add(device_cfg_, device_cfg_.g_buffer_format, device_cfg_.presentation_extent);

	auto&& [g_depth_image, g_depth_roughness_image_view] = image_collection_.Add(device_cfg_, device_cfg_.depth_map_format, device_cfg_.depth_map_extent);

}

render::RenderGraph::~RenderGraph()
{
}

std::pair<render::Image&, render::ImageView&> render::RenderGraph::ImageCollection::Add(const DeviceConfiguration& device_cfg, VkFormat format, Extent extent)
{
	images.push_back(Image(device_cfg, format, extent));
	image_views.push_back(ImageView(device_cfg, images.back()));
	return { images.back(), image_views.back() };
}
