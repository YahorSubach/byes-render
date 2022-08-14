#ifndef RENDER_ENGINE_RENDER_DESCRIPTOR_SETS_MANAGER_H_
#define RENDER_ENGINE_RENDER_DESCRIPTOR_SETS_MANAGER_H_

#include <map>
#include <vector>


#include "vulkan/vulkan.h"

#include "render/buffer.h"
#include "render/descriptor_set.h"
#include "render/image_view.h"
#include "render/render_setup.h"
#include "render/sampler.h"

namespace render
{
	class DescriptorSetsManager: RenderObjBase<void*>
	{
	public:

		DescriptorSetsManager(const DeviceConfiguration& device_cfg, const RenderSetup& render_setup);

		DescriptorSetsManager(const DescriptorSetsManager&) = delete;
		DescriptorSetsManager(DescriptorSetsManager&&) = default;

		DescriptorSetsManager& operator=(const DescriptorSetsManager&) = delete;
		DescriptorSetsManager& operator=(DescriptorSetsManager&&) = default;

		VkDescriptorSet GetFreeDescriptor(DescriptorSetType);
		void FreeDescriptorSet(VkDescriptorSet);

		virtual ~DescriptorSetsManager();

	private:

		const RenderSetup& render_setup_;

		std::map<DescriptorSetType, std::vector<VkDescriptorSet>> descriptor_sets_;
		std::map<DescriptorSetType, std::vector<VkDescriptorSet>> free_descriptor_sets_;
		std::map<VkDescriptorSet, DescriptorSetType> descriptor_set_to_type_;

		std::map<DescriptorSetType, std::vector<UniformBuffer>> uniform_buffers_;
		std::map<DescriptorSetType, std::vector<ImageView>> image_views_;
	};
}

#endif  // RENDER_ENGINE_RENDER_DESCRIPTOR_SETS_MANAGER_H_