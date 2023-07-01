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

	render::RenderModel::RenderModel(const Global& global, DescriptorSetsManager& manager, Node& node_in, Mesh& mesh_in) :
		RenderModelDescriptorSetHolder(global, manager),
		node(node_in),
		mesh(mesh_in)
	{
	}

	Primitive::Primitive(const Global& global, DescriptorSetsManager& manager, PrimitiveFlags flags): PrimitiveDescriptorSetHolder(global, manager), flags(flags)
	{
	}

	void render::Primitive::FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<0>::Data& data)
	{
		data.flags = 0;
	}

	void render::Primitive::FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<1>::Data& data, util::NullableRef<const Sampler>& sampler)
	{
		if (material.albedo)
		{
			data.albedo = material.albedo;
		}
		else
		{
			data.albedo = global_.error_image;
		}

		sampler = global_.mipmap_cnt_to_global_samplers[data.albedo->GetMipMapLevelsCount()];
	}

	void render::Primitive::FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<2>::Data& data, util::NullableRef<const Sampler>& sampler)
	{
		if (material.metallic_roughness)
		{
			data.metallic_roughness = material.metallic_roughness;
		}
		else
		{
			data.metallic_roughness = global_.error_image;
		}

		sampler = global_.mipmap_cnt_to_global_samplers[data.metallic_roughness->GetMipMapLevelsCount()];
	}

	void render::Primitive::FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<3>::Data& data, util::NullableRef<const Sampler>& sampler)
	{
		if (material.normal_map)
		{
			data.normal_map = material.normal_map;
		}
		else
		{
			data.normal_map = global_.error_image;
		}

		sampler = global_.mipmap_cnt_to_global_samplers[data.normal_map->GetMipMapLevelsCount()];
	}

	void render::Primitive::FillData(render::DescriptorSet<render::DescriptorSetType::kColor>::Binding<0>::Data& data)
	{
		data.color = material.color;
	}

	//void render::Model::FillData(render::DescriptorSet<render::DescriptorSetType::kSkeleton>::Binding<0>::Data& data)
	//{
	//	
	//}

	//void render::Model::FillData(render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data)
	//{
	//	data.model_mat = node.GetGlobalTransformMatrix();
	//}


	void RenderModel::FillData(render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data)
	{
		if (node)
			data.model_mat = node->GetGlobalTransformMatrix();
		else data.model_mat = glm::identity<glm::mat4>();
	}
}
