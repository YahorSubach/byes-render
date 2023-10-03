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
	struct Glyph
	{
		int advance;

		int bitmap_x;
		int bitmap_y;

		int bitmap_width;
		int bitmap_heigth;

		glm::vec2 atlas_position;
		glm::vec2 atlas_width_height;
		
		util::NullableRef<const Image> bitmap;
	};

	class UI: public RenderObjBase<void*>
	{
	public:
		UI(const Global& global);


		const std::vector<BufferAccessor>& GetVertexBuffers() const;
		const BufferAccessor& GetIndexBuffer() const;

		const Glyph& GetGlyph(char32_t character, int font_size) const;

		const Sampler& GetUISampler() const;
		const Image& GetAtlas() const;


		Extent GetExtent() const;
		
		Image test_image_;

	private:

		struct FontData
		{
			std::map<char32_t, Glyph> glyphs;
			Image atlas;
		};

		mutable std::map<int, FontData> size_to_font_data_;

		GPULocalBuffer polygon_vert_pos_;
		GPULocalBuffer polygon_vert_tex_;

		GPULocalBuffer polygon_vert_ind_;

		std::vector<BufferAccessor> vertex_buffers_;
		BufferAccessor index_buffer_;


		Sampler ui_sampler_;

		Extent extent_;


		FT_Library ft;

		FT_Face face;

		//TODO add release for ft

	};
}

#endif  // RENDER_ENGINE_RENDER_UI_UI_H_