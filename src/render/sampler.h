#ifndef RENDER_ENGINE_RENDER_SAMPLER_H_
#define RENDER_ENGINE_RENDER_SAMPLER_H_

#include "vulkan/vulkan.h"

#include "common.h"
#include "render/data_types.h"

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

		VkSampler GetSamplerHandle() const;

	};
}
#endif  // RENDER_ENGINE_RENDER_SAMPLER_H_