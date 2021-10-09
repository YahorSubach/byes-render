#ifndef RENDER_ENGINE_RENDER_COMMAND_POOL_H_
#define RENDER_ENGINE_RENDER_COMMAND_POOL_H_

#include <vector>

#include "vulkan/vulkan.h"

#include "common.h"
#include "render/object_base.h"
#include "render/graphics_pipeline.h"
#include "render/framebuffer.h"
#include "render/render_pass.h"
#include "render/command_buffer.h"

namespace render
{
	class CommandPool : public RenderObjBase
	{
	public:
		CommandPool(const VkDevice & device, uint32_t queue_index);

		CommandPool(const CommandPool&) = delete;
		CommandPool(CommandPool&&) = default;

		CommandPool& operator=(const CommandPool&) = delete;
		CommandPool& operator=(CommandPool&&) = default;

		bool CreateCommandBuffers(uint32_t size);
		bool FillCommandBuffer(size_t index, const GraphicsPipeline& pipeline, const VkExtent2D& extent, const RenderPass& render_pass, const Framebuffer& framebuffer);

		void ClearCommandBuffers();

		const VkCommandPool& GetCommandPool() const;
		const VkCommandBuffer& GetCommandBuffer(size_t index) const;

		~CommandPool();
	private:

		std::vector<VkCommandBuffer> command_buffers_;

		VkCommandPool command_pool_;
	};
}
#endif  // RENDER_ENGINE_RENDER_COMMAND_POOL_H_