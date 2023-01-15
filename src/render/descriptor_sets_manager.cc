#include "descriptor_sets_manager.h"


render::DescriptorSetsManager::DescriptorSetsManager(const Global& global) : 
	RenderObjBase(global),
	descriptor_pool_(global, 2000, 2000),
	descriptor_set_layouts_
{
#define ENUM_OP(val) DescriptorSetLayout(global, DescriptorSetType::k##val),
#include "render/descriptor_types.inl"
#undef ENUM_OP
}
{
//#define ENUM_OP(x) descriptor_sets_[DescriptorSetType::k##x].resize(64); global_.descriptor_pool->AllocateSet(render_setup.GetDescriptorSetLayout(DescriptorSetType::k##x).GetHandle(), 64, descriptor_sets_[DescriptorSetType::k##x]);
//#include "descriptor_types.inl"
//#undef ENUM_OP
}

VkDescriptorSet render::DescriptorSetsManager::GetFreeDescriptor(DescriptorSetType type)
{
	VkDescriptorSet result;

	auto&& free_typed_desc_sets = free_descriptor_sets_[type];

	if (free_typed_desc_sets.size() == 0)
	{
		free_typed_desc_sets.resize(10);
		descriptor_pool_.AllocateSet(descriptor_set_layouts_[u32(type)].GetHandle(), 10, free_typed_desc_sets);

		descriptor_sets_[type].insert(descriptor_sets_[type].end(), free_typed_desc_sets.begin(), free_typed_desc_sets.end());
	}

	result = free_typed_desc_sets.back();
	descriptor_set_to_type_[result] = type;
	free_typed_desc_sets.pop_back();

	return result;
}

void render::DescriptorSetsManager::FreeDescriptorSet(VkDescriptorSet set)
{
	DescriptorSetType set_type = descriptor_set_to_type_.at(set);
	auto&& free_desc_sets = free_descriptor_sets_.at(set_type);
	free_desc_sets.push_back(set);
}

void render::DescriptorSetsManager::FreeAll()
{
	free_descriptor_sets_ = descriptor_sets_;
}

const std::array<render::DescriptorSetLayout, static_cast<uint32_t>(render::DescriptorSetType::Count)>& render::DescriptorSetsManager::GetLayouts() const
{
	return descriptor_set_layouts_;
}

render::DescriptorSetsManager::~DescriptorSetsManager()
{
	for (auto&& [type, sets] : descriptor_sets_)
	{
		//global_.descriptor_pool->FreeSet(sets);
	}
}
