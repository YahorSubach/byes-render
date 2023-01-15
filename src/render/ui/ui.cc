#include "ui.h"

#include "render/global.h"

struct Glyph
{

};



render::ui::UI::UI(Global& global, Extent extent): RenderObjBase(global),
    polygon_vert_pos_(global, 4 * sizeof(glm::vec3), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, { global.graphics_queue_index, global.transfer_queue_index }),
    polygon_vert_tex_(global, 4 * sizeof(glm::vec2), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, { global.graphics_queue_index, global.transfer_queue_index }),
    polygon_vert_ind_(global, 6 * sizeof(uint16_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, { global.graphics_queue_index, global.transfer_queue_index }),
    ui_sampler_(global, 0, Sampler::AddressMode::kClampToEdge), test_image_(Image::FromFile(global, "../images/old_green_painted_wood.jpg"/*, {ImageProperty::kShaderInput}*/)),
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

    int font_size = 30;

    std::map<char32_t, Glyph> glyphs;

    std::basic_string_view<char32_t> char_array = U"АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдеёжзийклмнопрстуфхцчшщъыьэюя1234567890*/!?,.-_=+@# \\";
    
    int inline_cnt = sqrt(char_array.size()) + 1;

    uint32_t atlas_width = 0;

    for (int char_ind = 0; char_ind < inline_cnt; char_ind++)
    {
        FT_Set_Pixel_Sizes(face, 0, font_size);
        FT_Load_Char(face, char_array[char_ind], FT_LOAD_RENDER);
        atlas_width += face->glyph->bitmap.width;
    }

    int total_height = 0;
    int current_line_height = 0;
    int current_line_width = 0;

    for (int char_ind = 0; char_ind < char_array.size(); char_ind++)
    {
        FT_Set_Pixel_Sizes(face, 0, font_size);
        FT_Load_Char(face, char_array[char_ind], FT_LOAD_RENDER);
        
        int bitmap_width = face->glyph->bitmap.width;
        int bitmap_height = face->glyph->bitmap.rows;

        if (current_line_width + bitmap_width <= atlas_width)
        {
            current_line_width += bitmap_width;
            current_line_height = std::max(current_line_height, bitmap_height);
        }
        else
        {
            total_height += current_line_height;

            current_line_width = bitmap_width;
            current_line_height = bitmap_height;
        }
    }

    total_height += current_line_height;

    uint32_t atlas_height = total_height;

    std::vector<unsigned char> atlas_data(atlas_width * atlas_height);


    int atlas_x = 0;
    int atlas_y = 0;

    current_line_height = 0;

    for (int char_ind = 0; char_ind < char_array.size(); char_ind++)
    {
        FT_Set_Pixel_Sizes(face, 0, font_size);
        FT_Load_Char(face, char_array[char_ind], FT_LOAD_RENDER);

        int bitmap_width = face->glyph->bitmap.width;
        int bitmap_height = face->glyph->bitmap.rows;

        if (atlas_x + bitmap_width <= atlas_width)
        {
            current_line_height = std::max(current_line_height, bitmap_height);
        }
        else
        {
            atlas_y += current_line_height;
            atlas_x = 0;

            current_line_height = bitmap_height;
        }

        for (int bitmap_row_ind = 0; bitmap_row_ind < bitmap_height; bitmap_row_ind++)
        {
            std::memcpy(&atlas_data[(atlas_y + bitmap_row_ind) * atlas_width] + atlas_x, &face->glyph->bitmap.buffer[bitmap_width * bitmap_row_ind], bitmap_width);
        }


        render::ui::Glyph glyph =
        {
            face->glyph->advance.x / 64,

            face->glyph->bitmap_left,
            font_size - face->glyph->bitmap_top,
            bitmap_width,
            bitmap_height,
            glm::vec2(1.0f * atlas_x / atlas_width, 1.0f * atlas_y / atlas_height),
            glm::vec2(1.0f * bitmap_width / atlas_width, 1.0f * bitmap_height / atlas_height)
        };

        glyphs.emplace(char_array[char_ind], glyph);

        atlas_x += bitmap_width;
    }

    FontData font_data = { {},  Image(global_, VK_FORMAT_R8_SRGB, { atlas_width, atlas_height}, atlas_data.data()) };

    size_to_font_data_.emplace(font_size, std::move(font_data));
    size_to_font_data_.at(font_size).glyphs = std::move(glyphs);

    for (auto&& glyph : size_to_font_data_.at(font_size).glyphs)
    {
        glyph.second.bitmap = size_to_font_data_.at(font_size).atlas;
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

const render::ui::Glyph& render::ui::UI::GetGlyph(char32_t character, int font_size) const
{
    auto&& font_data = size_to_font_data_.at(font_size);

    if (auto it = font_data.glyphs.find(character); it == font_data.glyphs.end())
    {
        FT_Set_Pixel_Sizes(face, 0, font_size);

        auto t = U"АБВГДЕЁЖЗИ";

        if (FT_Load_Char(face, t[0], FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
        }

        bool contains_bitmap = false;

        if (face->glyph->bitmap.width != 0 && face->glyph->bitmap.rows != 0)
        {
            //font_data.glyph_images.emplace((char32_t)character, Image(global_, VK_FORMAT_R8_SRGB, { face->glyph->bitmap.width, face->glyph->bitmap.rows }, face->glyph->bitmap.buffer/*, {ImageProperty::kShaderInput }*/));
            contains_bitmap = true;
        }

        //render::ui::Glyph glyph =
        //{
        //    face->glyph->advance.x / 64,

        //    face->glyph->bitmap_left,
        //    font_size - face->glyph->bitmap_top,
        //    face->glyph->bitmap.width,
        //    face->glyph->bitmap.rows,

        //    contains_bitmap ? util::MakeNullableRef(font_data.glyph_images.at(character)) : std::nullopt
        //};

        //font_data.glyphs.emplace(character, glyph);

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

const render::Image& render::ui::UI::GetAtlas() const
{
    return size_to_font_data_.at(30).atlas;
}

render::Extent render::ui::UI::GetExtent() const
{
    return extent_;
}
