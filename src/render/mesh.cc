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
	namespace primitive
	{
		Geometry::Geometry(const Global& global, DescriptorSetsManager& manager, PrimitiveFlags flags) : GeometryDescriptorSetHolder(global, manager), flags(flags)
		{
		}

		bool Geometry::FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<0>::Data& data)
		{
			data.flags = 0;
			return true;
		}

		bool Geometry::FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<1>::Data& data)
		{
			if (material.albedo)
			{
				SamplerData sampler_data{ *material.albedo, global_.mipmap_cnt_to_global_samplers[material.albedo->GetMipMapLevelsCount()] };
				data.albedo = sampler_data;
			}
			else
			{
				SamplerData sampler_data{ *global_.error_image, global_.mipmap_cnt_to_global_samplers[global_.error_image->GetMipMapLevelsCount()] };
				data.albedo = sampler_data;
			}

			return true;
		}

		bool Geometry::FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<2>::Data& data)
		{
			if (material.metallic_roughness)
			{
				SamplerData sampler_data{ *material.metallic_roughness, global_.mipmap_cnt_to_global_samplers[material.metallic_roughness->GetMipMapLevelsCount()] };
				data.metallic_roughness = sampler_data;
			}
			else
			{
				SamplerData sampler_data{ *global_.error_image, global_.mipmap_cnt_to_global_samplers[global_.error_image->GetMipMapLevelsCount()] };
				data.metallic_roughness = sampler_data;
			}

			return true;
		}

		bool Geometry::FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<3>::Data& data)
		{
			if (material.normal_map)
			{
				SamplerData sampler_data{ *material.normal_map, global_.mipmap_cnt_to_global_samplers[material.normal_map->GetMipMapLevelsCount()] };
				data.normal_map = sampler_data;
			}
			else
			{
				SamplerData sampler_data{ *global_.error_image, global_.mipmap_cnt_to_global_samplers[global_.error_image->GetMipMapLevelsCount()] };
				data.normal_map = sampler_data;
			}

			return true;
		}

		bool Geometry::FillData(render::DescriptorSet<render::DescriptorSetType::kColor>::Binding<0>::Data& data)
		{
			data.color = material.color;
			return true;
		}


		Bitmap::Bitmap(const Global& global, DescriptorSetsManager& manager, PrimitiveFlags flags) : BitmapDescriptorSetHolder(global, manager)
		{
		}

		bool Bitmap::FillData(render::DescriptorSet<render::DescriptorSetType::kBitmapAtlas>::Binding<0>::Data& data)
		{
			return false;
		}

		bool Bitmap::FillData(render::DescriptorSet<render::DescriptorSetType::kTexture>::Binding<0>::Data& data)
		{
			return false;
		}
	}

	//void render::Model::FillData(render::DescriptorSet<render::DescriptorSetType::kSkeleton>::Binding<0>::Data& data)
	//{
	//	
	//}

	//void render::Model::FillData(render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data)
	//{
	//	data.model_mat = node.GetGlobalTransformMatrix();
	//}


	bool RenderModel::FillData(render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data)
	{
		if (node)
			data.model_mat = node->GetGlobalTransformMatrix();
		else data.model_mat = glm::identity<glm::mat4>();

		return true;
	}
}
