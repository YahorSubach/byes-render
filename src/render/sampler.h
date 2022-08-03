#ifndef RENDER_ENGINE_RENDER_SAMPLER_H_
#define RENDER_ENGINE_RENDER_SAMPLER_H_

#include "vulkan/vulkan.h"

#include "common.h"
#include "object_base.h"
#include "image.h"

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

		Sampler(const DeviceConfiguration& device_cfg, uint32_t mipmap_cnt = 0, AddressMode address_mode = AddressMode::kRepeat, bool use_nearest_filtering = false);
		Sampler(const DeviceConfiguration& device_cfg, stl_util::NullableRef<const Image> image);

		Sampler(const Sampler&) = delete;
		Sampler(Sampler&&) = default;

		Sampler& operator=(const Sampler&) = delete;
		Sampler& operator=(Sampler&&) = default;

		virtual ~Sampler() override;

	private:

		void Init(uint32_t mipmap_cnt, AddressMode address_mode, bool use_nearest_filtering = false);


	};
}
#endif  // RENDER_ENGINE_RENDER_SAMPLER_H_