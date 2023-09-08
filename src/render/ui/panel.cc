#define NOMINMAX
#include "panel.h"

//#include <glm/glm/gtc/matrix_transform.hpp>

namespace render::ui
{
    Panel::Panel(Scene& scene, int x, int y, int width, int height) : scene_(scene), x_(x), y_(y), width_(width), height_(height)
    {
        node_id_ = scene_.AddNode();
        //local_transform = glm::identity<glm::mat4>();
    }

    //Panel::Panel(const Panel& panel) :
    //    x_(panel.x_), y_(panel.y_), width_(panel.width_), height_(panel.height_), local_transform(panel.local_transform),
    //    atlas_position(panel.atlas_position), atlas_width_heigth(panel.atlas_width_heigth), children(panel.children), image_(panel.image_)
    //{
    //    for (auto&& child : children)
    //    {
    //        child.SetParent(*this);
    //    }
    //}


    //void Panel::CollectRender(glm::mat4 parent_transform, std::vector<std::pair<glm::mat4, std::pair<glm::vec2, glm::vec2>>>& to_render)
    //{
    //    if (atlas_position.x >= 0)
    //    {
    //        std::pair<glm::mat4, std::pair<glm::vec2, glm::vec2>> res(parent_transform * local_transform, { atlas_position, atlas_width_heigth });

    //        to_render.push_back(res);
    //    }

    //    for (auto&& child : children)
    //    {
    //        child.CollectRender(parent_transform * local_transform, to_render);
    //    }
    //}

    void Panel::AddChild(const Panel& panel)
    {
        children_.push_back(panel);
        children_.back().SetParent(*this);
    }

    void Panel::AddModel(int x, int y, int width, int height, Mesh& mesh)
    {
        uint32_t node_id = scene_.AddNode();
        Node& node = scene_.GetNode(node_id);

        node.parent = scene_.GetNode(node_id_);

        node.local_transform = glm::translate(glm::identity<glm::mat4>(), glm::vec3(1.0f * x / width_, 1.0f * y / height_, 1.0f));
        node.local_transform = glm::scale(node.local_transform, glm::vec3(1.0f * width / width_, 1.0f * height / height_, 1.0f));

        scene_.AddModel(node, mesh);
        //models_.push_back({node_id, })
    }

    void Panel::SetParent(const Panel& parent)
    {
        parent_ = util::NullableRef<const Panel>(parent);

        Node& node = scene_.GetNode(node_id_);

        node.parent = scene_.GetNode(parent.node_id_);

        node.local_transform = glm::translate(glm::identity<glm::mat4>(), glm::vec3(1.0f * x_ / parent_->width_, 1.0f * y_ / parent_->height_, 1.0f));
        node.local_transform = glm::scale(node.local_transform, glm::vec3(1.0f * width_ / parent_->width_, 1.0f * height_ / parent_->height_, 1.0f));
    }

    TextBlock::TextBlock(const UI& ui, Scene& scene, DescriptorSetsManager& desc_manager, int x, int y, int font_size, const std::basic_string<char32_t>& text) : Panel(scene, x, y, 0, 0)
    {
        width_ = 0;
        height_ = font_size;

        std::vector<Glyph> glyphs;
        int mesh_cnt = 0;
        for (char32_t c : text)
        {
            auto glyph = ui.GetGlyph(c, font_size);
            glyphs.emplace_back(glyph);
            
            if (glyph.bitmap)
            {
                mesh_cnt++;
            }
            
            //if (glyph.bitmap)
            //{
            //    //glyphs.emplace_back(width_, 0, font_size, glyph);
            //}

            width_ += glyph.advance;
            height_ = std::max(height_, glyph.bitmap_y + glyph.bitmap_heigth);
        }

        meshes_.reserve(mesh_cnt);

        int x_pos = 0;
        height_ = font_size;

        for (auto&& glyph : glyphs)
        {
            if (glyph.bitmap)
            {
                meshes_.push_back(Mesh());
                meshes_.back().primitives.push_back(primitive::Bitmap(scene.GetDeviceCfg(), desc_manager,ui, glyph));

                AddModel(x_pos, glyph.bitmap_y, glyph.bitmap_width, glyph.bitmap_heigth, meshes_.back());
            }

            //if (glyph.bitmap)
            //{
            //    //glyphs.emplace_back(width_, 0, font_size, glyph);
            //}

            x_pos += glyph.advance;
            /*height_ = max(height_, glyph.bitmap_y + glyph.bitmap_heigth);*/
        }


        //for (auto&& glyph : glyphs)
        //{
        //    AddChild(glyph);
        //}

        //image_ = ui.test_image_;
    }


    //GlyphPanel::GlyphPanel(int x, int y, int font_size, Glyph glyph) : Panel(x, y, glyph.advance, font_size), character_panel_(glyph.bitmap_x, glyph.bitmap_y, glyph.bitmap_width, glyph.bitmap_heigth)
    //{
    //    character_panel_.image_ = util::NullableRef<const Image>(glyph.bitmap);

    //    character_panel_.atlas_position = glyph.atlas_position;
    //    character_panel_.atlas_width_heigth = glyph.atlas_width_height;

    //    AddChild(character_panel_);
    //}
}