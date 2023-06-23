#include "scene.h"

#include <vector>

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>


#include "descriptor_set_holder.h"
#include "render/global.h"
#include "render/render_setup.h"

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


	//Scene::Scene()
	//{
	//}

	//Camera& Scene::GetActiveCamera()
	//{
	//	return impl_->camera;
	//}
	//const Camera& Scene::GetActiveCamera() const
	//{
	//	return impl_->camera;
	//}

	//Scene::~Scene()
	//{
	//}


	/*Scene::*/SceneImpl::SceneImpl(const Global& global, DescriptorSetsManager& manager, DebugGeometry& debug_geometry_):
		SceneDescriptorSetHolder(global, manager),
		viewport_vertex_buffer_(global, 6 * sizeof(glm::vec3)),
		//viewport_primitive(global, manager),
		env_image_(global, Image::BuiltinImageType::kBlack),
		debug_geometry_(debug_geometry_),
		viewport_node_(),
		viewport_mesh_(),
		viewport_model_(global, manager, viewport_node_, viewport_mesh_),
		desc_set_manager_(manager)
	{

		active_camera_ = 0;

		std::vector<glm::vec3> viewport_vertex_data = {
			{-1, -1, 0},
			{-1, 1, 0},
			{1, 1, 0},
			
			{1, 1, 0},
			{1, -1, 0},
			{-1, -1, 0}
		};

		viewport_vertex_buffer_.LoadData(viewport_vertex_data.data(), viewport_vertex_data.size() * sizeof(glm::vec3));
		Primitive viewport_primitive(global, manager, RenderModelCategory::kViewport);
		viewport_primitive.material.pipeline_type = PipelineId::kCollectGBuffers;
		viewport_primitive.vertex_buffers[u32(VertexBufferType::kPOSITION)].emplace(BufferAccessor(viewport_vertex_buffer_, sizeof(glm::vec3), 0, 6));

		viewport_mesh_.primitives.push_back(std::move(viewport_primitive));

		models_.push_back(std::move(viewport_model_));
		models_.push_back(std::move(debug_geometry_.model));
	}

	void /*Scene::*/SceneImpl::Update(int frame_index)
	{
		debug_geometry_.Update();
		UpdateAndTryFillWrites(frame_index);
	}

	void /*Scene::*/SceneImpl::FillData(render::DescriptorSet<render::DescriptorSetType::kCameraPositionAndViewProjMat>::Binding<0>::Data& data)
	{
		auto&& camera = cameras_[active_camera_];

		data.position.x = camera.position.x;
		data.position.y = camera.position.y;
		data.position.z = camera.position.z;
		data.position.w = 1.0f;

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
		data.proj_view_mat = proj * glm::lookAt(position, position + orientation, glm::vec3(0.0f, 0.0f, 1.0f));
	}

	void /*Scene::*/SceneImpl::FillData(render::DescriptorSet<render::DescriptorSetType::kLightPositionAndViewProjMat>::Binding<0>::Data& data)
	{
		data.near_plane = 0.1f;
		data.far_plane = 100.f;


		data.position = glm::vec4(2.0f, 1.0f, 4.0f, 1.0f);
		data.proj_mat = glm::perspective(glm::radians(60.0f), 1.f, data.near_plane, data.far_plane);
		data.proj_mat[1][1] *= -1;

		data.view_mat = glm::lookAt(glm::vec3(2.0f, 1.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	}

	void /*Scene::*/SceneImpl::FillData(render::DescriptorSet<render::DescriptorSetType::kEnvironement>::Binding<0>::Data& data, util::NullableRef<const Sampler>& sampler)
	{
		data.environement = env_image_;
		sampler = global_.mipmap_cnt_to_global_samplers[env_image_.GetMipMapLevelsCount()];
	}

	Node& /*Scene::*/SceneImpl::AddNode(const Node& node)
	{
		nodes_.reserve(32);  // TODO FIX ME!!!
		nodes_.push_back(node);
		return nodes_.back();
	}

	void /*Scene::*/SceneImpl::AddModel(Node& node, Mesh& mesh)
	{
		RenderModel model(global_, desc_set_manager_, node, mesh);
		models_.push_back(std::move(model));
	}

	void SceneImpl::AddCamera()
	{
		cameras_.push_back(Camera());
	}


	std::pair<render::DebugGeometry::Point, render::DebugGeometry::Point> render::DebugGeometry::Point::operator>>(const Point& rhs)
	{
		return { *this, rhs };
	}

	DebugGeometry::DebugGeometry(const Global& global, DescriptorSetsManager& manager) :
		coords_lines_position_buffer_(global, sizeof(glm::vec3) * 512),
		coords_lines_color_buffer_(global, sizeof(glm::vec3) * 512),
		coords_lines_vertex_cnt(0),
		debug_lines_position_buffer_(global, sizeof(glm::vec3) * 512),
		debug_lines_color_buffer_(global, sizeof(glm::vec3) * 512),
		debug_lines_vertex_cnt(0),
		node(),
		mesh(),
		model(global, manager, node, mesh)
	{
		ready_to_write.store(true);
		ready_to_read.store(false);

		mesh.primitives.push_back(Primitive(global, manager, RenderModelCategory::kViewport));

		mesh.primitives.back().material = {PipelineId::kDebugLines};
		mesh.primitives.back().vertex_buffers[u32(VertexBufferType::kPOSITION)].emplace(BufferAccessor(coords_lines_position_buffer_, sizeof(glm::vec3), 0, 512));
		mesh.primitives.back().vertex_buffers[u32(VertexBufferType::kCOLOR)].emplace(BufferAccessor(coords_lines_color_buffer_, sizeof(glm::vec3), 0, 512));

		mesh.primitives.push_back(Primitive(global, manager, RenderModelCategory::kViewport));

		mesh.primitives.back().material = { PipelineId::kDebugLines };
		mesh.primitives.back().vertex_buffers[u32(VertexBufferType::kPOSITION)].emplace(BufferAccessor(debug_lines_position_buffer_, sizeof(glm::vec3), 0, 512));
		mesh.primitives.back().vertex_buffers[u32(VertexBufferType::kCOLOR)].emplace(BufferAccessor(debug_lines_color_buffer_, sizeof(glm::vec3), 0, 512));
	}

	void DebugGeometry::Update()
	{
		if (coords_lines_vertex_cnt == 0)
		{
			std::vector<glm::vec3> coords_lines_position_data = {
			{0, 0, 0},
			{1, 0, 0},

			{0, 0, 0},
			{0, 1, 0},

			{0, 0, 0},
			{0, 0, 1},
			};

			std::vector<glm::vec3> coords_lines_color_data = {
				{1, 0, 0},
				{1, 0, 0},

				{0, 1, 0},
				{0, 1, 0},

				{0, 0, 1},
				{0, 0, 1},
			};

			for (int i = -20; i < 21; i++)
			{
				coords_lines_position_data.push_back({ 0.5 * i, -10, -0.1 });
				coords_lines_position_data.push_back({ 0.5 * i, 10, -0.1 });

				coords_lines_color_data.push_back({ 0.2, 0.2, 0.2 });
				coords_lines_color_data.push_back({ 0.2, 0.2, 0.2 });
			}

			for (int i = -20; i < 21; i++)
			{
				coords_lines_position_data.push_back({ -10, 0.5 * i, -0.1 });
				coords_lines_position_data.push_back({ 10, 0.5 * i, -0.1 });

				coords_lines_color_data.push_back({ 0.2, 0.2, 0.2 });
				coords_lines_color_data.push_back({ 0.2, 0.2, 0.2 });
			}


			coords_lines_position_buffer_.LoadData(coords_lines_position_data.data(), coords_lines_position_data.size() * sizeof(glm::vec3));
			coords_lines_color_buffer_.LoadData(coords_lines_color_data.data(), coords_lines_position_data.size() * sizeof(glm::vec3));

			coords_lines_vertex_cnt = u32(coords_lines_position_data.size());
		}

		if (ready_to_read.load(std::memory_order_acquire))
		{
			debug_lines_position_buffer_.LoadData(debug_lines_position_data_.data(), debug_lines_position_data_.size() * sizeof(glm::vec3));
			debug_lines_color_buffer_.LoadData(debug_lines_color_data_.data(), debug_lines_color_data_.size() * sizeof(glm::vec3));

			debug_lines_vertex_cnt = u32(debug_lines_position_data_.size());
			mesh.primitives.back().vertex_buffers[u32(VertexBufferType::kPOSITION)]->count = debug_lines_vertex_cnt;
			mesh.primitives.back().vertex_buffers[u32(VertexBufferType::kCOLOR)]->count = debug_lines_vertex_cnt;

			ready_to_read.store(false, std::memory_order_relaxed);
			ready_to_write.store(true, std::memory_order_release);
		}
	}

	void DebugGeometry::SetDebugLines(const std::vector<std::pair<Point, Point>>& lines)
	{

		if (ready_to_write.load(std::memory_order_acquire))
		{

			debug_lines_position_data_.resize(0);
			debug_lines_color_data_.resize(0);

			for (auto&& line : lines)
			{
				debug_lines_position_data_.push_back(line.first.position);
				debug_lines_position_data_.push_back(line.second.position);

				debug_lines_color_data_.push_back(line.first.color);
				debug_lines_color_data_.push_back(line.second.color);
			}

			ready_to_write.store(false, std::memory_order_relaxed);
			ready_to_read.store(true, std::memory_order_release);
		}
	}


}