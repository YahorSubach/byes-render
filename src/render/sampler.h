#ifndef RENDER_ENGINE_RENDER_SAMPLER_H_
#define RENDER_ENGINE_RENDER_SAMPLER_H_

#include "vulkan/vulkan.h"

#include "common.h"
#include "object_base.h"

namespace render
{
	class Sampler : public RenderObjBase<VkSampler>
	{
	public:

		enum class AddressMode
		{
			kRepeat,
			kClampToEdge,
			kClampToBorder,
		};

		Sampler(const DeviceConfiguration& device_cfg, AddressMode address_mode);

		Sampler(const Sampler&) = delete;
		Sampler(Sampler&&) = default;

		Sampler& operator=(const Sampler&) = delete;
		Sampler& operator=(Sampler&&) = default;

		virtual ~Sampler() override;
	};
}
#endif  // RENDER_ENGINE_RENDER_SAMPLER_H_