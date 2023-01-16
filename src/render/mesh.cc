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

	render::Model::Model(const Global& global, DescriptorSetsManager& manager, Node& node_in, Mesh& mesh_in) :
		ModelDescriptorSetHolder(global, manager),
		node(node_in),
		mesh(mesh_in)
	{
		UpdateData(*this);
		AttachDescriptorSets(manager);
	}

	Primitive::Primitive(const Global& global, DescriptorSetsManager& manager): PrimitiveDescriptorSetHolder(global, manager)
	{
		UpdateData(*this);
		AttachDescriptorSets(manager);
	}

	void render::Primitive::FillData(const Primitive& model, render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<0>::Data& data)
	{
		data.flags = 0;
	}

	void render::Primitive::FillData(const Primitive& model, render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<1>::Data& data)
	{
		if (material.metallic_roughness)
		{
			data.albedo = material.metallic_roughness;
		}
		else
		{
			data.albedo = global_.default_image;
		}

		data.albedo_sampler = global_.mipmap_cnt_to_global_samplers[data.albedo->GetMipMapLevelsCount()];
	}

	void render::Primitive::FillData(const Primitive& model, render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<2>::Data& data)
	{
		if (material.metallic_roughness)
		{
			data.metallic_roughness = material.metallic_roughness;
		}
		else
		{
			data.metallic_roughness = global_.default_image;
		}

		data.metallic_roughness_sampler = global_.mipmap_cnt_to_global_samplers[data.metallic_roughness->GetMipMapLevelsCount()];
	}

	void render::Primitive::FillData(const Primitive& model, render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<3>::Data& data)
	{
		if (material.normal_map)
		{
			data.normal_map = material.normal_map;
		}
		else
		{
			data.normal_map = global_.default_image;
		}

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
