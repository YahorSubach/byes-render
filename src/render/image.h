#ifndef RENDER_ENGINE_RENDER_IMAGE_H_
#define RENDER_ENGINE_RENDER_IMAGE_H_

#include <vector>
#include <array>
#include <memory>

#include "vulkan/vulkan.h"

#include "common.h"
#include "render/object_base.h"
#include "render/command_pool.h"
#include "render/buffer.h"
#include "render/memory.h"

namespace render
{
	class Image : public RenderObjBase<VkImage>
	{
	public:
		
		enum class TransitionType
		{
			kPrepareToTransfer,
			kTransferToFragment,
		};

		Image(const DeviceConfiguration& device_cfg, VkFormat format, const uint32_t& width, const uint32_t& height, const void* pixels);
		Image(const DeviceConfiguration& device_cfg, VkFormat format, const uint32_t& width, const uint32_t& height);

		static Image FromFile(const DeviceConfiguration& device_cfg, const std::string& path);

		Image(const Image&) = delete;
		Image(Image&&) = default;

		Image& operator=(const Image&) = delete;
		Image& operator=(Image&&) = default;

		VkFormat GetFromat() const;

		void TransitionImageLayout(const CommandPool& command_pool, TransitionType transfer_type);
		void CopyBuffer(const CommandPool& command_pool, const Buffer& buffer, uint32_t width, uint32_t height);

		virtual ~Image() override;

		VkImage GetImageHandle() const;

	private:

		std::unique_ptr<Memory> memory_;
		VkFormat format_;

	};
}
#endif  // RENDER_ENGINE_RENDER_IMAGE_H_