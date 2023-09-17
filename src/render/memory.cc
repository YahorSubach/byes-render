#include "memory.h"

#include <map>

#include "global.h"

uint32_t total = 0;

std::unordered_map<VkDeviceMemory, std::pair<uint32_t, uint32_t>> memory_to_ind_and_size;



struct MemoryPool
{
	uint32_t index;
	uint32_t size;
	std::vector<VkDeviceMemory> memory;
	std::vector<VkDeviceMemory> free_memory;

	MemoryPool(uint32_t index, uint32_t size): index(index), size(size)
	{}

	VkDeviceMemory Allocate(VkDevice logical_device)
	{
		if (free_memory.empty())
		{
			VkDeviceMemory vk_memory;

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = size;
			allocInfo.memoryTypeIndex = index;

			if (auto res = vkAllocateMemory(logical_device, &allocInfo, nullptr, &vk_memory); res != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate image memory!");
			}

			memory.push_back(vk_memory);
			free_memory.push_back(vk_memory);
		}

		auto res = free_memory.back();
		free_memory.pop_back();

		return res;
	}

	void Free(VkDeviceMemory vk_memory)
	{
		free_memory.push_back(vk_memory);
	}

};

std::map<std::pair<uint32_t, uint32_t>, MemoryPool> memory_pools;

void FreeMemory(VkDevice logical_device, VkDeviceMemory memory)
{
	auto&& [index, size] = memory_to_ind_and_size.at(memory);

	if (size > 1024)
	{
		vkFreeMemory(logical_device, memory, nullptr);
	}
	else
	{
		memory_pools.at(memory_to_ind_and_size[memory]).Free(memory);
	}
}

render::Memory::Memory(const Global& global, uint32_t size, uint32_t memory_type_bits, VkMemoryPropertyFlags memory_flags) : RenderObjBase(global), size_(size)
{
	uint32_t index = GetMemoryTypeIndex(global, memory_type_bits, memory_flags); // TODO pass memory properties

	//total += size;

	if (size > 1024)
	{
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = size;
		allocInfo.memoryTypeIndex = index;

		if (auto res = vkAllocateMemory(global.logical_device, &allocInfo, nullptr, &handle_); res != VK_SUCCESS) {
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

	memory_to_ind_and_size[handle_] = { index , size };
}

uint32_t render::Memory::GetMemoryTypeIndex(const Global& global, uint32_t acceptable_memory_types_bits, VkMemoryPropertyFlags memory_flags)
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

render::Memory::~Memory()
{
	if (handle_ != VK_NULL_HANDLE)
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

VkDeviceMemory render::Memory::GetMemoryHandle()
{
	return handle_;
}
