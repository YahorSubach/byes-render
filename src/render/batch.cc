#include "batch.h"

render::Batch::Batch(const GraphicsPipeline& pipeline, std::vector<BufferAccessor> vertex_buffers, const BufferAccessor& index_buffer_, std::vector<UniformBuffer>&& uniform_buffer, std::vector<VkDescriptorSet> descriptor_sets, uint64_t draw_size, const glm::mat4& model_matrix):
	pipeline_(std::move(pipeline)), vertex_buffers_(vertex_buffers), index_buffer_(index_buffer_), uniform_buffers_(std::move(uniform_buffer)), descriptor_sets_(descriptor_sets), draw_size_(draw_size), model_matrix_(model_matrix)
{
}

const render::GraphicsPipeline& render::Batch::GetPipeline() const
{
	return pipeline_;
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

const glm::mat4& render::Batch::GetModelMatrix()
{
	return model_matrix_;
}

render::UniformBuffer& render::Batch::GetUniformBuffer(uint32_t index) const
{
	return uniform_buffers_[index];
}

VkDescriptorSet render::Batch::GetDescriptorSet(uint32_t frame_index) const
{
	return descriptor_sets_[frame_index];
}

