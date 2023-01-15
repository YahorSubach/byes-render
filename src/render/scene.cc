#include "scene.h"

#include <vector>

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

#include "descriptor_set_holder.h"

namespace render
{

	//ModelDescSetHolder::ModelDescSetHolder(const Global& global, const Model& model) :
	//	DescriptorSetsHolder(global), model_(model)
	//{
	//	diffuse_sampler_ = Sampler(global, model_.mesh->primitives[0].material.albedo->GetMipMapLevelsCount());
	//}

	//const Model& ModelDescSetHolder::GetModel() const
	//{
	//	return model_;
	//}

	//void ModelDescSetHolder::FillData(DescriptorSet<DescriptorSetType::kMaterial>::Binding<0>::Data& data)
	//{
	//	if (model_.mesh)
	//	{
	//		data.flags = model_.mesh->primitives[0].material.flags;
	//	}
	//}

	//void ModelDescSetHolder::FillData(DescriptorSet<DescriptorSetType::kMaterial>::Binding<1>::Data& data)
	//{
	//	if (model_.mesh && model_.mesh->primitives[0].material.albedo)
	//	{
	//		data.albedo = model_.mesh->primitives[0].material.flags;
	//	}
	//	else
	//	{
	//		data.albedo = global_.default_image;
	//	}

	//	data.albedo_sampler = diffuse_sampler_;
	//}

	//void ModelDescSetHolder::FillData(DescriptorSet<DescriptorSetType::kMaterial>::Binding<2>::Data& data)
	//{


	//	if (model_.mesh->primitives[0].material.metallic_roughness)
	//	{
	//		data.metallic_roughness = model_.mesh->primitives[0].material.metallic_roughness;
	//	}
	//	else if (model_.mesh->primitives[0].material.albedo)
	//	{
	//		data.metallic_roughness = model_.mesh->primitives[0].material.albedo;
	//	}
	//	else
	//	{
	//		data.metallic_roughness = global_.default_image;
	//	}

	//	data.metallic_roughness_sampler = diffuse_sampler_;
	//}

	//void ModelDescSetHolder::FillData(DescriptorSet<DescriptorSetType::kMaterial>::Binding<3>::Data& data)
	//{
	//	if (model_.mesh->primitives[0].material.normal_map)
	//	{
	//		data.normal_map = model_.mesh->primitives[0].material.normal_map;
	//	}
	//	else
	//	{
	//		data.normal_map = global_.default_image;
	//	}

	//	data.normal_map_sampler = diffuse_sampler_;
	//}

	//void ModelDescSetHolder::FillData(DescriptorSet<DescriptorSetType::kSkeleton>::Binding<0>::Data& data)
	//{
	//	int ind = 0;

	//	for (auto&& mat : data.matrices)
	//	{
	//		mat = glm::identity<glm::mat4>();
	//	}

	//	for (auto&& joint : model_.skin->joints)
	//	{
	//		glm::mat4 joint_transform = glm::inverse(model_.node.GetGlobalTransformMatrix()) * joint.node.GetGlobalTransformMatrix() * joint.inverse_bind_matrix;
	//		data.matrices[ind] = joint_transform;
	//		ind++;
	//	}

	//	//data.use = mesh_.primitives[0].type == RenderModelType::kSkinned;
	//}

	//void ModelDescSetHolder::FillData(DescriptorSet<DescriptorSetType::kModelMatrix>::Binding<0>::Data& data)
	//{
	//	data.model_mat = model_.node.GetGlobalTransformMatrix();
	//}

	//PrimitivesHolderRenderNode ModelDescSetHolder::GetRenderNode()
	//{
	//	return PrimitivesHolderRenderNode(*this, model_.mesh->primitives);
	//}


	//ModelSceneDescSetHolder::ModelSceneDescSetHolder(const Global& global, const Scene& scene) :
	//	DescriptorSetsHolder(global), env_image_(global, Image::BuiltinImageType::kBlack),
	//	diffuse_sampler_(global, 0, Sampler::AddressMode::kRepeat), nearest_sampler_(global, 10, Sampler::AddressMode::kRepeat, true),
	//	shadow_sampler_(global, 0, Sampler::AddressMode::kClampToBorder), scene_(scene)

	//{

	//	//model_descriptor_sets_holders_.reserve(batch_manager.GetMeshes().size());

	//	//for (auto&& mesh : batch_manager.GetMeshes())
	//	//{
	//	//	model_descriptor_sets_holders_.push_back(ModelDescSetHolder(global, mesh));
	//	//	children_nodes_.push_back(model_descriptor_sets_holders_.back().GetRenderNode());
	//	//}


	//}

