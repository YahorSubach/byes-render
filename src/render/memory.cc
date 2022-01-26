#include "memory.h"

render::Memory::Memory(const DeviceConfiguration& device_cfg, uint64_t size, uint32_t memory_type_bits, VkMemoryPropertyFlags memory_flags) : RenderObjBase(device_cfg.logical_device)
{
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = size;
	allocInfo.memoryTypeIndex = GetMemoryTypeIndex(device_cfg, memory_type_bits, memory_flags); // TODO pass memory properties

	if (vkAllocateMemory(device_cfg.logical_device, &allocInfo, nullptr, &handle_) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}
}

uint32_t render::Memory::GetMemoryTypeIndex(const DeviceConfiguration& device_cfg, uint32_t acceptable_memory_types_bits, VkMemoryPropertyFlags memory_flags)
{
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(device_cfg.physical_device, &memory_properties);

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
		vkFreeMemory(device_, handle_, nullptr);
	}
}

VkDeviceMemory render::Memory::GetMemoryHandle()
{
	return handle_;
}
