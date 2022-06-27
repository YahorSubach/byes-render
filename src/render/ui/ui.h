#ifndef RENDER_ENGINE_RENDER_UI_UI_H_
#define RENDER_ENGINE_RENDER_UI_UI_H_

#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H  

#include "render/image.h"

#include "common.h"
#include "render/object_base.h"
#include "render/sampler.h"


namespace render::ui
{
	class UI: public RenderObjBase<void*>
	{
	public:
		UI(DeviceConfiguration& device_cfg, Extent extent);


		const std::vector<BufferAccessor>& GetVertexBuffers() const;
		const BufferAccessor& GetIndexBuffer() const;

		const Image& GetTestImage() const;
		const Sampler& GetUISampler() const;

		Extent GetExtent() const;

	private:

		std::map<char32_t, Image> char_images_;

		GPULocalBuffer polygon_vert_pos_;
		GPULocalBuffer polygon_vert_tex_;

		GPULocalBuffer polygon_vert_ind_;

		std::vector<BufferAccessor> vertex_buffers_;
		BufferAccessor index_buffer_;

		Image test_image_;
		Sampler ui_sampler_;

		Extent extent_;

	};
}

#endif  // RENDER_ENGINE_RENDER_UI_UI_H_