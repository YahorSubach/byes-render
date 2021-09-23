#ifndef RENDER_ENGINE_RENDER_OBJ_BASE_H_
#define RENDER_ENGINE_RENDER_OBJ_BASE_H_

#include "vulkan/vulkan.h"

namespace render
{
	class ValidateObject
	{
	public:
		virtual bool IsValid() { return valid_; }

	protected:
		bool valid_ = true;
	};
}

#endif  // RENDER_ENGINE_RENDER_OBJ_BASE_H_
