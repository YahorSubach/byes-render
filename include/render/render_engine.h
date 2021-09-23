#ifndef RENDER_ENGINE_RENDER_VKVF_H_
#define RENDER_ENGINE_RENDER_VKVF_H_

#include <memory>
#include <windows.h>



namespace render
{
#ifdef WIN32
	using InitParam = HINSTANCE;
#endif

	class RenderEngine
	{
	public:
		RenderEngine(InitParam param);

		void CreateGraphicsPipeline();
		void ShowWindow();

		~RenderEngine();

		bool VKInitSuccess();
		//class RenderEngineImpl;
	private:
		class RenderEngineImpl;
		std::unique_ptr<RenderEngineImpl> impl_;
	};
}
#endif  // RENDER_ENGINE_RENDER_VKVF_H_