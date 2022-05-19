#include "scene.h"

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

render::Model::Model(const DeviceConfiguration& device_cfg, const Batch& batch, const Sampler& diffuse_sampler): 
	DescriptorSetHolder<DescriptorSetType::kModelMatrix, DescriptorSetType::kMaterial>(device_cfg), batch_(batch), diffuse_sampler_(diffuse_sampler)
{
}

const render::Batch& render::Model::GetBatch() const
{
	return batch_;
}

void render::Model::Attach(DescriptorSetsManager& desc_set_manager)
{
	AttachDescriptorSets(desc_set_manager);
}

void render::Model::Update()
{
	UpdateData();
}

void render::Model::FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<0>::Data& data)
{
	data.emit = batch_.emit;
}

void render::Model::FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<1>::Data& data)
{
	data.image = &batch_.GetColorImage();
	data.sampler = &diffuse_sampler_;
}

void render::Model::FillData(render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data)
{
	data.model_mat = batch_.GetModelMatrix();
}

render::Scene::Scene(const DeviceConfiguration& device_cfg, const BatchesManager& batch_manager, const Image& shadow_map):
	DescriptorSetHolder(device_cfg), env_image_(Image::FromFile(device_cfg, "../images/textures/CaveEnv.png")), shadow_map_(shadow_map), 
	diffuse_sampler_(device_cfg, Sampler::AddressMode::kRepeat), shadow_sampler_(device_cfg, Sampler::AddressMode::kClampToBorder)
{
	for (auto&& batch : batch_manager.GetBatches())
	{
		models_.push_back(Model(device_cfg, batch, diffuse_sampler_));
	}

	light_data_.near_plane = 0.1f;
	light_data_.far_plane = 100.f;


	light_data_.position = glm::vec4(2.0f, 1.0f, 4.0f, 1.0f);
	light_data_.proj_mat = glm::perspective(glm::radians(60.0f), 1.f, light_data_.near_plane, light_data_.far_plane);
	light_data_.proj_mat[1][1] *= -1;
	
	light_data_.view_mat = glm::lookAt(glm::vec3(2.0f, 1.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
}	

const std::vector<render::Model>& render::Scene::GetModels() const
{
	return models_;
}

void render::Scene::UpdateCameraData(glm::vec3 pos, glm::vec3 look)
{
	camera_data_.position = glm::vec4(pos, 1.0f);
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), 16.0f / 9.f, 0.1f, 200.0f);
	proj[1][1] *= -1;
	camera_data_.proj_view_mat = proj * glm::lookAt(pos, pos + look, glm::vec3(0.0f, 0.0f, 1.0f));
}

void render::Scene::Attach(DescriptorSetsManager& desc_set_manager)
{
	AttachDescriptorSets(desc_set_manager);
	for (auto && model : models_)
	{
		model.Attach(desc_set_manager);
	}
}

void render::Scene::Update()
{
	UpdateData();

	for (auto&& model : models_)
	{
		model.Update();
	}
}

void render::Scene::FillData(render::DescriptorSet<render::DescriptorSetType::kCameraPositionAndViewProjMat>::Binding<0>::Data& data)
{
	data = camera_data_;
}

void render::Scene::FillData(render::DescriptorSet<render::DescriptorSetType::kLightPositionAndViewProjMat>::Binding<0>::Data& data)
{
	data = light_data_;
}

void render::Scene::FillData(render::DescriptorSet<render::DescriptorSetType::kEnvironement>::Binding<0>::Data& data)
{
	data.image = &env_image_;
	data.sampler = &diffuse_sampler_;
}

void render::Scene::FillData(render::DescriptorSet<render::DescriptorSetType::kEnvironement>::Binding<1>::Data& data)
{
	data.image = &shadow_map_;
	data.sampler = &shadow_sampler_;
}