#include "descriptor_set_layout.h"

render::DescriptorSetLayout::DescriptorSetLayout(const DeviceConfiguration& device_cfg, DescriptorSetType type): RenderObjBase(device_cfg), type_(type)
{

	std::map<DescriptorSetType, DescriptorSetInfo> infos = DescriptorSetsInfos::Get();

	DescriptorSetInfo info = infos[type];

	std::vector<VkDescriptorSetLayoutBinding> bindings(info.bindings.size());

	for (int i = 0; i < info.bindings.size(); i++)
	{
		bindings[i].binding = i;

			bindings[i].descriptorType =
			info.bindings[i].type == DescriptorBindingType::kUniform			? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER :
			info.bindings[i].type == DescriptorBindingType::kSampler			? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : VK_DESCRIPTOR_TYPE_MAX_ENUM;

			bindings[i].descriptorCount = 1;

			bindings[i].stageFlags =
			((info.bindings[i].shaders_flags & ShaderTypeFlags::Vertex) != ShaderTypeFlags::Empty ? VK_SHADER_STAGE_VERTEX_BIT : 0) |
			((info.bindings[i].shaders_flags & ShaderTypeFlags::Fragment) != ShaderTypeFlags::Empty ? VK_SHADER_STAGE_FRAGMENT_BIT : 0);

		bindings[i].pImmutableSamplers = nullptr;
	}

	VkDescriptorSetLayoutCreateInfo desc_set_layout_create_info{};
	desc_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	desc_set_layout_create_info.bindingCount = u32(bindings.size());
	desc_set_layout_create_info.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device_cfg_.logical_device, &desc_set_layout_create_info, nullptr, &handle_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

render::DescriptorSetType render::DescriptorSetLayout::GetType() const
{
	return type_;
}

render::DescriptorSetLayout::~DescriptorSetLayout()
{
	if (handle_ != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(device_cfg_.logical_device, handle_, nullptr);
	}
}
