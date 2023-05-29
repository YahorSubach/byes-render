#ifndef RENDER_ENGINE_RENDER_VKVF_H_
#define RENDER_ENGINE_RENDER_VKVF_H_

#include <array>
#include <variant>
#include <vector>
#include <cmath>
#include <memory>
#include <windows.h>

#include "glm/vec3.hpp"

#pragma warning(push, 0)
#include "tiny_gltf.h"
#pragma warning(pop)
namespace render
{
#ifdef WIN32
	using InitParam = HINSTANCE;
#endif

	struct Camera
	{
		glm::vec3 position;
		glm::vec3 orientation;
		glm::vec3 up;
	};

	struct Scene
	{
		Scene();
		Camera& GetActiveCamera();
		const Camera& GetActiveCamera() const;

		~Scene();

		class SceneImpl;
		std::unique_ptr<SceneImpl> impl_;
	private:

	};

	struct InputState
	{
		std::array<int, 0xFF> button_states;
		std::pair<int, int> mouse_position;
	};

	struct LoadCommand
	{
		std::shared_ptr<tinygltf::Model> model;
	};

	struct GeomCommand
	{
		std::vector<glm::vec3> faces;
	};

	using RenderCommand = std::variant<LoadCommand, GeomCommand>;


	class RenderEngine
	{
	public:
		RenderEngine(InitParam param);

		//void CreateGraphicsPipeline();
		void StartRender();

		Scene& GetCurrentScene();

		const InputState& GetInputState();

		struct DebugPoint
		{
			glm::vec3 position;
			glm::vec3 color;
		};

		void SetDebugLines(const std::vector<std::pair<DebugPoint, DebugPoint>>& lines);
		void QueueCommand(const RenderCommand& render_command);

		~RenderEngine();

		bool VKInitSuccess();
		//class RenderEngineImpl;
	private:
		class RenderEngineImpl;
		std::unique_ptr<RenderEngineImpl> impl_;
	};
}
#endif  // RENDER_ENGINE_RENDER_VKVF_H_