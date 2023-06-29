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







namespace render
{
	ModelPack::ModelPack(const Global& global, DescriptorSetsManager& manager):global_(global), desc_set_manager_(manager)
	{
	}

	void ModelPack::AddGLTF(const tinygltf::Model& gltf_model)
	{
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string wrn;

		//bool load_result = loader.LoadBinaryFromFile(&gltf_model_, &err, &wrn, path);

	//	if (load_result)
	//	{
		std::vector<uint32_t> queue_indices = { global_.graphics_queue_index, global_.transfer_queue_index };

		buffers_.reserve(16);

		for (auto&& buffer : gltf_model.buffers)
		{
			buffers_.push_back(GPULocalBuffer(global_, buffer.data.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, queue_indices));
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

		std::vector<short> index_to_parent;

		nodes.resize(gltf_model.nodes.size());
		index_to_parent.resize(gltf_model.nodes.size(), -1);

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
				index_to_parent[children_index] = i;
			}
		}


		meshes.resize(gltf_model.meshes.size());

		for (int mesh_index = 0; mesh_index < meshes.size(); mesh_index++)
		{
			const tinygltf::Mesh& gltf_mesh = gltf_model.meshes[mesh_index];

			for (auto&& gltf_primitive : gltf_mesh.primitives)
			{
				Primitive primitive(global_, desc_set_manager_, RenderModelCategory::kRenderModel);
				primitive.material.pipeline_type = PipelineId::kBuildGBuffers;
				primitive.indices.emplace(BuildBufferAccessor(gltf_model, gltf_primitive.indices));

				std::array<int, u32(VertexBufferType::Count)> attribute_accessor_indices{};

				for (VertexBufferType vertex_buffer_type = VertexBufferType::Begin; vertex_buffer_type != VertexBufferType::End; vertex_buffer_type = util::enums::Next(vertex_buffer_type))
				{
					if (int buffer_acc_index = GetBufferViewIndexFromAttributes(gltf_primitive.attributes, vertex_buffer_type); buffer_acc_index >= 0)
					{
						attribute_accessor_indices[u32(vertex_buffer_type)] = buffer_acc_index;
						primitive.vertex_buffers[u32(vertex_buffer_type)].emplace(BuildBufferAccessor(gltf_model, buffer_acc_index));
					}
					else if (buffer_acc_index = GetBufferViewIndexFromAttributes(gltf_primitive.attributes, vertex_buffer_type, 0); buffer_acc_index >= 0)
					{
						attribute_accessor_indices[u32(vertex_buffer_type)] = buffer_acc_index;
						primitive.vertex_buffers[u32(vertex_buffer_type)].emplace(BuildBufferAccessor(gltf_model, buffer_acc_index));
					}
					else attribute_accessor_indices[u32(vertex_buffer_type)] = -1;
				}

				if (attribute_accessor_indices[u32(VertexBufferType::kPOSITION)] >= 0 && attribute_accessor_indices[u32(VertexBufferType::kTEXCOORD)] >= 0
					&& attribute_accessor_indices[u32(VertexBufferType::kNORMAL)] >= 0 && attribute_accessor_indices[u32(VertexBufferType::kTANGENT)] < 0)
				{
					std::span<const short> indices = GetBufferSpanByAccessor<short>(gltf_model, gltf_primitive.indices);
					std::span<const glm::vec3> positions = GetBufferSpanByAccessor<glm::vec3>(gltf_model, attribute_accessor_indices[u32(VertexBufferType::kPOSITION)]);
					std::span<const glm::vec2> uvs = GetBufferSpanByAccessor<glm::vec2>(gltf_model, attribute_accessor_indices[u32(VertexBufferType::kTEXCOORD)]);
					std::span<const glm::vec3> normals = GetBufferSpanByAccessor<glm::vec3>(gltf_model, attribute_accessor_indices[u32(VertexBufferType::kNORMAL)]);

					assert(normals.size() == positions.size());

					std::vector<glm::vec3> tangents(normals.size());

					for (auto it = indices.begin(); it != indices.end();)
					{
						std::array<short, 3> tri_verts_indices;
						std::array<glm::vec3, 3> tri_verts;
						std::array<glm::vec2, 3> tri_uvs;

						for (int i = 0; i < 3; i++)
						{
							tri_verts_indices[i] = *it++;
							tri_verts[i] = positions[tri_verts_indices[i]];
							tri_uvs[i] = uvs[tri_verts_indices[i]];
						}

						for (int i = 0; i < 3; i++)
						{							
							short ind_0 = i;
							short ind_1 = (i + 1) % 3;
							short ind_2 = (i + 2) % 3;

							glm::vec2 duv1 = tri_uvs[ind_1] - tri_uvs[ind_0];
							glm::vec2 duv2 = tri_uvs[ind_2] - tri_uvs[ind_0];

							glm::vec3 dpos1 = tri_verts[ind_1] - tri_verts[ind_0];
							glm::vec3 dpos2 = tri_verts[ind_2] - tri_verts[ind_0];

							float u1v2_minus_u2v1 = duv1.x * duv2.y - duv2.x * duv1.y;

							assert(fabs(u1v2_minus_u2v1) > glm::epsilon<float>());

							float a = duv2.y / u1v2_minus_u2v1;
							float b = - duv1.y / u1v2_minus_u2v1;

							float test_u = a * duv1.x + b * duv2.x;
							float test_v = a * duv1.y + b * duv2.y;

							glm::vec3 tangent = glm::normalize(a * dpos1 + b * dpos2);
							tangents[tri_verts_indices[ind_0]] = tangent;
						}

					}

					buffers_.push_back(GPULocalBuffer(global_, tangents.size()*sizeof(glm::vec3), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, queue_indices));
					buffers_.back().LoadData(tangents.data(), tangents.size() * sizeof(glm::vec3));

					primitive.vertex_buffers[u32(VertexBufferType::kTANGENT)].emplace(BufferAccessor(ElemType<glm::vec3>{}, buffers_.back()));
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


				meshes[mesh_index].primitives.push_back(std::move(primitive));
				meshes[mesh_index].name = gltf_mesh.name;
			}

		}

		skins.resize(gltf_model.skins.size());

		for (int i = 0; i < gltf_model.nodes.size(); i++)
		{
			auto&& gltf_node = gltf_model.nodes[i];
			auto&& node = nodes[i];

			if (gltf_node.mesh != -1)
			{
				const tinygltf::Mesh& gltf_mesh = gltf_model.meshes[gltf_node.mesh];

				models.emplace(gltf_node.name, Model{ node, meshes[gltf_node.mesh] });

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

					std::map<short, short> node_ind_to_skin_ind;
					
					for (int i = 0; i < gltf_skin.joints.size(); i++)
					{
						node_ind_to_skin_ind[gltf_skin.joints[i]] = i;
					}

					for (int i = 0; i < gltf_skin.joints.size(); i++)
					{
						skins[gltf_node.skin].nodes.push_back(nodes[gltf_skin.joints[i]]);
						skins[gltf_node.skin].inverse_bind_matrices.push_back(inverce_matrices[i]);
						skins[gltf_node.skin].parent_indices.push_back(node_ind_to_skin_ind[index_to_parent[gltf_skin.joints[i]]]);
					}

					for (int i = 0; i < skins[gltf_node.skin].nodes.size(); i++)
					{
						if (skins[gltf_node.skin].parent_indices[i] != -1)
						{
							skins[gltf_node.skin].nodes[i].parent = skins[gltf_node.skin].nodes[skins[gltf_node.skin].parent_indices[i]];
						}
					}
				}
			}


			for (auto&& anim : gltf_model.animations)
			{
				Animation animation;

				for (int channel_ind = 0; channel_ind < anim.channels.size(); channel_ind++)
				{
					int sampler_ind = anim.channels[channel_ind].sampler;

					std::span<const float> times = GetBufferSpanByAccessor<float>(gltf_model, anim.samplers[sampler_ind].input);

					if (anim.channels[channel_ind].target_path == "translation")
					{
						std::span<const glm::vec3> values = GetBufferSpanByAccessor<glm::vec3>(gltf_model, anim.samplers[sampler_ind].output);
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
						std::span<const glm::vec3> values = GetBufferSpanByAccessor<glm::vec3>(gltf_model, anim.samplers[sampler_ind].output);
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
						std::span<const glm::vec4> values = GetBufferSpanByAccessor<glm::vec4>(gltf_model, anim.samplers[sampler_ind].output);
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

	void ModelPack::AddSimpleMesh(const std::vector<glm::vec3>& faces)
	{
		std::vector<uint32_t> queue_indices = { global_.graphics_queue_index, global_.transfer_queue_index };

		buffers_.push_back(GPULocalBuffer(global_, faces.size() * sizeof(glm::vec3), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, queue_indices));
		buffers_.back().LoadData(faces.data(), faces.size() * sizeof(glm::vec3));

		Mesh mesh;

		Primitive primitive(global_, desc_set_manager_, RenderModelCategory::kUIShape);
		primitive.material.pipeline_type = PipelineId::kPos;
		primitive.vertex_buffers[u32(VertexBufferType::kPOSITION)].emplace(BufferAccessor(buffers_.back(), sizeof(glm::vec3), 0, faces.size()));
		mesh.primitives.push_back(std::move(primitive));
		meshes.push_back(std::move(mesh));
	}

	BufferAccessor ModelPack::BuildBufferAccessor(const tinygltf::Model& gltf_model, int acc_ind) const
	{
		assert(acc_ind >= 0);

		auto overriden_stride = gltf_model.bufferViews[gltf_model.accessors[acc_ind].bufferView].byteStride;
		auto element_size = tinygltf::GetNumComponentsInType(gltf_model.accessors[acc_ind].type) * tinygltf::GetComponentSizeInBytes(gltf_model.accessors[acc_ind].componentType);

		size_t actual_stride = overriden_stride > 0 ? overriden_stride : element_size;

		size_t offset = gltf_model.bufferViews[gltf_model.accessors[acc_ind].bufferView].byteOffset + gltf_model.accessors[acc_ind].byteOffset;

		BufferAccessor buffer_accessor(buffers_[gltf_model.bufferViews[gltf_model.accessors[acc_ind].bufferView].buffer], actual_stride, offset, gltf_model.accessors[acc_ind].count);

		return buffer_accessor;
	}

	int ModelPack::GetBufferViewIndexFromAttributes(const std::map<std::string, int>& attributes, VertexBufferType vertex_buffer_type, int index) const
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
}