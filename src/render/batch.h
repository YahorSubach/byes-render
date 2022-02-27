#ifndef RENDER_ENGINE_RENDER_BATCH_H_
#define RENDER_ENGINE_RENDER_BATCH_H_

#include "vulkan/vulkan.h"

#include "common.h"
#include "render/object_base.h"
#include "render/buffer.h"
#include "render/graphics_pipeline.h"


namespace render
{
	class Batch
	{
	public:

		Batch(const GraphicsPipeline& pipeline, std::vector<BufferAccessor> vertex_buffers, const BufferAccessor& index_buffer, std::vector<UniformBuffer>&& uniform_buffer, std::vector<VkDescriptorSet> descriptor_sets, uint64_t draw_size, const glm::mat4& model_matrix);

		Batch(const Batch&) = delete;
		Batch(Batch&&) = default;

		Batch& operator=(const Batch&) = delete;
		Batch& operator=(Batch&&) = default;

		const GraphicsPipeline& GetPipeline() const;

		const std::vector<BufferAccessor>& GetVertexBuffers() const;
		const BufferAccessor& GetIndexBuffer() const;

		uint64_t GetDrawSize() const;

		const glm::mat4& GetModelMatrix();

		UniformBuffer& GetUniformBuffer(uint32_t frame_index) const;
		VkDescriptorSet GetDescriptorSet(uint32_t frame_index) const;
	private:

		const GraphicsPipeline& pipeline_;
		std::vector<BufferAccessor> vertex_buffers_;
		BufferAccessor index_buffer_;

		std::vector<VkDescriptorSet> descriptor_sets_;
		mutable std::vector<UniformBuffer> uniform_buffers_;

		uint64_t draw_size_;

		glm::mat4 model_matrix_;
	};
}

#endif  // RENDER_ENGINE_RENDER_BATCH_H_