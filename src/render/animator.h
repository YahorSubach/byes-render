#ifndef RENDER_ENGINE_RENDER_ANIMATOR_H_
#define RENDER_ENGINE_RENDER_ANIMATOR_H_

#include <chrono>

#include "render/mesh.h"


namespace render
{
	class Animator
	{
	public:

		using Clock = std::chrono::steady_clock;
		using Duration = std::chrono::steady_clock::duration;
		using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;
		using Milliseconds = std::chrono::milliseconds;

		Animator(const Animation& animation, std::vector<Node>& animated_nodes);

		void Start();

		void Update();
		void Update(TimePoint explicit_current_time);

	private:

		TimePoint start_time_;

		const Animation& animation_;
		std::vector<Node>& animated_nodes_;

		std::vector<int> channels_indexes_;
	};
}

#endif  // RENDER_ENGINE_RENDER_ANIMATOR_H_