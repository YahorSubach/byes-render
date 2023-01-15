#include "mesh.h"

#include "render/global.h"
//#include "mesh.h"
//
//#include "common.h"
//
//render::Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<Face>& faces): vertices_(vertices), faces_(faces)
//{
//}
namespace render
{

	render::Model::Model(const Global& global, DescriptorSetsManager& manager, Node& node_in, Mesh& mesh_in, const GraphicsPipeline& pipeline_in, RenderModelCategory category) :
		ModelDescriptorSetHolder(global, manager),
		node(node_in),
		mesh(mesh_in),
		pipeline(pipeline_in),
		category(category)
	{
		UpdateData(*this);
		AttachDescriptorSets(manager);
	}

	void render::Model::FillData(const Model& model, render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<0>::Data& data)
	{
		data.flags = 0;
	}

	void render::Model::FillData(const Model& model, render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<1>::Data& data)
	{
		data.albedo = model.mesh->primitives[0].material.albedo;
		data.albedo_sampler = global_.mipmap_cnt_to_global_samplers[data.albedo->GetMipMapLevelsCount()];
	}

	void render::Model::FillData(const Model& model, render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<2>::Data& data)
	{
		data.metallic_roughness = model.mesh->primitives[0].material.metallic_roughness;
		data.metallic_roughness_sampler = global_.mipmap_cnt_to_global_samplers[data.metallic_roughness->GetMipMapLevelsCount()];
	}

	void render::Model::FillData(const Model& model, render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<3>::Data& data)
	{
		data.normal_map = model.mesh->primitives[0].material.normal_map;
		data.normal_map_sampler = global_.mipmap_cnt_to_global_samplers[data.normal_map->GetMipMapLevelsCount()];
	}

	void render::Model::FillData(const Model& model, render::DescriptorSet<render::DescriptorSetType::kSkeleton>::Binding<0>::Data& data)
	{
		
	}

	void render::Model::FillData(const Model& model, render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data)
	{
		data.model_mat = model.node.GetGlobalTransformMatrix();
	}
}
