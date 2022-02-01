#include "batch.h"

render::Batch::Batch(GraphicsPipeline&& pipeline, Buffer&& vertex_buffer, Buffer&& index_buffer_, std::vector<Buffer>&& uniform_buffer, std::vector<VkDescriptorSet> descriptor_sets, uint64_t draw_size):
	pipeline_(std::move(pipeline)), vertex_buffer_(std::move(vertex_buffer)), index_buffer_(std::move(index_buffer_)), uniform_buffers_(std::move(uniform_buffer)), descriptor_sets_(descriptor_sets), draw_size_(draw_size)
{
}

const render::GraphicsPipeline& render::Batch::GetPipeline() const
{
	return pipeline_;
}

const render::Buffer& render::Batch::GetVertexBuffer() const
{
	return vertex_buffer_;
}

const render::Buffer& render::Batch::GetIndexBuffer() const
{
	return index_buffer_;
}

uint64_t render::Batch::GetDrawSize() const
{
	return draw_size_;
}

render::Buffer & render::Batch::GetUniformBuffer(uint32_t index) const
{
	return uniform_buffers_[index];
}

VkDescriptorSet render::Batch::GetDescriptorSet(uint32_t frame_index) const
{
	return descriptor_sets_[frame_index];
}

