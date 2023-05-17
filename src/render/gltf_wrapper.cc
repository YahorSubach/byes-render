#include "gltf_wrapper.h"

#pragma warning(push, 0)
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "tinygltf/tiny_gltf.h"

#undef TINYGLTF_IMPLEMENTATION
#pragma warning(pop)

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

#include "stl_util.h"

#include "global.h"

#include "render/render_setup.h"

render::GLTFWrapper::GLTFWrapper(const Global& global, const tinygltf::Model& gltf_model, DescriptorSetsManager& manager)
{
	

	//}
	//else
	//{
	//	valid_ = false;
	//}

}

int render::GLTFWrapper::GetBufferViewIndexFromAttributes(const std::map<std::string, int>& attributes, VertexBufferType vertex_buffer_type, int index) const
{
	std::string name = GetVertexBufferTypesToNames().at(vertex_buffer_type);

	if (index < 0)
	{
		if (auto&& it = attributes.find(name); it != attributes.end())
		{
			return it->second;
		}
	}
	else
	{
		std::string indexed_name = name + "_" + std::to_string(index);
		if (auto&& it = attributes.find(indexed_name); it != attributes.end())
		{
			return it->second;
		}
	}

	return -1;
}



render::BufferAccessor render::GLTFWrapper::BuildBufferAccessor(const tinygltf::Model& gltf_model, int acc_ind) const
{
	assert(acc_ind >= 0);

	auto overriden_stride = gltf_model.bufferViews[gltf_model.accessors[acc_ind].bufferView].byteStride;
	auto element_size = tinygltf::GetNumComponentsInType(gltf_model.accessors[acc_ind].type) * tinygltf::GetComponentSizeInBytes(gltf_model.accessors[acc_ind].componentType);

	size_t actual_stride = overriden_stride > 0 ? overriden_stride : element_size;

	size_t offset = gltf_model.bufferViews[gltf_model.accessors[acc_ind].bufferView].byteOffset + gltf_model.accessors[acc_ind].byteOffset;

	BufferAccessor buffer_accessor(buffers_[gltf_model.bufferViews[gltf_model.accessors[acc_ind].bufferView].buffer], actual_stride, offset, gltf_model.accessors[acc_ind].count);

	return buffer_accessor;
}
namespace render
{
	ModelPack::ModelPack(const Global& global, DescriptorSetsManager& manager):global_(global), desc_set_manager_(manager)
	{
	}

