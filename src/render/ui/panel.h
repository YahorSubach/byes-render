#ifndef RENDER_ENGINE_RENDER_UI_PANEL_H_
#define RENDER_ENGINE_RENDER_UI_PANEL_H_

#include <vector>

#include <glm/glm/glm.hpp>

#include <render/image.h>
#include <render/ui/ui.h>
#include <stl_util.h>



namespace render::ui
{
	struct Panel
	{
		Panel(int x, int y, int width, int height);
		Panel(const Panel& panel);
		
		stl_util::NullableRef<const Panel> parent_;

		int x_ = 0;
		int y_ = 0;
		int width_ = 0;
		int height_ = 0;

		glm::vec2 atlas_position;
		glm::vec2 atlas_width_heigth;

		glm::mat4 local_transform;
		std::vector<Panel> children;

		stl_util::NullableRef<const Image> image_;

		void CollectRender(glm::mat4 parent_transform, std::vector<std::pair<glm::mat4, std::pair<glm::vec2, glm::vec2>>>& to_render);

		void AddChild(const Panel& panel);

	protected:

		void SetParent(const Panel& parent);
	};


	class GlyphPanel: public Panel
	{
	public:
		GlyphPanel(int x, int y, int font_size, render::ui::Glyph glyph);
	
	protected:
		Panel character_panel_;
	};

	class TextBlock : public Panel
	{
	public:
		TextBlock(const UI& ui, int x, int y, int font_size, const std::basic_string<char32_t>& text);

	};

}

#endif  // RENDER_ENGINE_RENDER_UI_PANEL_H_