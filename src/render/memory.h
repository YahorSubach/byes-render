#ifndef RENDER_ENGINE_RENDER_MEMORY_HOLDER_H_
#define RENDER_ENGINE_RENDER_MEMORY_HOLDER_H_

#include <vector>
#include <array>

#include "vulkan/vulkan.h"

#include "common.h"
#include "render/object_base.h"
#include "render/data_types.h"

namespace render
{
	class Memory : public RenderObjBase<OffsettedMemory>
	{
	public:
		Memory(const Global& global, uint32_t size, uint32_t align, uint32_t memory_type_bits, VkMemoryPropertyFlags memory_flags, bool deferred_free = true);
	
		Memory(const Memory&) = delete;
		Memory(Memory&&) = default;

		Memory& operator=(const Memory&) = delete;
		Memory& operator=(Memory&&) = default;

		virtual ~Memory() override;

		VkDeviceMemory GetMemoryHandle();
		uint32_t GetMemoryOffset();

		static uint32_t GetMemoryTypeIndex(const Global& global, uint32_t acceptable_memory_types_bits, VkMemoryPropertyFlags memory_flags);

	private:
		bool deferred_free_;
		uint32_t size_;

	};
}
#endif  // RENDER_ENGINE_RENDER_MEMORY_HOLDER_H_