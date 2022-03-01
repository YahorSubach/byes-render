#ifndef RENDER_ENGINE_RENDER_BATCH_H_
#define RENDER_ENGINE_RENDER_BATCH_H_

#include "vulkan/vulkan.h"

#include "common.h"
#include "render/object_base.h"
#include "render/buffer.h"
#include "render/graphics_pipeline.h"
#include "render/image_view.h"


namespace render
{
	class Batch
	{
	public:

		Batch(std::vector<BufferAccessor> vertex_buffers, const BufferAccessor& index_buffer, const ImageView& color_image_view, uint64_t draw_size, const glm::mat4& model_matrix);

		Batch(const Batch&) = delete;
		Batch(Batch&&) = default;

		Batch& operator=(const Batch&) = delete;
		Batch& operator=(Batch&&) = default;

		const std::vector<BufferAccessor>& GetVertexBuffers() const;
		const BufferAccessor& GetIndexBuffer() const;

		uint64_t GetDrawSize() const;

		const glm::mat4& GetModelMatrix() const;

		const ImageView& GetColorImageView() const;

	private:

		std::vector<BufferAccessor> vertex_buffers_;
		BufferAccessor index_buffer_;

		uint64_t draw_size_;

		glm::mat4 model_matrix_;
		const ImageView& color_image_view_;
	};
}

#endif  // RENDER_ENGINE_RENDER_BATCH_H_