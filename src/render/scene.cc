#include "scene.h"

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

render::Model::Model(const DeviceConfiguration& device_cfg, const Batch& batch, const Sampler& diffuse_sampler): 
	DescriptorSetHolder(device_cfg), batch_(batch), diffuse_sampler_(diffuse_sampler)
{
	primitives_.push_back(Primitive{ batch_.GetVertexBuffers(), batch_.GetIndexBuffer()});
	if (primitives_.back().vertex_buffers.size() == 5)
	{
		primitives_.back().type = RenderModelType::kSkinned;
	}
}

const render::Batch& render::Model::GetBatch() const
{
	return batch_;
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

void render::Model::FillData(render::DescriptorSet<render::DescriptorSetType::kSkeleton>::Binding<0>::Data& data)
{
	data.use = batch_.GetVertexBuffers().size() > 3;
}

void render::Model::FillData(render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data)
{
	data.model_mat = batch_.GetModelMatrix();
}

const std::vector<render::Primitive>& render::Model::GetPrimitives() const
{
	return primitives_;
}


render::ModelScene::ModelScene(const DeviceConfiguration& device_cfg, const BatchesManager& batch_manager, const Image& shadow_map):
	DescriptorSetHolder(device_cfg), env_image_(Image::FromFile(device_cfg, "../images/textures/CaveEnv.png")), shadow_map_(shadow_map),
	diffuse_sampler_(device_cfg, Sampler::AddressMode::kRepeat), shadow_sampler_(device_cfg, Sampler::AddressMode::kClampToBorder)
{

	models_.reserve(batch_manager.GetBatches().size());

	for (auto&& batch : batch_manager.GetBatches())
	{
		models_.push_back(Model(device_cfg, batch, diffuse_sampler_));
		children_.push_back(models_.back());
	}

	light_data_.near_plane = 0.1f;
	light_data_.far_plane = 100.f;


	light_data_.position = glm::vec4(2.0f, 1.0f, 4.0f, 1.0f);
	light_data_.proj_mat = glm::perspective(glm::radians(60.0f), 1.f, light_data_.near_plane, light_data_.far_plane);
	light_data_.proj_mat[1][1] *= -1;
	
	light_data_.view_mat = glm::lookAt(glm::vec3(2.0f, 1.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
}	

const std::vector<render::Model>& render::ModelScene::GetModels() const
{
	return models_;
}

void render::ModelScene::UpdateCameraData(glm::vec3 pos, glm::vec3 look, float aspect)
{
	camera_data_.position = glm::vec4(pos, 1.0f);
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 200.0f);
	proj[1][1] *= -1;
	camera_data_.proj_view_mat = proj * glm::lookAt(pos, pos + look, glm::vec3(0.0f, 0.0f, 1.0f));
}


void render::ModelScene::FillData(render::DescriptorSet<render::DescriptorSetType::kCameraPositionAndViewProjMat>::Binding<0>::Data& data)
{
	data = camera_data_;
}

void render::ModelScene::FillData(render::DescriptorSet<render::DescriptorSetType::kLightPositionAndViewProjMat>::Binding<0>::Data& data)
{
	data = light_data_;
}

void render::ModelScene::FillData(render::DescriptorSet<render::DescriptorSetType::kEnvironement>::Binding<0>::Data& data)
{
	data.image = &env_image_;
	data.sampler = &diffuse_sampler_;
}

void render::ModelScene::FillData(render::DescriptorSet<render::DescriptorSetType::kEnvironement>::Binding<1>::Data& data)
{
	data.image = &shadow_map_;
	data.sampler = &shadow_sampler_;
}

std::vector<std::reference_wrapper<render::Model>>& render::ModelScene::GetChildren()
{
	return children_;
}


render::UIScene::UIScene(const DeviceConfiguration& device_cfg, const ui::UI& ui): DescriptorSetHolder(device_cfg), ui_(ui)
{
	ui_polygones_.reserve(64);
	ui_polygones_geom_.reserve(64);

	ui_polygones_.push_back(UIPoly(device_cfg, ui, {100, 100}, 40));
	ui_polygones_geom_.push_back(ui_polygones_.back());

	ui_polygones_.push_back(UIPoly(device_cfg, ui, { 150, 100 }, 50));
	ui_polygones_geom_.push_back(ui_polygones_.back());
}

void render::UIScene::FillData(render::DescriptorSet<render::DescriptorSetType::kTexture>::Binding<0>::Data& data)
{
	data.image = &ui_.GetTestImage();
	data.sampler = &ui_.GetUISampler();
}

std::vector<std::reference_wrapper<render::UIPoly>>& render::UIScene::GetChildren()
{
	return ui_polygones_geom_;
}

render::UIPoly::UIPoly(const DeviceConfiguration& device_cfg, const ui::UI& ui, glm::vec2 pos, uint32_t height): DescriptorSetHolder(device_cfg), ui_(ui), polygon_vert_pos_(device_cfg, 4 * sizeof(glm::vec3), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, { device_cfg.graphics_queue_index, device_cfg.transfer_queue_index })
{
	float scale = 1.0f * height / ui.GetExtent().height;

	glm::vec2 scaled_pos = glm::vec2(pos.x / (1.0f * ui.GetExtent().width), pos.y / (1.0f * ui.GetExtent().height));

	scaled_pos = scaled_pos * 2.f - 1.0f;

	std::array<glm::vec3, 4> positions =
	{
		glm::vec3(scaled_pos.x, scaled_pos.y, 0.0f),
		glm::vec3(scaled_pos.x, scaled_pos.y + scale, 0.0f),
		glm::vec3(scaled_pos.x + scale, scaled_pos.y, 0.0f),
		glm::vec3(scaled_pos.x + scale, scaled_pos.y + scale, 0.0f)
	};

	polygon_vert_pos_.LoadData(positions.data(), sizeof(positions));

	primitives_.push_back({ui.GetVertexBuffers(), ui.GetIndexBuffer()});
}

void render::UIPoly::FillData(render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data)
{
}