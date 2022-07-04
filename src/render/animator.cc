#include "animator.h"

template<typename ChannelValueType>
static int FindFrameIntervalBeginIndex(const render::AnimSampler<ChannelValueType>& sampler, render::Animator::Milliseconds loop_time, int channel_index, std::vector<int>& channels_indexes)
{
	if (channel_index >= channels_indexes.size())
		channels_indexes.push_back(0);

	int frame_interval_begin = channels_indexes[channel_index];

	for (; frame_interval_begin + 1 < sampler.frames.size() && sampler.frames[frame_interval_begin + 1].time < loop_time; frame_interval_begin++) {}

	return frame_interval_begin;
}

template<typename ChannelValueType>
static auto Interpolate(const render::AnimSampler<ChannelValueType>& sampler, int frame_interval_begin, render::Animator::Milliseconds loop_time, render::Animator::Milliseconds animation_duration, int channel_index, std::vector<int>& channels_indexes)
{
	if (sampler.interpolation_type == render::InterpolationType::kCubicSpline)
	{
		//https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#appendix-c-interpolation

		int k = 3 * (frame_interval_begin / 3) + 1;
		int k_pl_1 = k + 3;
		if (k_pl_1 >= sampler.frames.size()) k_pl_1 = 1;


		float tc = static_cast<float>(loop_time.count());
		float tk = static_cast<float>(sampler.frames[k].time.count());

		float tk_pl_1 = sampler.frames[k_pl_1].time.count();
		if (tk_pl_1 < tk) tk_pl_1 += animation_duration.count();


		float td = tk_pl_1 - tk;

		float t = (tc - tk) / td;

		float t2 = t * t;
		float t3 = t2 * t;


		auto&& bk = sampler.frames[k + 1].value;
		auto&& ak_pl_1 = sampler.frames[k_pl_1 - 1].value;

		auto&& vk = sampler.frames[k].value;
		auto&& vk_pl_1 = sampler.frames[k_pl_1].value;


		auto vt = (2 * t3 - 3 * t2 + 1) * vk + td * (t3 - 2 * t2 + t) * bk + (-2 * t3 + 3 * t2) * vk_pl_1 + td * (t3 - t2) * ak_pl_1;

		return vt;
	}
	else
	{
		assert(false);
	}

	return sampler.frames[frame_interval_begin].value;
}

template<typename ChannelValueType>
static auto GetChannelValue(const render::AnimSampler<ChannelValueType>& sampler, render::Animator::TimePoint start_time, render::Animator::TimePoint current_time, int channel_index, std::vector<int>& channels_indexes)
{
	render::Animator::Milliseconds time_since_start = std::chrono::duration_cast<render::Animator::Milliseconds>(current_time - start_time);

	render::Animator::Milliseconds anim_duration = (sampler.frames.back().time - sampler.frames.front().time);

	render::Animator::Milliseconds current_loop_time = time_since_start - (time_since_start / anim_duration) * anim_duration;

	int frame_interval_begin = FindFrameIntervalBeginIndex<ChannelValueType>(sampler, current_loop_time, channel_index, channels_indexes);

	return Interpolate<ChannelValueType>(sampler, frame_interval_begin, current_loop_time, anim_duration, channel_index, channels_indexes);

}


template<>
static auto GetChannelValue<glm::quat>(const render::AnimSampler<glm::quat>& sampler, render::Animator::TimePoint start_time, render::Animator::TimePoint current_time, int channel_index, std::vector<int>& channels_indexes)
{
	render::Animator::Milliseconds time_since_start = std::chrono::duration_cast<render::Animator::Milliseconds>(current_time - start_time);

	render::Animator::Milliseconds anim_duration = (sampler.frames.back().time - sampler.frames.front().time);

	render::Animator::Milliseconds current_loop_time = time_since_start - (time_since_start / anim_duration) * anim_duration;

	int frame_interval_begin = FindFrameIntervalBeginIndex<glm::quat>(sampler, current_loop_time, channel_index, channels_indexes);

	glm::quat cubic_val = Interpolate<glm::quat>(sampler, frame_interval_begin, current_loop_time, anim_duration, channel_index, channels_indexes);

	//use w component to determine factor for glm::slerp

	int k = 3 * (frame_interval_begin / 3) + 1;
	int k_pl_1 = k + 3;
	if (k_pl_1 >= sampler.frames.size()) k_pl_1 = 1;

	float factor = (cubic_val.w - sampler.frames[k].value.w) / (sampler.frames[k_pl_1].value.w - sampler.frames[k].value.w);

	return glm::slerp(sampler.frames[k].value, sampler.frames[k_pl_1].value, factor);
}


render::Animator::Animator(const Animation& animation, std::vector<Node>& animated_nodes): animation_(animation), animated_nodes_(animated_nodes)
{
}

void render::Animator::Start()
{
	start_time_ = std::chrono::steady_clock::now();
}

void render::Animator::Update()
{
	TimePoint current_time = std::chrono::steady_clock::now();
	Update(current_time);
}

void render::Animator::Update(TimePoint explicit_current_time)
{
	int channel_index = 0;

	for (auto&& rotation : animation_.rotations)
	{
		auto val = GetChannelValue<glm::quat>(rotation, start_time_, explicit_current_time, channel_index, channels_indexes_);

		auto&& node = animated_nodes_[rotation.node_index];

		node.rotation = glm::normalize(glm::quat(val.w, val.x, val.y, val.z));

		node.node_matrix = glm::translate(glm::identity<glm::mat4>(), node.translation);
		node.node_matrix *= glm::toMat4(node.rotation);
		node.node_matrix = glm::scale(node.node_matrix, node.scale);

		channel_index++;
	}

	for (auto&& translation : animation_.translations)
	{
		auto val = GetChannelValue<glm::vec3>(translation, start_time_, explicit_current_time, channel_index, channels_indexes_);

		auto&& node = animated_nodes_[translation.node_index];

		node.translation = val;

		node.node_matrix = glm::translate(glm::identity<glm::mat4>(), node.translation);
		node.node_matrix *= glm::toMat4(node.rotation);
		node.node_matrix = glm::scale(node.node_matrix, node.scale);

		channel_index++;
	}
}
