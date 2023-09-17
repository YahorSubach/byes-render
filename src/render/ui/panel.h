#ifndef RENDER_ENGINE_RENDER_UI_PANEL_H_
#define RENDER_ENGINE_RENDER_UI_PANEL_H_

#include <vector>

#include <glm/glm/glm.hpp>

#include <render/image.h>
#include <render/ui/ui.h>
#include <render/scene.h>
#include <stl_util.h>



namespace render::ui
{
	class Panel
	{
	public:
		Panel(render::Scene& scene, int x, int y, int width, int height);
		//Panel(const Panel& panel);
		
		util::NullableRef<const Panel> parent_;

		//glm::mat4 local_transform;
		//std::vector<std::pair<uint32_t, const Mesh&>> models_;
		std::vector<std::shared_ptr<Panel>> children_;

		//void CollectRender(glm::mat4 parent_transform, std::vector<std::pair<glm::mat4, std::pair<glm::vec2, glm::vec2>>>& to_render);

		void AddChild(const std::shared_ptr<Panel>& panel);
		void AddModel(int x, int y, int width, int height, Mesh& model);

		void SetWidth(float width);
		void SetHeight(float height);

		void ClearModels();

	protected:

		render::Scene& scene_;
		NodeId node_id_;

		std::vector<std::pair<NodeId, RenderModelId>> node_models_ids_;

		int x_ = 0;
		int y_ = 0;
		int width_ = 0;
		int height_ = 0;


		void SetParent(const Panel& parent);
	};


	//class GlyphPanel: public Panel
	//{
	//public:
	//	GlyphPanel(int x, int y, int font_size, render::ui::Glyph glyph);
	//
	//protected:
	//	Panel character_panel_;
	//};

	class TextBlock : public Panel
	{
	public:
		TextBlock(const UI& ui, render::Scene& scene, render::DescriptorSetsManager& desc_manager, int x, int y);
		TextBlock(const TextBlock&) = delete;
		void SetText(const std::basic_string<char32_t>& text, int font_size);
	protected:
		std::vector<Mesh> meshes_;
		const UI& ui_;
		render::DescriptorSetsManager& desc_manager_;

	};

}

#endif  // RENDER_ENGINE_RENDER_UI_PANEL_H_