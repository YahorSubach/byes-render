#include "descriptor_pool.h"

#include <vector>

#include "common.h"

render::DescriptorPool::DescriptorPool(const VkDevice& device, uint32_t descriptors_count, VkDescriptorSetLayout descriptor_set_layout):
	RenderObjBase(device)
{
	VkDescriptorPoolSize pool_size{};
	pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_size.descriptorCount = descriptors_count;

	VkDescriptorPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = 1;
	pool_info.pPoolSizes = &pool_size;
	pool_info.maxSets = static_cast<uint32_t>(descriptors_count);

	if (vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}

	std::vector<VkDescriptorSetLayout> layouts(descriptors_count, descriptor_set_layout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptor_pool_;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(descriptors_count);
	allocInfo.pSetLayouts = layouts.data();

	descriptor_sets_.resize(descriptors_count);
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptor_sets_.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
}

const VkDescriptorPool& render::DescriptorPool::GetDescriptorPool() const
{
	return descriptor_pool_;
}

const VkDescriptorSet& render::DescriptorPool::GetDescriptorSet(uint32_t index) const
{
	return descriptor_sets_[index];
}

render::DescriptorPool::~DescriptorPool()
{
	if(descriptor_pool_ != VK_NULL_HANDLE)
	{ 
		vkDestroyDescriptorPool(device_, descriptor_pool_, nullptr);
	}
	descriptor_sets_.clear();
}
