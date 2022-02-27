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

		Sampler(const DeviceConfiguration& device_cfg);

		Sampler(const Sampler&) = delete;
		Sampler(Sampler&&) = default;

		Sampler& operator=(const Sampler&) = delete;
		Sampler& operator=(Sampler&&) = default;

		virtual ~Sampler() override;
	};
}
#endif  // RENDER_ENGINE_RENDER_SAMPLER_H_