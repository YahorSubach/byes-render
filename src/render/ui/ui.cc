#include "ui.h"


struct Glyph
{

};



render::ui::UI::UI(DeviceConfiguration& device_cfg, Extent extent): RenderObjBase(device_cfg),
    polygon_vert_pos_(device_cfg, 4 * sizeof(glm::vec3), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, { device_cfg.graphics_queue_index, device_cfg.transfer_queue_index }),
    polygon_vert_tex_(device_cfg, 4 * sizeof(glm::vec2), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, { device_cfg.graphics_queue_index, device_cfg.transfer_queue_index }),
    polygon_vert_ind_(device_cfg, 6 * sizeof(uint16_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, { device_cfg.graphics_queue_index, device_cfg.transfer_queue_index }),
    ui_sampler_(device_cfg, 0, Sampler::AddressMode::kClampToEdge), test_image_(Image::FromFile(device_cfg, "../images/old_green_painted_wood.jpg", {ImageProperty::kShaderInput})),
    index_buffer_(polygon_vert_ind_, sizeof(uint16_t), 0, 6), extent_(extent)
{
    std::array<glm::vec3, 4> positions =
    {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f)
    };

    polygon_vert_pos_.LoadData(positions.data(), sizeof(positions));


    std::array<glm::vec2, 4> tex_coords =
    {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f)
    };

    polygon_vert_tex_.LoadData(tex_coords.data(), sizeof(tex_coords));

    vertex_buffers_.push_back(BufferAccessor(polygon_vert_pos_, sizeof(glm::vec3), 0, 4));
    vertex_buffers_.push_back(BufferAccessor(polygon_vert_tex_, sizeof(glm::vec2), 0, 4));


    std::array<uint16_t, 6> indices =
    {
        0, 1, 2,
        2, 1, 3
    };

    polygon_vert_ind_.LoadData(indices.data(), sizeof(indices));

    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }

    if (FT_New_Face(ft, "../fonts/yagora_6919027/Yagora.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }

}


const std::vector<render::BufferAccessor>& render::ui::UI::GetVertexBuffers() const
{
    return vertex_buffers_;

}

const render::BufferAccessor& render::ui::UI::GetIndexBuffer() const
{
    return index_buffer_;
}

const render::ui::Glyph& render::ui::UI::GetGlyph(char character, int font_size) const
{
    auto&& font_data = size_to_font_data_[font_size];

    if (auto it = font_data.glyphs.find(character); it == font_data.glyphs.end())
    {
        FT_Set_Pixel_Sizes(face, 0, font_size);

        if (FT_Load_Char(face, character, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
        }

        bool contains_bitmap = false;

        if (face->glyph->bitmap.width != 0 && face->glyph->bitmap.rows != 0)
        {
            font_data.glyph_images.emplace((char32_t)character, Image(device_cfg_, VK_FORMAT_R8_UINT, face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer, {ImageProperty::kShaderInput }));
            contains_bitmap = true;
        }

        render::ui::Glyph glyph =
        {
            face->glyph->advance.x / 64,

            face->glyph->bitmap_left,
            font_size - face->glyph->bitmap_top,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,

            contains_bitmap ? stl_util::MakeNullableRef(font_data.glyph_images.at(character)) : std::nullopt
        };

        font_data.glyphs.emplace(character, glyph);

        return font_data.glyphs.at(character);
    }
    else
    {
        return it->second;
    }
}


const render::Sampler& render::ui::UI::GetUISampler() const
{
    return ui_sampler_;
}

render::Extent render::ui::UI::GetExtent() const
{
    return extent_;
}
