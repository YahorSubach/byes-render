#ifndef RENDER_ENGINE_RENDER_BATCH_H_
#define RENDER_ENGINE_RENDER_BATCH_H_

#include "vulkan/vulkan.h"

#include "common.h"
#include "render/object_base.h"
#include "render/buffer.h"
#include "render/image_view.h"

#include "render/descriptor_set.h"


namespace render
{
	class Batch
	{
	public:

		Batch(std::vector<BufferAccessor> vertex_buffers, const BufferAccessor& index_buffer, const Image& color_image, uint32_t draw_size, const glm::mat4& model_matrix, bool emit_param);

		Batch(const Batch&) = delete;
		Batch(Batch&&) = default;

		Batch& operator=(const Batch&) = delete;
		Batch& operator=(Batch&&) = default;

		const std::vector<BufferAccessor>& GetVertexBuffers() const;
		const BufferAccessor& GetIndexBuffer() const;

		uint32_t GetDrawSize() const;

		const glm::mat4& GetModelMatrix() const;

		const Image& GetColorImage() const;

		bool emit;

	private:

		std::vector<BufferAccessor> vertex_buffers_;
		BufferAccessor index_buffer_;

		uint32_t draw_size_;

		glm::mat4 model_matrix_;
		const Image& color_image_;
	};
}

#endif  // RENDER_ENGINE_RENDER_BATCH_H_