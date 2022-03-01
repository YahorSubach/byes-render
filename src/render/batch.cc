#include "batch.h"

render::Batch::Batch(std::vector<BufferAccessor> vertex_buffers, const BufferAccessor& index_buffer, const ImageView& color_image_view, uint64_t draw_size, const glm::mat4& model_matrix):
	vertex_buffers_(vertex_buffers), index_buffer_(index_buffer), draw_size_(draw_size), model_matrix_(model_matrix), color_image_view_(color_image_view)
{
}

const std::vector<render::BufferAccessor>& render::Batch::GetVertexBuffers() const
{
	return vertex_buffers_;
}

const render::BufferAccessor& render::Batch::GetIndexBuffer() const
{
	return index_buffer_;
}

uint64_t render::Batch::GetDrawSize() const
{
	return draw_size_;
}


const glm::mat4& render::Batch::GetModelMatrix() const
{
	return model_matrix_;
}

const render::ImageView& render::Batch::GetColorImageView() const
{
	return color_image_view_;
}

