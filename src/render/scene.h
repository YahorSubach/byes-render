#ifndef RENDER_ENGINE_RENDER_SCENE_H_
#define RENDER_ENGINE_RENDER_SCENE_H_

#include "vulkan/vulkan.h"

#include "common.h"
#include "render/object_base.h"
#include "render/batches_manager.h"
#include "render/graphics_pipeline.h"
#include "render/image_view.h"

#include "render/descriptor_set_holder.h"


namespace render
{

	class Model : public DescriptorSetHolder<DescriptorSetType::kModelMatrix, DescriptorSetType::kMaterial>
	{
	public:
		Model(const DeviceConfiguration& device_cfg, const Batch& batch, const Sampler& diffuse_sampler);
		const Batch& GetBatch() const;

		void Attach(DescriptorSetsManager& desc_set_manager);
		void Update();

		void FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<0>::Data& data) override;

		void FillData(render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data) override;
	private:
		const Batch& batch_;
		const Sampler& diffuse_sampler_;
	};

	class Scene : public DescriptorSetHolder<DescriptorSetType::kCameraPositionAndViewProjMat, DescriptorSetType::kLightPositionAndViewProjMat, DescriptorSetType::kEnvironement>
	{
	public:

		Scene(const DeviceConfiguration& device_cfg, const BatchesManager& batch_manager, const Image& shadow_map);

		const std::vector<Model>& GetModels() const;

		void UpdateCameraData(glm::vec3 pos, glm::vec3 look);
		
		void Attach(DescriptorSetsManager& desc_set_manager);
		void Update();

		void FillData(render::DescriptorSet<render::DescriptorSetType::kCameraPositionAndViewProjMat>::Binding<0>::Data& data) override;
		void FillData(render::DescriptorSet<render::DescriptorSetType::kLightPositionAndViewProjMat>::Binding<0>::Data& data) override;
		void FillData(render::DescriptorSet<render::DescriptorSetType::kEnvironement>::Binding<0>::Data& data) override;
		void FillData(render::DescriptorSet<render::DescriptorSetType::kEnvironement>::Binding<1>::Data& data) override;
	private:
		
		std::vector<Model> models_;
		DescriptorSet<DescriptorSetType::kCameraPositionAndViewProjMat>::Binding<0>::Data camera_data_;
		DescriptorSet<DescriptorSetType::kLightPositionAndViewProjMat>::Binding<0>::Data light_data_;

		Image env_image_;
		const Image& shadow_map_;

		Sampler diffuse_sampler_;
		Sampler shadow_sampler_;
	};

}

#endif  // RENDER_ENGINE_RENDER_SCENE_H_