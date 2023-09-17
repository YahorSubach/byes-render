#include "sampler.h"

#include "global.h"

render::Sampler::Sampler(const Global& global, uint32_t mipmap_cnt, AddressMode address_mode, bool use_nearest_filtering): RenderObjBase(global)
{
	Init(mipmap_cnt, address_mode, use_nearest_filtering);

}

void render::Sampler::Init(uint32_t mipmap_cnt, AddressMode address_mode, bool use_nearest_filtering)
{
	VkSamplerCreateInfo sampler_info{};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = use_nearest_filtering ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
	sampler_info.minFilter = use_nearest_filtering ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;

	VkSamplerAddressMode vk_address_mode;

	switch (address_mode)
	{
	case render::Sampler::AddressMode::kRepeat:
		vk_address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		break;
	case render::Sampler::AddressMode::kClampToEdge:
		vk_address_mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		break;
	case render::Sampler::AddressMode::kClampToBorder:
		vk_address_mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		break;
	default:
		break;
	}

	sampler_info.addressModeU = vk_address_mode;
	sampler_info.addressModeV = vk_address_mode;
	sampler_info.addressModeW = vk_address_mode;
	sampler_info.anisotropyEnable = VK_TRUE;
	sampler_info.maxAnisotropy = global_.physical_device_properties.limits.maxSamplerAnisotropy;
	sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	sampler_info.unnormalizedCoordinates = VK_FALSE;
	sampler_info.compareEnable = VK_FALSE;
	sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
	sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_info.mipLodBias = 0.0f;
	sampler_info.minLod = 0.0f;
	sampler_info.maxLod = static_cast<float>(mipmap_cnt);

	if (vkCreateSampler(global_.logical_device, &sampler_info, nullptr, &handle_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

render::Sampler::~Sampler()
{
	if (handle_ != VK_NULL_HANDLE)
	{
		vkDestroySampler(global_.logical_device, handle_, nullptr);
	}
}
