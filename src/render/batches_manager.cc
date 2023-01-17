#include "batches_manager.h"

#include <fstream>

#include "gltf_wrapper.h"

#include "command_pool.h"

namespace render
{


	BatchesManager::BatchesManager(const Global& global) : RenderObjBase(global)
	{

		gltf_wrappers_.reserve(16);
#if 0


		{

			gltf_wrappers_.push_back(GLTFWrapper(global, "../blender/old_chair/old_chair_with_cube.glb"));

			auto&& wrapper = gltf_wrappers_.back();


			for (auto&& mesh : wrapper.meshes)
			{
				meshes_.push_back(mesh);
			}
		}

		{

			gltf_wrappers_.push_back(GLTFWrapper(global, "../blender/phil/phil.glb"));

			auto&& wrapper = gltf_wrappers_.back();


			for (auto&& mesh : wrapper.meshes)
			{
				meshes_.push_back(mesh);
			}

			//animators_.push_back(Animator(gltf_wrappers_.back().animations["sit"], gltf_wrappers_.back().nodes));
			//animators_.back().Start();

			animators_.push_back(Animator(gltf_wrappers_.back().animations.at("breath "), gltf_wrappers_.back().nodes));
			animators_.back().Start();

			//animators_.push_back(Animator(gltf_wrappers_.back().animations["test_move"], gltf_wrappers_.back().nodes));
			//animators_.back().Start();
		}

		{

			gltf_wrappers_.push_back(GLTFWrapper(global, "../blender/room/room.glb"));

			auto&& wrapper = gltf_wrappers_.back();


			for (auto&& mesh : wrapper.meshes)
			{
				meshes_.push_back(mesh);
			}
		}

		{

			gltf_wrappers_.push_back(GLTFWrapper(global, "../blender/sky/sky.glb"));

			auto&& wrapper = gltf_wrappers_.back();


			for (auto&& mesh : wrapper.meshes)
			{
				meshes_.push_back(mesh);
			}
		}
#endif

	}

	
	std::vector<ModelPack>& BatchesManager::GetModelPacks()
	{
		return gltf_wrappers_;
	}

	const ImageView& BatchesManager::GetEnvImageView() const
	{
		return image_views_.front();
	}

	void BatchesManager::Update()
	{
		for (auto&& animator : animators_)
		{
			animator.Update();
		}
	}

	void BatchesManager::Add(const tinygltf::Model& model, DescriptorSetsManager& manager, const RenderSetup& render_setup)
	{
		gltf_wrappers_.push_back(GLTFWrapper(global_, model, manager, render_setup));

		auto&& wrapper = gltf_wrappers_.back();


		for (auto&& mesh : wrapper.meshes)
		{
			meshes_.push_back(mesh);
		}
	}

	glm::mat4 Node::GetGlobalTransformMatrix() const
	{
		if (parent)
			return parent->GetGlobalTransformMatrix() * local_transform;

		return local_transform;
	}
}