	//const std::vector<ModelDescSetHolder>& ModelSceneDescSetHolder::GetModels() const
	//{
	//	return model_descriptor_sets_holders_;
	//}



	//void ModelSceneDescSetHolder::FillData(DescriptorSet<DescriptorSetType::kCameraPositionAndViewProjMat>::Binding<0>::Data& data)
	//{
	//	Camera camera = scene_.GetActiveCamera();

	//	data.position.x = camera.position.x;
	//	data.position.y = camera.position.y;
	//	data.position.z = camera.position.z;
	//	data.position.w = 1.0f;

	//	glm::vec3 position;
	//	glm::vec3 orientation;

	//	position.x = camera.position.x;
	//	position.y = camera.position.y;
	//	position.z = camera.position.z;

	//	orientation.x = camera.orientation.x;
	//	orientation.y = camera.orientation.y;
	//	orientation.z = camera.orientation.z;

	//	glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.5f, 0.1f, 200.0f);
	//	proj[1][1] *= -1;
	//	data.proj_view_mat = proj * glm::lookAt(position, position + orientation, glm::vec3(0.0f, 0.0f, 1.0f));
	//}

	//void ModelSceneDescSetHolder::FillData(DescriptorSet<DescriptorSetType::kLightPositionAndViewProjMat>::Binding<0>::Data& data)
	//{
	//	data.near_plane = 0.1f;
	//	data.far_plane = 100.f;


	//	data.position = glm::vec4(2.0f, 1.0f, 4.0f, 1.0f);
	//	data.proj_mat = glm::perspective(glm::radians(60.0f), 1.f, data.near_plane, data.far_plane);
	//	data.proj_mat[1][1] *= -1;

	//	data.view_mat = glm::lookAt(glm::vec3(2.0f, 1.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//}

	//void ModelSceneDescSetHolder::FillData(DescriptorSet<DescriptorSetType::kEnvironement>::Binding<0>::Data& data)
	//{
	//	data.environement = env_image_;
	//	data.environement_sampler = diffuse_sampler_;
	//}

	////void ModelSceneDescSetHolder::FillData(DescriptorSet<DescriptorSetType::kEnvironement>::Binding<1>::Data& data)
	////{
	////	data.shadow_map = shadowmap_image;
	////	data.shadow_map_sampler = shadow_sampler_;
	////}

	////void ModelSceneDescSetHolder::FillData(DescriptorSet<DescriptorSetType::kGBuffers>::Binding<0>::Data& data)
	////{
	////	data.albedo = g_albedo_image;
	////	data.albedo_sampler = nearest_sampler_;
	////}
	////
	////void ModelSceneDescSetHolder::FillData(DescriptorSet<DescriptorSetType::kGBuffers>::Binding<1>::Data& data)
	////{
	////	data.position = g_position_image;
	////	data.position_sampler = nearest_sampler_;
	////}
	////
	////void ModelSceneDescSetHolder::FillData(DescriptorSet<DescriptorSetType::kGBuffers>::Binding<2>::Data& data)
	////{
	////	data.normal = g_normal_image;
	////	data.normal_sampler = nearest_sampler_;
	////}
	////
	////void ModelSceneDescSetHolder::FillData(DescriptorSet<DescriptorSetType::kGBuffers>::Binding<3>::Data& data)
	////{
	////	data.metallic_roughness = g_metal_rough_image;
	////	data.metallic_roughness_sampler = nearest_sampler_;
	////}

	//SceneRenderNode ModelSceneDescSetHolder::GetRenderNode()
	//{
	//	children_nodes_.clear();
	//	for (auto&& poly : model_descriptor_sets_holders_)
	//		children_nodes_.push_back(poly.GetRenderNode());
	//	return SceneRenderNode(*this, children_nodes_);
	//}

	//void ModelSceneDescSetHolder::UpdateData()
	//{
	//	DescriptorSetsHolder::UpdateData();
	//	for (auto&& child_model : model_descriptor_sets_holders_)
	//	{
	//		child_model.UpdateData();
	//	}
	//}

	//void ModelSceneDescSetHolder::AttachDescriptorSets(DescriptorSetsManager& manager)
	//{
	//	DescriptorSetsHolder::AttachDescriptorSets(manager);
	//	for (auto&& child_model : model_descriptor_sets_holders_)
	//	{
	//		child_model.AttachDescriptorSets(manager);
	//	}
	//}

	////void ModelSceneDescSetHolder::AddModel(const Mesh& model)
	////{
	////	model_descriptor_sets_holders_.push_back(ModelDescSetHolder(global_, model));
	////	children_nodes_.push_back(model_descriptor_sets_holders_.back().GetRenderNode());
	////}

