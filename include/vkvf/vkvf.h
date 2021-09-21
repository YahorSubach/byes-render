#ifndef VK_VISUAL_FACADE_VKVF_H_
#define VK_VISUAL_FACADE_VKVF_H_

#include <memory>
#include <windows.h>



namespace vkvf
{
#ifdef WIN32
	using InitParam = HINSTANCE;
#endif

	class VKVisualFacade
	{
	public:
		VKVisualFacade(InitParam param);

		void CreateGraphicsPipeline();
		void ShowWindow();

		~VKVisualFacade();

		bool VKInitSuccess();
		//class VKVisualFacadeImpl;
	private:
		class VKVisualFacadeImpl;
		std::unique_ptr<VKVisualFacadeImpl> impl_;
	};
}
#endif  // VK_VISUAL_FACADE_VKVF_H_