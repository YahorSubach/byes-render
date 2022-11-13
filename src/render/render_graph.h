#ifndef RENDER_ENGINE_RENDER_RENDER_GRAPH_H_
#define RENDER_ENGINE_RENDER_RENDER_GRAPH_H_


#include "vulkan/vulkan.h"

#include <vector>

#include "render/object_base.h"
#include "render/image.h"

namespace render
{
	class RenderGraph : public RenderObjBase<int>
	{
	public:
		RenderGraph(const DeviceConfiguration& device_cfg, const Extent& extent, const Image& presentation_image);

		RenderGraph(const RenderGraph&) = delete;
		RenderGraph(RenderGraph&&) = default;

		RenderGraph& operator=(const RenderGraph&) = delete;
		RenderGraph& operator=(RenderGraph&&) = default;

		virtual ~RenderGraph() override;
	
	private:

		std::vector<Image> images_;
		std::vector<Image> image_views_;

	};
}
#endif  // RENDER_ENGINE_RENDER_RENDER_GRAPH_H_