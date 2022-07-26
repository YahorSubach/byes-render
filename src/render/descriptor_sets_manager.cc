#include "descriptor_sets_manager.h"
#include "render/descriptor_pool.h"


render::DescriptorSetsManager::DescriptorSetsManager(const DeviceConfiguration& device_cfg, const RenderSetup& render_setup) : RenderObjBase(device_cfg)
{
#define ENUM_OP(x) descriptor_sets_[DescriptorSetType::k##x].resize(64); device_cfg_.descriptor_pool->AllocateSet(render_setup.GetDescriptorSetLayout(DescriptorSetType::k##x).GetHandle(), 64, descriptor_sets_[DescriptorSetType::k##x]);
#include "descriptor_types.inl"
#undef ENUM_OP
}

VkDescriptorSet render::DescriptorSetsManager::GetFreeDescriptor(DescriptorSetType type)
{
	auto&& descriptor_sets_info = DescriptorSetUtil::GetTypeToInfoMap();

	if (descriptor_sets_free_indices[type] < descriptor_sets_[type].size())
	{
		return descriptor_sets_[type][descriptor_sets_free_indices[type]++];
	}

	throw std::runtime_error("OVERFLOW!");

	return VkDescriptorSet();
}

render::DescriptorSetsManager::~DescriptorSetsManager()
{
	for (auto&& [type, sets] : descriptor_sets_)
	{
		//device_cfg_.descriptor_pool->FreeSet(sets);
	}
}