#include "scene.h"

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

render::ModelDescSetHolder::ModelDescSetHolder(const DeviceConfiguration& device_cfg, const Mesh& mesh): 
	DescriptorSetHolder(device_cfg), mesh_(mesh), diffuse_sampler_(Sampler(device_cfg, mesh_.primitives[0].material.albedo))
{}

const render::Mesh& render::ModelDescSetHolder::GetMesh() const
{
	return mesh_;
}

void render::ModelDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<0>::Data& data)
{
	data.flags = mesh_.primitives[0].material.flags;
}

void render::ModelDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<1>::Data& data)
{
	if (mesh_.primitives[0].material.albedo)
	{
		data.albedo = mesh_.primitives[0].material.albedo;
	}
	else
	{
		data.albedo = device_cfg_.default_image;
	}

	data.albedo_sampler = diffuse_sampler_;
}

void render::ModelDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<2>::Data& data)
{
	if (mesh_.primitives[0].material.metallic_roughness)
	{
		data.metallic_roughness = mesh_.primitives[0].material.metallic_roughness;
	}
	else if(mesh_.primitives[0].material.albedo)
	{
		data.metallic_roughness = mesh_.primitives[0].material.albedo;
	}
	else
	{
		data.metallic_roughness = device_cfg_.default_image;
	}

	data.metallic_roughness_sampler = diffuse_sampler_;
}

void render::ModelDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<3>::Data& data)
{
	if (mesh_.primitives[0].material.normal_map)
	{
		data.normal_map = mesh_.primitives[0].material.normal_map;
	}
	else
	{
		data.normal_map = device_cfg_.default_image;
	}

	data.normal_map_sampler = diffuse_sampler_;
}

void render::ModelDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kSkeleton>::Binding<0>::Data& data)
{
	int ind = 0;

	for (auto&& mat : data.matrices)
	{
		mat = glm::identity<glm::mat4>();
	}

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


render::ModelSceneDescSetHolder::ModelSceneDescSetHolder(const DeviceConfiguration& device_cfg, const BatchesManager& batch_manager, const Scene& scene):
	DescriptorSetHolder(device_cfg), env_image_(device_cfg, Image::BuiltinImageType::kBlack),
	diffuse_sampler_(device_cfg, 0, Sampler::AddressMode::kRepeat), nearest_sampler_(device_cfg, 10, Sampler::AddressMode::kRepeat, true), 
	shadow_sampler_(device_cfg, 0, Sampler::AddressMode::kClampToBorder), scene_(scene)

{

	model_descriptor_sets_holders_.reserve(batch_manager.GetMeshes().size());

	for (auto&& mesh : batch_manager.GetMeshes())
	{
		model_descriptor_sets_holders_.push_back(ModelDescSetHolder(device_cfg, mesh));
		children_nodes_.push_back(model_descriptor_sets_holders_.back().GetRenderNode());
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
	return model_descriptor_sets_holders_;
}



void render::ModelSceneDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kCameraPositionAndViewProjMat>::Binding<0>::Data& data)
{
	Camera camera = scene_.GetActiveCamera();

	camera_data_.position.x = camera.position.x;
	camera_data_.position.y = camera.position.y;
	camera_data_.position.z = camera.position.z;
	camera_data_.position.w = 1.0f;

	glm::vec3 position;
	glm::vec3 orientation;
	
	position.x = camera.position.x;
	position.y = camera.position.y;
	position.z = camera.position.z;
	
	orientation.x = camera.orientation.x;
	orientation.y = camera.orientation.y;
	orientation.z = camera.orientation.z;

	glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.5f, 0.1f, 200.0f);
	proj[1][1] *= -1;
	camera_data_.proj_view_mat = proj * glm::lookAt(position, position + orientation, glm::vec3(0.0f, 0.0f, 1.0f));

	data = camera_data_;
}

void render::ModelSceneDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kLightPositionAndViewProjMat>::Binding<0>::Data& data)
{
	data = light_data_;
}

void render::ModelSceneDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kEnvironement>::Binding<0>::Data& data)
{
	data.environement = env_image_;
	data.environement_sampler = diffuse_sampler_;
}

