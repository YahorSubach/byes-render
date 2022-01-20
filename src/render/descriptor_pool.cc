#include "descriptor_pool.h"

#include <vector>
#include <array>

#include "common.h"

render::DescriptorPool::DescriptorPool(const VkDevice& device, uint32_t descriptors_count, VkDescriptorSetLayout descriptor_set_layout):
	RenderObjBase(device)
{
	std::array<VkDescriptorPoolSize, 2> pool_sizes{};
	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_sizes[0].descriptorCount = static_cast<uint32_t>(descriptors_count);
	pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_sizes[1].descriptorCount = static_cast<uint32_t>(descriptors_count);

	VkDescriptorPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = pool_sizes.size();
	pool_info.pPoolSizes = pool_sizes.data();
	pool_info.maxSets = static_cast<uint32_t>(descriptors_count);

	if (vkCreateDescriptorPool(device, &pool_info, nullptr, &handle_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}

	std::vector<VkDescriptorSetLayout> layouts(descriptors_count, descriptor_set_layout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = handle_;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(descriptors_count);
	allocInfo.pSetLayouts = layouts.data();

	descriptor_sets_.resize(descriptors_count);
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptor_sets_.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
}

const VkDescriptorSet& render::DescriptorPool::GetDescriptorSet(uint32_t index) const
{
	return descriptor_sets_[index];
}

render::DescriptorPool::~DescriptorPool()
{
	if(handle_ != VK_NULL_HANDLE)
	{ 
		vkDestroyDescriptorPool(device_, handle_, nullptr);
	}
	descriptor_sets_.clear();
}
