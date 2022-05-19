#ifndef RENDER_ENGINE_RENDER_RENDER_COLLECTION_H_
#define RENDER_ENGINE_RENDER_RENDER_COLLECTION_H_

#include <vector>
#include <map>

#include "vulkan/vulkan.h"

#include "render/batches_manager.h"
#include "render/framebuffer.h"
#include "render/graphics_pipeline.h"
#include "render/render_pass.h"

namespace render
{
	enum class FramebufferId
	{
		kScreen,
		kDepth
	};

	enum class RenderPassId
	{
		kScreen,
		kDepth
	};

	enum class PipelineId
	{
		kColor,
		kDepth
	};

	enum class DescriptionSetLayoutId
	{
		kCameraMat,
		kModelMat,
		kColorTex,
		kMirrorTex,

		Count
	};

	class RenderCollection
	{
	public:
		RenderCollection(const BatchesManager& batches_manager);

	};

}
#endif  // RENDER_ENGINE_RENDER_RENDER_COLLECTION_H_