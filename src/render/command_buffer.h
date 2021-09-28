#ifndef RENDER_ENGINE_RENDER_COMMAND_BUFFER_H_
#define RENDER_ENGINE_RENDER_COMMAND_BUFFER_H_

#include <vector>

#include "vulkan/vulkan.h"

#include "render/object_base.h"
#include "render/command_buffer.h"

namespace render
{
	//class CommandBuffer : public RenderObjBase
	//{
	//public:
	//	CommandBuffer(const VkDevice& device, const VkExtent2D& extent, const VkImageView& image_view, const RenderPass& render_pass);

	//	CommandBuffer(const CommandBuffer&) = delete;
	//	CommandBuffer(CommandBuffer&&) = default;

	//	CommandBuffer& operator=(const CommandBuffer&) = delete;
	//	CommandBuffer& operator=(CommandBuffer&&) = default;

	//	const CommandBuffer& GetFramebufferHandle() const;

	//	~CommandBuffer();
	//private:

	//	VkCommandBuffer framebuffer_;
	//};
}
#endif  // RENDER_ENGINE_RENDER_COMMAND_BUFFER_H_