#ifndef RENDER_ENGINE_RENDER_DESCRIPTOR_SET_LAYOUT_H_
#define RENDER_ENGINE_RENDER_DESCRIPTOR_SET_LAYOUT_H_


#include "vulkan/vulkan.h"

#include <vector>

#include "render/descriptor_set.h"
#include "render/object_base.h"
#include "render/render_pass.h"

namespace render
{
	class DescriptorSetLayout : public RenderObjBase<VkDescriptorSetLayout>
	{
	public:


		struct DescriptorSetLayoutBindingDesc
		{
			uint32_t type;
			uint32_t shader_stage_flags;

		};


		DescriptorSetLayout(const DeviceConfiguration& device_cfg, DescriptorSetType type);

		DescriptorSetLayout(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout(DescriptorSetLayout&&) = default;

		DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout& operator=(DescriptorSetLayout&&) = default;

		DescriptorSetType GetType() const;

		virtual ~DescriptorSetLayout() override;

	private:

		DescriptorSetType type_;
	};
}
#endif  // RENDER_ENGINE_RENDER_DESCRIPTOR_SET_LAYOUT_H_