#ifndef RENDER_ENGINE_RENDER_RENDER_GRAPH_H_
#define RENDER_ENGINE_RENDER_RENDER_GRAPH_H_


#include "vulkan/vulkan.h"

#include <vector>

#include "render/object_base.h"
#include "render/image.h"
#include "render/image_view.h"

namespace render
{
	class RenderGraph : public RenderObjBase<int>
	{
	public:
		RenderGraph(const DeviceConfiguration& device_cfg, const Image& presentation_image);

		RenderGraph(const RenderGraph&) = delete;
		RenderGraph(RenderGraph&&) = default;

		RenderGraph& operator=(const RenderGraph&) = delete;
		RenderGraph& operator=(RenderGraph&&) = default;

		virtual ~RenderGraph() override;
	
		struct ImageCollection
		{
			std::pair<Image&, ImageView&> Add(const DeviceConfiguration& device_cfg, VkFormat format, Extent extent);

			std::vector<Image> images;
			std::vector<ImageView> image_views;
		};


	private:

		ImageCollection image_collection_;
	};
}
#endif  // RENDER_ENGINE_RENDER_RENDER_GRAPH_H_