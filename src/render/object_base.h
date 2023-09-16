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
 		RenderObjBase(const Global& global) : global_(global), handle_(VK_NULL_HANDLE) {}

		RenderObjBase(const RenderObjBase&) = delete;
		RenderObjBase(RenderObjBase&& rhs): global_(rhs.global_), handle_(rhs.handle_)
		{
			rhs.handle_ = VK_NULL_HANDLE;
			if (handle_ == reinterpret_cast<HandleType>(0x41862000000117e))
			{
				int a = 1;
			}
		}

		RenderObjBase& operator=(const RenderObjBase&) = delete;
		RenderObjBase& operator=(RenderObjBase&& rhs)
		{
			handle_ = rhs.handle_;
			rhs.handle_ = VK_NULL_HANDLE;

			if (handle_ == reinterpret_cast<HandleType>(0x41862000000117e))
			{
				int a = 1;
			}
			return *this;
		}

		virtual HandleType GetHandle() const { return handle_; }

		const Global& GetDeviceCfg() const { return global_; };
		//virtual ~RenderObjBase() override
		//{
		//	int a = 1;
		//}
		
	protected:

		const Global& global_;
		mutable HandleType handle_;
	};

	template<typename HandleType>
	class LazyRenderObj : public RenderObjBase<HandleType>
	{
	public:
		LazyRenderObj(const Global& global) : RenderObjBase<HandleType>(global) {}

		LazyRenderObj(const LazyRenderObj&) = delete;
		LazyRenderObj(LazyRenderObj&& rhs) = default;

		LazyRenderObj& operator=(const LazyRenderObj&) = delete;
		LazyRenderObj& operator=(LazyRenderObj&& rhs) = default;

		[[nodiscard]] bool Construct() const
		{
			assert(RenderObjBase<HandleType>::handle_ == VK_NULL_HANDLE);

			bool constructed_ = InitHandle();
			return constructed_;
		}

		virtual HandleType GetHandle() const 
		{
			if (RenderObjBase<HandleType>::handle_ == VK_NULL_HANDLE)
			{
				bool res = Construct();
				assert(res);
			}

			return RenderObjBase<HandleType>::handle_;
		}

	protected:

		virtual bool InitHandle() const = 0;

	};


}

#endif  // RENDER_ENGINE_RENDER_VALIDATION_BASE_H_
