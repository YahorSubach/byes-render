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

    void Panel::AddChild(const std::shared_ptr<Panel>& panel)
    {
        children_.push_back(std::move(panel));
        children_.back()->SetParent(*this);
    }

    void Panel::AddModel(int x, int y, int width, int height, Mesh& mesh)
    {
        NodeId node_id = scene_.AddNode();
        Node& node = scene_.GetNode(node_id);

        node.parent = scene_.GetNode(node_id_);

        node.local_transform = glm::translate(glm::identity<glm::mat4>(), glm::vec3(1.0f * x / width_, 1.0f * y / height_, 1.0f));
        node.local_transform = glm::scale(node.local_transform, glm::vec3(1.0f * width / width_, 1.0f * height / height_, 1.0f));

        RenderModelId model_id = scene_.AddModel(node, mesh);
        node_models_ids_.push_back({ node_id, model_id });
    }

    void Panel::SetWidth(int width)
    {
        width_ = width;
        for (auto&& child : children_)
        {
            child->SetParent(*this);
        }
    }

    void Panel::SetHeight(int height)
    {
        height_ = height;
        for (auto&& child : children_)
        {
            child->SetParent(*this);
        }
    }

    void Panel::SetExtent(Extent extent)
    {
        width_ = extent.width;
        height_= extent.height;
        for (auto&& child : children_)
        {
            child->SetParent(*this);
        }
    }

    void Panel::ClearModels()
    {
        for (auto&& [node_id, model_id] : node_models_ids_)
        {
            scene_.RemoveModel(model_id);
            scene_.RemoveNode(node_id);
        }
        node_models_ids_.clear();
    }

    void Panel::SetParent(const Panel& parent)
    {
        parent_ = util::NullableRef<const Panel>(parent);

        Node& node = scene_.GetNode(node_id_);

        node.parent = scene_.GetNode(parent.node_id_);

        node.local_transform = glm::translate(glm::identity<glm::mat4>(), glm::vec3(1.0f * x_ / parent_->width_, 1.0f * y_ / parent_->height_, 1.0f));
        node.local_transform = glm::scale(node.local_transform, glm::vec3(1.0f * width_ / parent_->width_, 1.0f * height_ / parent_->height_, 1.0f));
    }

    TextBlock::TextBlock(const UI& ui, Scene& scene, DescriptorSetsManager& desc_manager, int x, int y) : Panel(scene, x, y, 0, 0), ui_(ui), desc_manager_(desc_manager)
    {}

    void TextBlock::SetText(const std::basic_string<char32_t>& text, int font_size)
    {
        ClearModels();
        meshes_.clear();

        width_ = 0;
        height_ = 0;

        std::vector<Glyph> glyphs;
        int mesh_cnt = 0;
        for (char32_t c : text)
        {
            auto glyph = ui_.GetGlyph(c, font_size);
            glyphs.emplace_back(glyph);

            if (glyph.bitmap)
            {
                mesh_cnt++;
            }

            width_ += glyph.advance;
            height_ = std::max(height_, glyph.bitmap_y + glyph.bitmap_heigth);
        }

        meshes_.reserve(mesh_cnt);

        int x_pos = 0;

        for (auto&& glyph : glyphs)
        {
            if (glyph.bitmap)
            {
                meshes_.push_back(Mesh());
                meshes_.back().primitives.push_back(primitive::Bitmap(scene_.GetDeviceCfg(), desc_manager_, ui_, glyph));

                AddModel(x_pos, glyph.bitmap_y, glyph.bitmap_width, glyph.bitmap_heigth, meshes_.back());
            }

            x_pos += glyph.advance;
        }

        if (parent_)
        {
            SetParent(*parent_);
        }
    }
}