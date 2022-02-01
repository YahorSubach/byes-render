#include "descriptor_pool.h"

#include <vector>
#include <array>

#include "common.h"

render::DescriptorPool::DescriptorPool(const VkDevice& device, uint32_t uniform_set_cnt, uint32_t sampler_set_cnt):
	RenderObjBase(device)
{
	std::array<VkDescriptorPoolSize, 2> pool_sizes{};
	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_sizes[0].descriptorCount = uniform_set_cnt;
	pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_sizes[1].descriptorCount = sampler_set_cnt;

	VkDescriptorPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = pool_sizes.size();
	pool_info.pPoolSizes = pool_sizes.data();
	pool_info.maxSets = uniform_set_cnt + sampler_set_cnt;

	if (vkCreateDescriptorPool(device, &pool_info, nullptr, &handle_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}

	
}

void render::DescriptorPool::AllocateSet(VkDescriptorSetLayout descriptor_set_layout, uint32_t count, std::vector<VkDescriptorSet>& allocated_sets)
{
	std::vector<VkDescriptorSetLayout> layouts(count, descriptor_set_layout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = handle_;
	allocInfo.descriptorSetCount = count;
	allocInfo.pSetLayouts = layouts.data();

	allocated_sets.resize(count);
	if (vkAllocateDescriptorSets(device_, &allocInfo, allocated_sets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
}

render::DescriptorPool::~DescriptorPool()
{
	if(handle_ != VK_NULL_HANDLE)
	{ 
		vkDestroyDescriptorPool(device_, handle_, nullptr);
	}
}
