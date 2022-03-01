#ifndef RENDER_ENGINE_RENDER_COMMAND_POOL_H_
#define RENDER_ENGINE_RENDER_COMMAND_POOL_H_

#include <vector>
#include <functional>

#include "vulkan/vulkan.h"

#include "common.h"
#include "render/object_base.h"
#include "render/command_buffer.h"
#include "render/data_types.h"


namespace render
{
	class CommandPool : public RenderObjBase<VkCommandPool>
	{
	public:

		enum class PoolType
		{
			kTransfer,
			kGraphics
		};

		CommandPool(const DeviceConfiguration& device_cfg, PoolType pool_type);

		CommandPool(const CommandPool&) = delete;
		CommandPool(CommandPool&&) = default;

		CommandPool& operator=(const CommandPool&) = delete;
		CommandPool& operator=(CommandPool&&) = default;

		bool CreateCommandBuffers(uint32_t size);
		VkCommandBuffer GetCommandBuffer();

		void ClearCommandBuffers();

		const VkCommandBuffer& GetCommandBuffer(size_t index) const;

		void ExecuteOneTimeCommand(const std::function<void(VkCommandBuffer)>& command) const;

		virtual ~CommandPool() override;
	private:

		uint32_t next_available_buffer_;
		VkQueue pool_queue_;

		std::vector<VkCommandBuffer> command_buffers_;
	};
}
#endif  // RENDER_ENGINE_RENDER_COMMAND_POOL_H_