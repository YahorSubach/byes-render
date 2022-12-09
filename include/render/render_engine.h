#ifndef RENDER_ENGINE_RENDER_VKVF_H_
#define RENDER_ENGINE_RENDER_VKVF_H_

#include <array>
#include <cmath>
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

		Vec3 operator+ (const Vec3& rhs) const
		{
			return { x + rhs.x, y + rhs.y , z + rhs.z };
		}

		Vec3 operator- (const Vec3& rhs) const
		{
			return { x - rhs.x, y - rhs.y , z - rhs.z };
		}

		Vec3 operator- () const
		{
			return { -x, -y, -z };
		}

		Vec3& operator+= (const Vec3& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			return *this;
		}

		Vec3& operator-= (const Vec3& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			return *this;
		}

		float length()
		{
			return std::sqrt(x * x + y * y + z * z);
		}

		friend Vec3 operator*(float a, const Vec3& vec)
		{
			return { a * vec.x, a * vec.y, a * vec.z };
		}

		Vec3& operator*=(float a)
		{
			x *= a;
			y *= a;
			z *= a;
			return *this;
		}

	};

	struct Camera
	{
		Vec3 position;
		Vec3 orientation;
		Vec3 up;
	};

	struct Scene
	{
		Scene();
		Camera& GetActiveCamera();
		const Camera& GetActiveCamera() const;

	private:
		class SceneImpl;
		std::unique_ptr<SceneImpl> impl_;
	};

	struct InputState
	{
		std::array<int, 0xFF> button_states;

		std::pair<int, int> mouse_position;
		std::pair<int, int> mouse_position_prev;

		std::pair<int, int> mouse_delta;
	};

	class RenderEngine
	{
	public:
		RenderEngine(InitParam param);

		//void CreateGraphicsPipeline();
		void StartRender();

		Scene& GetCurrentScene();

		const InputState& GetInputState();

		~RenderEngine();

		bool VKInitSuccess();
		//class RenderEngineImpl;
	private:
		class RenderEngineImpl;
		std::unique_ptr<RenderEngineImpl> impl_;
	};
}
#endif  // RENDER_ENGINE_RENDER_VKVF_H_