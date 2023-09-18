#include "memory.h"

#include <map>

#include "global.h"
#include "data_types.h"


uint32_t total = 0;

std::unordered_map<VkDeviceMemory, std::pair<uint32_t, uint32_t>> memory_to_ind_and_size;

namespace render
{

	struct MemoryPool
	{


		uint32_t index;
		uint32_t item_size;
		uint32_t items_per_chunk;
		uint32_t chunk_size;

		std::vector<std::pair<VkDeviceMemory, uint32_t>> memory;
		std::vector<OffsettedMemory> free_memory;

		MemoryPool(uint32_t index, uint32_t size) : index(index), item_size(size)
		{
			chunk_size = (10 * 1024 / item_size) * item_size;
			items_per_chunk = chunk_size / item_size;
		}

		OffsettedMemory Allocate(VkDevice logical_device)
		{
			if (free_memory.empty())
			{
				if (memory.empty() || (memory.back().second == items_per_chunk))
				{
					VkDeviceMemory vk_memory;

					VkMemoryAllocateInfo allocInfo{};
					allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
					allocInfo.allocationSize = chunk_size;
					allocInfo.memoryTypeIndex = index;

					if (auto res = vkAllocateMemory(logical_device, &allocInfo, nullptr, &vk_memory); res != VK_SUCCESS) {
						throw std::runtime_error("failed to allocate image memory!");
					}

					memory.push_back({ vk_memory, 0 });
				}

				auto res = OffsettedMemory{ memory.back().first, memory.back().second * item_size };
				memory.back().second++;
				return res;
			}
			else
			{
				auto res = free_memory.back();
				free_memory.pop_back();

				return res;
			}
		}

		void Free(OffsettedMemory vk_memory)
		{
			free_memory.push_back(vk_memory);
		}

	};

	std::map<std::pair<uint32_t, uint32_t>, MemoryPool> memory_pools;

	void FreeMemory(VkDevice logical_device, OffsettedMemory memory)
	{
		auto&& [index, size] = memory_to_ind_and_size.at(memory.vk_memory);

		if (size > 1024)
		{
			vkFreeMemory(logical_device, memory.vk_memory, nullptr);
		}
		else
		{
			memory_pools.at(memory_to_ind_and_size[memory.vk_memory]).Free(memory);
		}
	}

	Memory::Memory(const Global& global, uint32_t size, uint32_t memory_type_bits, VkMemoryPropertyFlags memory_flags, bool deferred_free) :
		RenderObjBase(global), size_(size), deferred_free_(deferred_free)
	{
		handle_ = { VK_NULL_HANDLE, 0 };

		uint32_t index = GetMemoryTypeIndex(global, memory_type_bits, memory_flags); // TODO pass memory properties

		//total += size;

		if (size > 1024)
		{
			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = size;
			allocInfo.memoryTypeIndex = index;

			if (auto res = vkAllocateMemory(global.logical_device, &allocInfo, nullptr, &handle_.vk_memory); res != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate image memory!");
			}
		}
		else
		{
			if (!memory_pools.contains({ index , size }))
			{
				memory_pools.emplace(std::make_pair(index, size), MemoryPool(index, size));
			}

			handle_ = memory_pools.at({ index , size }).Allocate(global_.logical_device);
		}

		memory_to_ind_and_size[handle_.vk_memory] = { index , size };
	}

	uint32_t Memory::GetMemoryTypeIndex(const Global& global, uint32_t acceptable_memory_types_bits, VkMemoryPropertyFlags memory_flags)
	{
		VkPhysicalDeviceMemoryProperties memory_properties;
		vkGetPhysicalDeviceMemoryProperties(global.physical_device, &memory_properties);

		uint32_t memory_type_index = 0;

		for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
			if ((acceptable_memory_types_bits & (1 << i)) &&
				(memory_properties.memoryTypes[i].propertyFlags & memory_flags) == memory_flags) {
				memory_type_index = i;
			}
		}

		return memory_type_index;
	}

	Memory::~Memory()
	{
		if (handle_.vk_memory != VK_NULL_HANDLE)
		{
			//total -= size_;

			if (!deferred_free_)
			{
				FreeMemory(global_.logical_device, handle_);
			}
			else
			{
				global_.delete_list.push_back({ global_.frame_ind, handle_ });
			}
		}
	}

	VkDeviceMemory Memory::GetMemoryHandle()
	{
		return handle_.vk_memory;
	}

	uint32_t Memory::GetMemoryOffset()
	{
		return handle_.offset;
	}
}