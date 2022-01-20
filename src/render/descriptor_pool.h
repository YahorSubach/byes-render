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
		DescriptorPool(const VkDevice& device, uint32_t descriptors_count, VkDescriptorSetLayout descriptor_set_layout);

		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool(DescriptorPool&&) = default;

		DescriptorPool& operator=(const DescriptorPool&) = delete;
		DescriptorPool& operator=(DescriptorPool&&) = default;

		const VkDescriptorSet& GetDescriptorSet(uint32_t index) const;

		virtual ~DescriptorPool() override;
	private:

		std::vector<VkDescriptorSet> descriptor_sets_;
	};
}
#endif  // RENDER_ENGINE_RENDER_DESCRIPTOR_POOL_H_