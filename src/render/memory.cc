#include "memory.h"

#include "global.h"

uint32_t total = 0;

render::Memory::Memory(const Global& global, size_t size, uint32_t memory_type_bits, VkMemoryPropertyFlags memory_flags) : RenderObjBase(global), size_(size)
{
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = size;
	allocInfo.memoryTypeIndex = GetMemoryTypeIndex(global, memory_type_bits, memory_flags); // TODO pass memory properties

	std::cout << "alloc: " << size << ", total: "<< total << std::endl;
	total += size;

	if (auto res = vkAllocateMemory(global_.logical_device, &allocInfo, nullptr, &handle_); res != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}
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
		vkFreeMemory(global_.logical_device, handle_, nullptr);
		total -= size_;
	}
}

VkDeviceMemory render::Memory::GetMemoryHandle()
{
	return handle_;
}