	//UIScene::UIScene(const Global& global, const ui::UI& ui) :
	//	DescriptorSetsHolder(global), ui_(ui), screen_panel_(0, 0, ui.GetExtent().width, ui.GetExtent().height),
	//	text_block_(ui, 20, 20, 30, U"!")
	//{
	//	ui_polygones_.reserve(64);
	//	ui_polygones_geom_.reserve(64);

	//	screen_panel_.AddChild(text_block_);
	//	std::vector<std::pair<glm::mat4, std::pair<glm::vec2, glm::vec2>>> to_render;
	//	screen_panel_.CollectRender(glm::identity<glm::mat4>(), to_render);

	//	for (auto&& [transform, atlas] : to_render)
	//	{
	//		ui_polygones_.push_back(UIPoly(global_, ui_, transform, atlas.first, atlas.second));
	//		ui_polygones_geom_.push_back(ui_polygones_.back());
	//	}
	//}

	//void UIScene::FillData(DescriptorSet<DescriptorSetType::kTexture>::Binding<0>::Data& data)
	//{
	//	data.texture = ui_.GetAtlas();
	//	data.texture_sampler = ui_.GetUISampler();
	//}



	//SceneRenderNode UIScene::GetRenderNode()
	//{
	//	children_nodes_.clear();
	//	for (auto&& poly : ui_polygones_)
	//		children_nodes_.push_back(poly.GetRenderNode());
	//	return SceneRenderNode(*this, children_nodes_);
	//}

	//void UIScene::UpdateData()
	//{
	//	DescriptorSetsHolder::UpdateData();
	//	for (auto&& child_model : ui_polygones_)
	//	{
	//		child_model.UpdateData();
	//	}
	//}

	//void UIScene::AttachDescriptorSets(DescriptorSetsManager& manager)
	//{
	//	DescriptorSetsHolder::AttachDescriptorSets(manager);
	//	for (auto&& child_model : ui_polygones_)
	//	{
	//		child_model.AttachDescriptorSets(manager);
	//	}
	//}



	//UIPoly::UIPoly(const Global& global, const ui::UI& ui, glm::mat4 transform, glm::vec2 atlas_position, glm::vec2 atlas_width_height) :
	//	DescriptorSetsHolder(global), ui_(ui), transform_(transform), atlas_position_(atlas_position), atlas_width_height_(atlas_width_height)
	//{

	//	Primitive prim{ ui.GetIndexBuffer() };
	//	prim.vertex_buffers[u32(VertexBufferType::kPOSITION)] = ui.GetVertexBuffers()[0];
	//	prim.vertex_buffers[u32(VertexBufferType::kTEXCOORD)] = ui.GetVertexBuffers()[1];

	//	primitives_.push_back(prim);
	//}

	//void UIPoly::FillData(DescriptorSet<DescriptorSetType::kModelMatrix>::Binding<0>::Data& data)
	//{
	//	data.model_mat = transform_;
	//}

	//void UIPoly::FillData(DescriptorSet<DescriptorSetType::kBitmapAtlas>::Binding<0>::Data& data)
	//{
	//	data.atlas_position = atlas_position_;
	//	data.width_heigth = atlas_width_height_;
	//	data.color = glm::vec4(0.5, 1.0, 0.5, 1.0);
	//}

	//PrimitivesHolderRenderNode UIPoly::GetRenderNode()
	//{
	//	return PrimitivesHolderRenderNode(*this, primitives_);
	//}


	Scene::Scene()
	{
		impl_ = std::make_unique<SceneImpl>();
	}

	Camera& Scene::GetActiveCamera()
	{
		return impl_->camera;
	}
	const Camera& Scene::GetActiveCamera() const
	{
		return impl_->camera;
	}
	void Scene::AddModel(Model& model) const
	{
		impl_->models_.push_back(model);
	}
	const std::vector<std::reference_wrapper<Model>>& Scene::GetModels() const
	{
		return impl_->models_;
	}
	Scene::~Scene()
	{
	}


	Scene::SceneImpl::SceneImpl(const Global& global, DescriptorSetsManager& manager):
		SceneDescriptorSetHolder<SceneImpl>(global, manager),
		viewport_vertex_buffer_(global_, 6 * sizeof(glm::vec3)),
		viewport_primitive({ BufferAccessor(viewport_vertex_buffer_, sizeof(glm::vec3), 0, 6) })
	{
		std::vector<glm::vec3> viewport_vertex_data = {
			{-1, -1, 0},
			{-1, 1, 0},
			{1, 1, 0},
			
			{1, 1, 0},
			{1, -1, 0},
			{-1, -1, 0}
		};

		viewport_vertex_buffer_.LoadData(viewport_vertex_data.data(), viewport_vertex_data.size() * sizeof(glm::vec3));
	}

}