#ifndef RENDER_ENGINE_RENDER_DESCRIPTOR_POOL_H_
#define RENDER_ENGINE_RENDER_DESCRIPTOR_POOL_H_


#include "vulkan/vulkan.h"

#include <vector>

#include "render/object_base.h"
#include "render/render_pass.h"

namespace render
{
	class DescriptorPool : public RenderObjBase<VkDescriptorPool>
	{
	public:
		DescriptorPool(const DeviceConfiguration& device_cfg, uint32_t uniform_set_cnt, uint32_t sampler_set_cnt);

		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool(DescriptorPool&&) = default;

		DescriptorPool& operator=(const DescriptorPool&) = delete;
		DescriptorPool& operator=(DescriptorPool&&) = default;

		void AllocateSet(VkDescriptorSetLayout descriptor_set_layout, uint32_t count,std::vector<VkDescriptorSet>& allocated_sets);

		virtual ~DescriptorPool() override;
	};
}
#endif  // RENDER_ENGINE_RENDER_DESCRIPTOR_POOL_H_