#ifndef RENDER_ENGINE_RENDER_VKVF_H_
#define RENDER_ENGINE_RENDER_VKVF_H_

#include <memory>
#include <windows.h>



namespace render
{
#ifdef WIN32
	using InitParam = HINSTANCE;
#endif

	struct Vec3
	{
		float x;
		float y;
		float z;
	};

	struct Camera
	{
		Vec3 position;
		Vec3 orientation;
		Vec3 up;
	};

	struct Scene
	{
		Camera& GetActiveCamera();
		const Camera& GetActiveCamera() const;

	private:
		class SceneImpl;
		std::unique_ptr<SceneImpl> impl_;
	};

	class RenderEngine
	{
	public:
		RenderEngine(InitParam param);

		//void CreateGraphicsPipeline();
		void StartRender();

		Scene& GetCurrentScene();

		~RenderEngine();

		bool VKInitSuccess();
		//class RenderEngineImpl;
	private:
		class RenderEngineImpl;
		std::unique_ptr<RenderEngineImpl> impl_;
	};
}
#endif  // RENDER_ENGINE_RENDER_VKVF_H_