	std::vector<Model> ModelPack::AddGLTF(const tinygltf::Model& gltf_model)
	{
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string wrn;

		//bool load_result = loader.LoadBinaryFromFile(&gltf_model_, &err, &wrn, path);

	//	if (load_result)
	//	{
		std::vector<uint32_t> queue_indices = { global_.graphics_queue_index, global_.transfer_queue_index };

		for (auto&& buffer : gltf_model.buffers)
		{
			buffers_.push_back(GPULocalBuffer(global_, gltf_model.buffers[0].data.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, queue_indices));
			buffers_.back().LoadData(buffer.data.data(), buffer.data.size());
		}

		for (auto&& image : gltf_model.images)
		{
			auto&& buffer_view = gltf_model.bufferViews[image.bufferView];
			auto&& buffer = gltf_model.buffers[buffer_view.buffer];

			if (image.name.find("albedo") != std::string::npos)
			{
				images_.push_back(Image(global_, VK_FORMAT_R8G8B8A8_UNORM, { u32(image.width), u32(image.height) }, image.image.data()/*, {ImageProperty::kShaderInput, ImageProperty::kMipMap}*/));
			}
			else
			{
				images_.push_back(Image(global_, VK_FORMAT_R8G8B8A8_SRGB, { u32(image.width), u32(image.height) }, image.image.data()/*, {ImageProperty::kShaderInput, ImageProperty::kMipMap}*/));
			}


			for (int i = 0; i < image.height; i++)
			{
				for (int j = 0; j < image.width; j++)
				{
					char r = image.image[4 * (i * image.width + j)];
					char g = image.image[4 * (i * image.width + j) + 1];
					char b = image.image[4 * (i * image.width + j) + 2];
					char a = image.image[4 * (i * image.width + j) + 3];

					if (r != -1 || g != -1 || b != -1)
					{
						int a = 1;
					}
				}
			}



			images_views_.push_back(ImageView(global_, images_.back()));
		}

		std::vector<Node> nodes;

		nodes.resize(gltf_model.nodes.size());

		for (int i = 0; i < gltf_model.nodes.size(); i++)
		{
			auto&& node = gltf_model.nodes[i];

			nodes[i].local_transform = glm::identity<glm::mat4>();


			if (node.translation.size() == 3)
			{
				nodes[i].translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
				nodes[i].local_transform = glm::translate(nodes[i].local_transform, nodes[i].translation);
			}

			nodes[i].rotation = glm::quat(1, 0, 0, 0);
			if (node.rotation.size() == 4)
			{
				//glm::rotate(glm::mat4(1.0f), (1 * (1.0f + 1 * 1.37f)) * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				glm::quat rot_quat = glm::quat(static_cast<float>(node.rotation[3]), static_cast<float>(node.rotation[0]), static_cast<float>(node.rotation[1]), static_cast<float>(node.rotation[2]));

				nodes[i].rotation = rot_quat;

				nodes[i].local_transform = nodes[i].local_transform * glm::mat4_cast(rot_quat);
				//nodes.back().node_matrix = glm::rotate(nodes.back().node_matrix, 1. glm::vec3());
			}

			nodes[i].scale = glm::vec3(1);
			if (node.scale.size() == 3)
			{
				nodes[i].scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
				nodes[i].local_transform = glm::scale(nodes[i].local_transform, nodes[i].scale);
			}
		}

		for (int i = 0; i < gltf_model.nodes.size(); i++)
		{
			auto&& node = gltf_model.nodes[i];
			for (auto&& children_index : node.children)
			{
				nodes[children_index].parent = nodes[i];
			}
		}


		meshes.resize(gltf_model.meshes.size());

		for (int mesh_index = 0; mesh_index < meshes.size(); mesh_index++)
		{
			const tinygltf::Mesh& gltf_mesh = gltf_model.meshes[mesh_index];

			for (auto&& gltf_primitive : gltf_mesh.primitives)
			{
				Primitive primitive(global, manager);
				primitive.category = RenderModelCategory::kRenderModel;
				primitive.material.pipeline_type = PipelineId::kBuildGBuffers;
				primitive.indices.emplace(BuildBufferAccessor(gltf_model, gltf_primitive.indices));

				for (VertexBufferType vertex_buffer_type = VertexBufferType::Begin; vertex_buffer_type != VertexBufferType::End; vertex_buffer_type = util::enums::Next(vertex_buffer_type))
				{
					if (int buffer_view_index = GetBufferViewIndexFromAttributes(gltf_primitive.attributes, vertex_buffer_type); buffer_view_index >= 0)
					{
						primitive.vertex_buffers[u32(vertex_buffer_type)].emplace(BuildBufferAccessor(gltf_model, buffer_view_index));
					}
					else if (buffer_view_index = GetBufferViewIndexFromAttributes(gltf_primitive.attributes, vertex_buffer_type, 0); buffer_view_index >= 0)
					{
						primitive.vertex_buffers[u32(vertex_buffer_type)].emplace(BuildBufferAccessor(gltf_model, buffer_view_index));
					}
				}

				if (gltf_primitive.material >= 0)
				{
					tinygltf::Material gltf_material = gltf_model.materials[gltf_primitive.material];

					if (gltf_material.pbrMetallicRoughness.baseColorTexture.index >= 0)
					{
						primitive.material.albedo = images_[gltf_model.textures[gltf_material.pbrMetallicRoughness.baseColorTexture.index].source];
					}

					if (gltf_material.emissiveTexture.index >= 0)
					{
						primitive.material.albedo = images_[gltf_model.textures[gltf_material.emissiveTexture.index].source];
						primitive.material.flags |= (1 << 0);
					}

					if (gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0)
					{
						primitive.material.metallic_roughness = images_[gltf_model.textures[gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index].source];
					}

					if (gltf_material.normalTexture.index >= 0)
					{
						primitive.material.normal_map = images_[gltf_model.textures[gltf_material.normalTexture.index].source];
					}
				}

				//auto&& tmp = *(primitive.positions.buffer);

				//if (primitive.positions.buffer) primitive.vertex_buffers.push_back(primitive.positions);
				//if (primitive.normals.buffer) primitive.vertex_buffers.push_back(primitive.normals);
				//if (primitive.tangents.buffer)
				//{
				//	primitive.vertex_buffers.push_back(primitive.tangents);
				//	primitive.material.flags |= (1 << 1);
				//}
				//else primitive.vertex_buffers.push_back(primitive.normals);
				//if (primitive.tex_coords.buffer) primitive.vertex_buffers.push_back(primitive.tex_coords);
				//if (primitive.joints.buffer) { primitive.vertex_buffers.push_back(primitive.joints);  primitive.type = RenderModelType::kSkinned; }
				//if (primitive.weights.buffer) primitive.vertex_buffers.push_back(primitive.weights);


				meshes.back().primitives.push_back(std::move(primitive));
			}

		}

		skins.resize(gltf_model.skins.size());

		for (int i = 0; i < gltf_model.nodes.size(); i++)
		{
			auto&& gltf_node = gltf_model.nodes[i];
			auto&& node = nodes[i];

			if (gltf_node.mesh != -1)
			{
				models.push_back(Model(global, manager, node, meshes[gltf_node.mesh]));

				const tinygltf::Mesh& gltf_mesh = gltf_model.meshes[gltf_node.mesh];


				if (gltf_node.skin != -1)
				{
					auto&& gltf_skin = gltf_model.skins[gltf_node.skin];
					auto&& gltf_inverse_matrix_acc = gltf_model.accessors[gltf_skin.inverseBindMatrices];
					auto&& gltf_inverse_matrix_buf_view = gltf_model.bufferViews[gltf_inverse_matrix_acc.bufferView];

					auto overriden_stride = gltf_inverse_matrix_buf_view.byteStride;
					auto element_size = tinygltf::GetNumComponentsInType(gltf_inverse_matrix_acc.type) * tinygltf::GetComponentSizeInBytes(gltf_inverse_matrix_acc.componentType);

					auto actual_stride = overriden_stride > 0 ? overriden_stride : element_size;
					auto offset = gltf_inverse_matrix_buf_view.byteOffset + gltf_inverse_matrix_acc.byteOffset;

					assert(tinygltf::GetNumComponentsInType(gltf_inverse_matrix_acc.type) == 4 * 4);
					assert(gltf_inverse_matrix_acc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

					std::vector<glm::mat4> inverce_matrices;

					for (int i = 0; i < gltf_inverse_matrix_acc.count; i++)
					{
						const glm::mat4* mat_ptr = reinterpret_cast<const glm::mat4*>(&(gltf_model.buffers[gltf_inverse_matrix_buf_view.buffer].data[offset + actual_stride * i]));
						inverce_matrices.push_back(*mat_ptr);
					}

					for (int i = 0; i < gltf_skin.joints.size(); i++)
					{
						skins[gltf_node.skin].joints.push_back(Joint{ nodes[gltf_skin.joints[i]], inverce_matrices[i] });
					}
				}
			}


			for (auto&& anim : gltf_model.animations)
			{
				Animation animation;

				for (int channel_ind = 0; channel_ind < anim.channels.size(); channel_ind++)
				{
					int sampler_ind = anim.channels[channel_ind].sampler;

					std::span<const float> times = BuildVectorFromAccessorIndex<float>(gltf_model, anim.samplers[sampler_ind].input);

					if (anim.channels[channel_ind].target_path == "translation")
					{
						std::span<const glm::vec3> values = BuildVectorFromAccessorIndex<glm::vec3>(gltf_model, anim.samplers[sampler_ind].output);
						AnimSampler<glm::vec3> sampler;
						sampler.node_index = anim.channels[channel_ind].target_node;
						sampler.interpolation_type = anim.samplers[sampler_ind].interpolation == "CUBICSPLINE" ? InterpolationType::kCubicSpline : InterpolationType::kLinear;

						for (int i = 0; i < values.size(); i++)
						{
							int ms_time = static_cast<int>(sampler.interpolation_type == InterpolationType::kCubicSpline ? times[i / 3] : times[i]) * 1000;
							sampler.frames.push_back({ std::chrono::milliseconds(ms_time), values[i] });
						}

						animation.translations.push_back(sampler);
					}
					else if (anim.channels[channel_ind].target_path == "scale")
					{
						std::span<const glm::vec3> values = BuildVectorFromAccessorIndex<glm::vec3>(gltf_model, anim.samplers[sampler_ind].output);
						AnimSampler<glm::vec3> sampler;
						sampler.node_index = anim.channels[channel_ind].target_node;
						sampler.interpolation_type = anim.samplers[sampler_ind].interpolation == "CUBICSPLINE" ? InterpolationType::kCubicSpline : InterpolationType::kLinear;

						for (int i = 0; i < values.size(); i++)
						{
							int ms_time = static_cast<int>(sampler.interpolation_type == InterpolationType::kCubicSpline ? times[i / 3] : times[i]) * 1000;
							sampler.frames.push_back({ std::chrono::milliseconds(ms_time), values[i] });
						}

						animation.scales.push_back(sampler);
					}
					else if (anim.channels[channel_ind].target_path == "rotation")
					{
						std::span<const glm::vec4> values = BuildVectorFromAccessorIndex<glm::vec4>(gltf_model, anim.samplers[sampler_ind].output);
						AnimSampler<glm::quat> sampler;
						sampler.node_index = anim.channels[channel_ind].target_node;
						sampler.interpolation_type = anim.samplers[sampler_ind].interpolation == "CUBICSPLINE" ? InterpolationType::kCubicSpline : InterpolationType::kLinear;

						for (int i = 0; i < values.size(); i++)
						{
							int ms_time = static_cast<int>(sampler.interpolation_type == InterpolationType::kCubicSpline ? times[i / 3] : times[i]) * 1000;
							sampler.frames.push_back({ std::chrono::milliseconds(ms_time), glm::quat(values[i].w,values[i].x,values[i].y,values[i].z) });
						}

						animation.rotations.push_back(sampler);
					}
				}

				animations.emplace(anim.name, animation);
			}
		}
	}
}