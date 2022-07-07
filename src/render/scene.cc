#include "scene.h"

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

render::ModelDescSetHolder::ModelDescSetHolder(const DeviceConfiguration& device_cfg, const Mesh& mesh): 
	DescriptorSetHolder(device_cfg), mesh_(mesh), diffuse_sampler_(Sampler(device_cfg, mesh_.primitives[0].color_tex))
{}

const render::Mesh& render::ModelDescSetHolder::GetMesh() const
{
	return mesh_;
}

void render::ModelDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<0>::Data& data)
{
	data.emit = mesh_.primitives[0].emit;
}

void render::ModelDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<1>::Data& data)
{
	if (mesh_.primitives[0].color_tex)
	{
		data.image = mesh_.primitives[0].color_tex;
	}
	else
	{
		data.image = GetDeviceCfg().default_image;
	}

	data.sampler = diffuse_sampler_;
}

void render::ModelDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kSkeleton>::Binding<0>::Data& data)
{
	int ind = 0;

	for (auto&& joint : mesh_.joints)
	{
		glm::mat4 joint_transform = glm::inverse(mesh_.node.GetGlobalTransformMatrix()) * joint.node.GetGlobalTransformMatrix() * joint.inverse_bind_matrix;
		data.matrices[ind] = joint_transform;
		ind++;
	}

	//data.use = mesh_.primitives[0].type == RenderModelType::kSkinned;
}

void render::ModelDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data)
{
	data.model_mat = mesh_.node.GetGlobalTransformMatrix();
}

render::PrimitivesHolderRenderNode render::ModelDescSetHolder::GetRenderNode()
{
	return PrimitivesHolderRenderNode(*this, mesh_.primitives);
}


render::ModelSceneDescSetHolder::ModelSceneDescSetHolder(const DeviceConfiguration& device_cfg, const BatchesManager& batch_manager, const Image& shadow_map):
	DescriptorSetHolder(device_cfg), env_image_(Image::FromFile(device_cfg, "../images/textures/CaveEnv.png")), shadow_map_(shadow_map),
	diffuse_sampler_(device_cfg, 0, Sampler::AddressMode::kRepeat), shadow_sampler_(device_cfg, 0, Sampler::AddressMode::kClampToBorder)
{

	models_.reserve(batch_manager.GetMeshes().size());

	for (auto&& mesh : batch_manager.GetMeshes())
	{
		models_.push_back(ModelDescSetHolder(device_cfg, mesh));
		children_nodes_.push_back(models_.back().GetRenderNode());
	}

	light_data_.near_plane = 0.1f;
	light_data_.far_plane = 100.f;


	light_data_.position = glm::vec4(2.0f, 1.0f, 4.0f, 1.0f);
	light_data_.proj_mat = glm::perspective(glm::radians(60.0f), 1.f, light_data_.near_plane, light_data_.far_plane);
	light_data_.proj_mat[1][1] *= -1;
	
	light_data_.view_mat = glm::lookAt(glm::vec3(2.0f, 1.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
}	

const std::vector<render::ModelDescSetHolder>& render::ModelSceneDescSetHolder::GetModels() const
{
	return models_;
}

void render::ModelSceneDescSetHolder::UpdateCameraData(glm::vec3 pos, glm::vec3 look, float aspect)
{
	camera_data_.position = glm::vec4(pos, 1.0f);
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 200.0f);
	proj[1][1] *= -1;
	camera_data_.proj_view_mat = proj * glm::lookAt(pos, pos + look, glm::vec3(0.0f, 0.0f, 1.0f));
}


void render::ModelSceneDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kCameraPositionAndViewProjMat>::Binding<0>::Data& data)
{
	data = camera_data_;
}

void render::ModelSceneDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kLightPositionAndViewProjMat>::Binding<0>::Data& data)
{
	data = light_data_;
}

void render::ModelSceneDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kEnvironement>::Binding<0>::Data& data)
{
	data.image = env_image_;
	data.sampler = diffuse_sampler_;
}

void render::ModelSceneDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kEnvironement>::Binding<1>::Data& data)
{
	data.image = shadow_map_;
	data.sampler = shadow_sampler_;
}

render::SceneRenderNode render::ModelSceneDescSetHolder::GetRenderNode()
{
	children_nodes_.clear();
	for (auto&& poly : models_)
		children_nodes_.push_back(poly.GetRenderNode());
	return SceneRenderNode(*this, children_nodes_);
}

void render::ModelSceneDescSetHolder::UpdateData()
{
	DescriptorSetHolder::UpdateData();
	for (auto&& child_model : models_)
	{
		child_model.UpdateData();
	}
}

void render::ModelSceneDescSetHolder::AttachDescriptorSets(DescriptorSetsManager& manager)
{
	DescriptorSetHolder::AttachDescriptorSets(manager);
	for (auto&& child_model : models_)
	{
		child_model.AttachDescriptorSets(manager);
	}
}

render::UIScene::UIScene(const DeviceConfiguration& device_cfg, const ui::UI& ui): DescriptorSetHolder(device_cfg), ui_(ui), screen_panel_(0,0,ui.GetExtent().width, ui.GetExtent().height), text_block_(ui, 300, 300, 30, "Hi, bro! Now we finally have a text!")
{
	ui_polygones_.reserve(64);
	ui_polygones_geom_.reserve(64);

	screen_panel_.AddChild(text_block_);
	std::vector<std::pair<glm::mat4, const Image&>> to_render;
	screen_panel_.CollectRender(glm::identity<glm::mat4>(), to_render);

	for (auto&& [transform, image] : to_render)
	{
		ui_polygones_.push_back(UIPoly(device_cfg_, ui_, transform, image));
		ui_polygones_geom_.push_back(ui_polygones_.back());
	}
}



render::SceneRenderNode render::UIScene::GetRenderNode()
{
	children_nodes_.clear();
	for (auto&& poly : ui_polygones_)
		children_nodes_.push_back(poly.GetRenderNode());
	return SceneRenderNode(*this, children_nodes_);
}

void render::UIScene::UpdateData()
{
	DescriptorSetHolder::UpdateData();
	for (auto&& child_model : ui_polygones_)
	{
		child_model.UpdateData();
	}
}

void render::UIScene::AttachDescriptorSets(DescriptorSetsManager& manager)
{
	DescriptorSetHolder::AttachDescriptorSets(manager);
	for (auto&& child_model : ui_polygones_)
	{
		child_model.AttachDescriptorSets(manager);
	}
}



render::UIPoly::UIPoly(const DeviceConfiguration& device_cfg, const ui::UI& ui, glm::mat4 transform, const Image& image): DescriptorSetHolder(device_cfg), ui_(ui), transform_(transform), image_(image)
{

	Primitive prim;
	prim.positions = ui.GetVertexBuffers()[0];
	prim.tex_coords = ui.GetVertexBuffers()[1];
	prim.indices = ui.GetIndexBuffer();

	prim.vertex_buffers = { prim.positions , prim.tex_coords };
	primitives_.push_back(prim);
}

void render::UIPoly::FillData(render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data)
{
	data.model_mat = transform_;
}

void render::UIPoly::FillData(render::DescriptorSet<render::DescriptorSetType::kTexture>::Binding<0>::Data& data)
{
	data.image = image_;
	data.sampler = ui_.GetUISampler();
}

render::PrimitivesHolderRenderNode render::UIPoly::GetRenderNode()
{
	return PrimitivesHolderRenderNode(*this, primitives_);
}


