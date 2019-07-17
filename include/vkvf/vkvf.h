#ifndef VK_VISUAL_FACADE_VKVF_H_
#define VK_VISUAL_FACADE_VKVF_H_

#include <memory>

namespace vkvf
{
	class VKVisualFacade
	{
	public:
		VKVisualFacade();
		~VKVisualFacade();

		bool VKInitSuccess();
		//class VKVisualFacadeImpl;
	private:
		class VKVisualFacadeImpl;
		std::unique_ptr<VKVisualFacadeImpl> impl_;
	};
}
#endif  // VK_VISUAL_FACADE_VKVF_H_