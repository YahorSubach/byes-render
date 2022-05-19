#include "batch.h"

render::Batch::Batch(std::vector<BufferAccessor> vertex_buffers, const BufferAccessor& index_buffer, const Image& color_image, uint32_t draw_size, const glm::mat4& model_matrix, bool emit_param):
	vertex_buffers_(vertex_buffers), index_buffer_(index_buffer), draw_size_(draw_size), model_matrix_(model_matrix), color_image_(color_image), emit(emit_param)
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

uint32_t render::Batch::GetDrawSize() const
{
	return draw_size_;
}


const glm::mat4& render::Batch::GetModelMatrix() const
{
	return model_matrix_;
}

const render::Image& render::Batch::GetColorImage() const
{
	return color_image_;
}

