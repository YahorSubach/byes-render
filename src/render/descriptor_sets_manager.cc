#include "descriptor_sets_manager.h"
#include "render/descriptor_pool.h"


render::DescriptorSetsManager::DescriptorSetsManager(const DeviceConfiguration& device_cfg, const RenderSetup& render_setup) : RenderObjBase(device_cfg)
{
#define ENUM_OP(x) descriptor_sets_[DescriptorSetType::k##x].resize(16); device_cfg_.descriptor_pool->AllocateSet(render_setup.GetDescriptorSetLayout(DescriptorSetType::k##x).GetHandle(), 16, descriptor_sets_[DescriptorSetType::k##x]);
#include "descriptor_types.inl"
#undef ENUM_OP
}

VkDescriptorSet render::DescriptorSetsManager::GetFreeDescriptor(DescriptorSetType type)
{
	std::map<DescriptorSetType, DescriptorSetInfo> descriptor_sets_info = DescriptorSetsInfos::Get();

	if (descriptor_sets_free_indices[type] < descriptor_sets_[type].size())
	{
		return descriptor_sets_[type][descriptor_sets_free_indices[type]++];
	}

	throw std::runtime_error("OVERFLOW!");

	return VkDescriptorSet();
}
