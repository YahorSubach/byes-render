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
			assert(&device_cfg_ == &rhs.device_cfg_);
			handle_ = rhs.handle_;
			rhs.handle_ = VK_NULL_HANDLE;
			return *this;
		}

		virtual HandleType GetHandle() const { return handle_; }

		const DeviceConfiguration& GetDeviceCfg() { return device_cfg_; };
		//virtual ~RenderObjBase() override
		//{
		//	int a = 1;
		//}
		
		const DeviceConfiguration& device_cfg_;
		mutable HandleType handle_;
	};

	template<typename HandleType>
	class LazyRenderObj : public RenderObjBase<HandleType>
	{
	public:
		LazyRenderObj(const DeviceConfiguration& device_cfg) : RenderObjBase(device_cfg) {}

		LazyRenderObj(const LazyRenderObj&) = delete;
		LazyRenderObj(LazyRenderObj&& rhs) = default;

		LazyRenderObj& operator=(const LazyRenderObj&) = delete;
		LazyRenderObj& operator=(LazyRenderObj&& rhs) = default;

		[[nodiscard]] bool Construct() const
		{
			assert(handle_ == VK_NULL_HANDLE);

			bool constructed_ = InitHandle();
			return constructed_;
		}

		virtual HandleType GetHandle() const 
		{
			if (handle_ == VK_NULL_HANDLE)
			{
				Construct();
			}

			return handle_; 
		}

	protected:

		virtual bool InitHandle() const = 0;

	};


}

#endif  // RENDER_ENGINE_RENDER_VALIDATION_BASE_H_