//void render::ModelSceneDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kEnvironement>::Binding<1>::Data& data)
//{
//	data.shadow_map = shadowmap_image;
//	data.shadow_map_sampler = shadow_sampler_;
//}

//void render::ModelSceneDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kGBuffers>::Binding<0>::Data& data)
//{
//	data.albedo = g_albedo_image;
//	data.albedo_sampler = nearest_sampler_;
//}
//
//void render::ModelSceneDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kGBuffers>::Binding<1>::Data& data)
//{
//	data.position = g_position_image;
//	data.position_sampler = nearest_sampler_;
//}
//
//void render::ModelSceneDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kGBuffers>::Binding<2>::Data& data)
//{
//	data.normal = g_normal_image;
//	data.normal_sampler = nearest_sampler_;
//}
//
//void render::ModelSceneDescSetHolder::FillData(render::DescriptorSet<render::DescriptorSetType::kGBuffers>::Binding<3>::Data& data)
//{
//	data.metallic_roughness = g_metal_rough_image;
//	data.metallic_roughness_sampler = nearest_sampler_;
//}

render::SceneRenderNode render::ModelSceneDescSetHolder::GetRenderNode()
{
	children_nodes_.clear();
	for (auto&& poly : model_descriptor_sets_holders_)
		children_nodes_.push_back(poly.GetRenderNode());
	return SceneRenderNode(*this, children_nodes_);
}

void render::ModelSceneDescSetHolder::UpdateData()
{
	DescriptorSetHolder::UpdateData();
	for (auto&& child_model : model_descriptor_sets_holders_)
	{
		child_model.UpdateData();
	}
}

void render::ModelSceneDescSetHolder::AttachDescriptorSets(DescriptorSetsManager& manager)
{
	DescriptorSetHolder::AttachDescriptorSets(manager);
	for (auto&& child_model : model_descriptor_sets_holders_)
	{
		child_model.AttachDescriptorSets(manager);
	}
}

void render::ModelSceneDescSetHolder::AddModel(const render::Mesh& model)
{
	model_descriptor_sets_holders_.push_back(ModelDescSetHolder(device_cfg_, model));
	children_nodes_.push_back(model_descriptor_sets_holders_.back().GetRenderNode());
}

render::UIScene::UIScene(const DeviceConfiguration& device_cfg, const ui::UI& ui): 
	DescriptorSetHolder(device_cfg), ui_(ui), screen_panel_(0,0,ui.GetExtent().width, ui.GetExtent().height), 
	text_block_(ui, 20, 20, 30, U"!")
{
	ui_polygones_.reserve(64);
	ui_polygones_geom_.reserve(64);

	screen_panel_.AddChild(text_block_);
	std::vector<std::pair<glm::mat4, std::pair<glm::vec2, glm::vec2>>> to_render;
	screen_panel_.CollectRender(glm::identity<glm::mat4>(), to_render);

	for (auto&& [transform, atlas] : to_render)
	{
		ui_polygones_.push_back(UIPoly(device_cfg_, ui_, transform, atlas.first, atlas.second));
		ui_polygones_geom_.push_back(ui_polygones_.back());
	}
}

void render::UIScene::FillData(render::DescriptorSet<render::DescriptorSetType::kTexture>::Binding<0>::Data& data)
{
	data.texture = ui_.GetAtlas();
	data.texture_sampler = ui_.GetUISampler();
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



render::UIPoly::UIPoly(const DeviceConfiguration& device_cfg, const ui::UI& ui, glm::mat4 transform, glm::vec2 atlas_position, glm::vec2 atlas_width_height): 
	DescriptorSetHolder(device_cfg), ui_(ui), transform_(transform), atlas_position_(atlas_position), atlas_width_height_(atlas_width_height)
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

void render::UIPoly::FillData(render::DescriptorSet<render::DescriptorSetType::kBitmapAtlas>::Binding<0>::Data& data)
{
	data.atlas_position = atlas_position_;
	data.width_heigth = atlas_width_height_;
	data.color = glm::vec4(0.5, 1.0, 0.5, 1.0);
}

render::PrimitivesHolderRenderNode render::UIPoly::GetRenderNode()
{
	return PrimitivesHolderRenderNode(*this, primitives_);
}


