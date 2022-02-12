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

		Batch(GraphicsPipeline&& pipeline, std::vector<BufferSlice> vertex_buffers, const BufferSlice& index_buffer, std::vector<Buffer>&& uniform_buffer, std::vector<VkDescriptorSet> descriptor_sets, uint64_t draw_size);

		Batch(const Batch&) = delete;
		Batch(Batch&&) = default;

		Batch& operator=(const Batch&) = delete;
		Batch& operator=(Batch&&) = default;

		const GraphicsPipeline& GetPipeline() const;

		const std::vector<BufferSlice>& GetVertexBuffers() const;
		const BufferSlice& GetIndexBuffer() const;

		uint64_t GetDrawSize() const;

		Buffer& GetUniformBuffer(uint32_t frame_index) const;
		VkDescriptorSet GetDescriptorSet(uint32_t frame_index) const;
	private:

		GraphicsPipeline pipeline_;
		std::vector<BufferSlice> vertex_buffers_;
		BufferSlice index_buffer_;

		std::vector<VkDescriptorSet> descriptor_sets_;
		mutable std::vector<Buffer> uniform_buffers_;

		uint64_t draw_size_;
	};
}

#endif  // RENDER_ENGINE_RENDER_BATCH_H_