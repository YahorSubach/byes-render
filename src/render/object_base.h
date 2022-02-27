#ifndef RENDER_ENGINE_RENDER_VALIDATION_BASE_H_
#define RENDER_ENGINE_RENDER_VALIDATION_BASE_H_

#include "vulkan/vulkan.h"

#include "render/data_types.h"

namespace render
{
	class ValidationBase
	{
	public:
		virtual bool IsValid() { return valid_; }
		virtual ~ValidationBase() {};
	protected:
		bool valid_ = true;
	};

	template<typename HandleType>
	class RenderObjBase: public ValidationBase
	{
	public:
 		RenderObjBase(const DeviceConfiguration& device_cfg) : device_cfg_(device_cfg), handle_(VK_NULL_HANDLE) {}

		RenderObjBase(const RenderObjBase&) = delete;
		RenderObjBase(RenderObjBase&& rhs): device_cfg_(rhs.device_cfg_), handle_(rhs.handle_)
		{
			rhs.handle_ = VK_NULL_HANDLE;
		}

		RenderObjBase& operator=(const RenderObjBase&) = delete;
		RenderObjBase& operator=(RenderObjBase&& rhs)
		{
			device_cfg_ = rhs.device_cfg_;
			handle_ = rhs.handle_;
			rhs.handle_ = VK_NULL_HANDLE;
		}

		HandleType GetHandle() const { return handle_; }

		//virtual ~RenderObjBase() override
		//{
		//	int a = 1;
		//}
		
	protected:
		const DeviceConfiguration& device_cfg_;
		HandleType handle_;
	};
}

#endif  // RENDER_ENGINE_RENDER_VALIDATION_BASE_H_
