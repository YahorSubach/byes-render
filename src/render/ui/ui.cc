#include "ui.h"

render::ui::UI::UI(DeviceConfiguration& device_cfg, Extent extent): RenderObjBase(device_cfg),
    polygon_vert_pos_(device_cfg, 4 * sizeof(glm::vec3), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, { device_cfg.graphics_queue_index, device_cfg.transfer_queue_index }),
    polygon_vert_tex_(device_cfg, 4 * sizeof(glm::vec2), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, { device_cfg.graphics_queue_index, device_cfg.transfer_queue_index }),
    polygon_vert_ind_(device_cfg, 6 * sizeof(uint16_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, { device_cfg.graphics_queue_index, device_cfg.transfer_queue_index }),
    ui_sampler_(device_cfg, 0, Sampler::AddressMode::kClampToEdge), test_image_(Image::FromFile(device_cfg, "../images/test_a.png")),
    index_buffer_(polygon_vert_ind_, sizeof(uint16_t), 0, 6), extent_(extent)
{
    std::array<glm::vec3, 4> positions =
    {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.01f, 0.0f),
        glm::vec3(0.01f, 0.0f, 0.0f),
        glm::vec3(0.01f, 0.01f, 0.0f)
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

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }

    FT_Face face;
    if (FT_New_Face(ft, "../fonts/yagora_6919027/Yagora.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    if (FT_Load_Char(face, 'A', FT_LOAD_RENDER))
    {
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
    }

    char_images_.emplace('A', Image(device_cfg_, VK_FORMAT_R8_SRGB, face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer, Image::ImageType::kBitmapImage));
}


const std::vector<render::BufferAccessor>& render::ui::UI::GetVertexBuffers() const
{
    return vertex_buffers_;

}

const render::BufferAccessor& render::ui::UI::GetIndexBuffer() const
{
    return index_buffer_;
}

const render::Image& render::ui::UI::GetTestImage() const
{
    return char_images_.at('A');
}

const render::Sampler& render::ui::UI::GetUISampler() const
{
    return ui_sampler_;
}

render::Extent render::ui::UI::GetExtent() const
{
    return extent_;
}
