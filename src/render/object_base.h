#ifndef RENDER_ENGINE_RENDER_VALIDATION_BASE_H_
#define RENDER_ENGINE_RENDER_VALIDATION_BASE_H_

#include "vulkan/vulkan.h"

namespace render
{
	class ValidationBase
	{
	public:
		virtual bool IsValid() { return valid_; }

	protected:
		bool valid_ = true;
	};

	class RenderObjBase: public ValidationBase
	{
	public:
		RenderObjBase(const VkDevice& device) : device_(device) {}

	protected:
		const VkDevice& device_;
	};
}

#endif  // RENDER_ENGINE_RENDER_VALIDATION_BASE_H_
