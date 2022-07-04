#include "gltf_wrapper.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "tinygltf/tiny_gltf.h"

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

render::GLTFWrapper::GLTFWrapper(const DeviceConfiguration& device_cfg, const std::string& path)
{
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string wrn;

	bool load_result = loader.LoadBinaryFromFile(&gltf_model_, &err, &wrn, path);

	if (load_result)
	{
		std::vector<uint32_t> queue_indices = { device_cfg.graphics_queue_index, device_cfg.transfer_queue_index };

		for (auto&& buffer : gltf_model_.buffers)
		{
			buffers_.push_back(GPULocalBuffer(device_cfg, gltf_model_.buffers[0].data.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, queue_indices));
			buffers_.back().LoadData(buffer.data.data(), buffer.data.size());
		}

		for (auto&& image : gltf_model_.images)
		{
			images_.push_back(Image(device_cfg, VK_FORMAT_R8G8B8A8_SRGB, image.width, image.height, image.image.data()));
			images_views_.push_back(ImageView(device_cfg, images_.back()));
		}

		nodes.resize(gltf_model_.nodes.size());

		for (int i = 0; i < gltf_model_.nodes.size(); i++)
		{
			auto&& node = gltf_model_.nodes[i];

			nodes[i].node_matrix = glm::identity<glm::mat4>();


			if (node.translation.size() == 3)
			{
				nodes[i].translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
				nodes[i].node_matrix = glm::translate(nodes[i].node_matrix, nodes[i].translation);
			}

			nodes[i].rotation = glm::quat(1, 0, 0, 0);
			if (node.rotation.size() == 4)
			{
				//glm::rotate(glm::mat4(1.0f), (1 * (1.0f + 1 * 1.37f)) * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				glm::quat rot_quat = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);

				nodes[i].rotation = rot_quat;

				nodes[i].node_matrix = nodes[i].node_matrix * glm::mat4_cast(rot_quat);
				//nodes.back().node_matrix = glm::rotate(nodes.back().node_matrix, 1. glm::vec3());
			}

			nodes[i].scale = glm::vec3(1);
			if (node.scale.size() == 3)
			{
				nodes[i].scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
				nodes[i].node_matrix = glm::scale(nodes[i].node_matrix, nodes[i].scale);
			}
		}

		for (int i = 0; i < gltf_model_.nodes.size(); i++)
		{
			auto&& node = gltf_model_.nodes[i];
			for (auto&& children_index : node.children)
			{
				nodes[children_index].parent = nodes[i];
			}
		}

		for (int i = 0; i < gltf_model_.nodes.size(); i++)
		{
			auto&& gltf_node = gltf_model_.nodes[i];

			if (gltf_node.mesh != -1)
			{

				tinygltf::Mesh& gltf_mesh = gltf_model_.meshes[gltf_node.mesh];

				meshes.push_back(Mesh{ nodes[i] });

				for (auto&& gltf_primitive : gltf_mesh.primitives)
				{


					Primitive primitive
					{
						BuildBufferAccessor(gltf_primitive.indices),
						BuildBufferAccessor(GetBufferViewIndexFromAttributes(gltf_primitive.attributes, "POSITION")),
						BuildBufferAccessor(GetBufferViewIndexFromAttributes(gltf_primitive.attributes, "NORMAL")),
						BuildBufferAccessor(GetBufferViewIndexFromAttributes(gltf_primitive.attributes, "TEXCOORD_0")),
						BuildBufferAccessor(GetBufferViewIndexFromAttributes(gltf_primitive.attributes, "JOINTS_0")),
						BuildBufferAccessor(GetBufferViewIndexFromAttributes(gltf_primitive.attributes, "WEIGHTS_0")),
					};

					if (gltf_primitive.material >= 0)
					{

						int diffuse_tex_index = gltf_model_.materials[gltf_primitive.material].pbrMetallicRoughness.baseColorTexture.index;
						int emit_tex_index = gltf_model_.materials[gltf_primitive.material].emissiveTexture.index;

						primitive.emit = emit_tex_index >= 0;
						primitive.color_tex = images_[
							gltf_model_.textures[
								diffuse_tex_index >= 0 ? diffuse_tex_index : emit_tex_index
							].source];
					}

					auto&& tmp = *(primitive.positions.buffer);

					if (primitive.positions.buffer) primitive.vertex_buffers.push_back(primitive.positions);
					if (primitive.normals.buffer) primitive.vertex_buffers.push_back(primitive.normals);
					if (primitive.tex_coords.buffer) primitive.vertex_buffers.push_back(primitive.tex_coords);
					if (primitive.joints.buffer) { primitive.vertex_buffers.push_back(primitive.joints);  primitive.type = RenderModelType::kSkinned; }
					if (primitive.weights.buffer) primitive.vertex_buffers.push_back(primitive.weights);


					meshes.back().primitives.push_back(primitive);
				}

				if(gltf_node.skin != -1)
				{
					auto&& gltf_skin = gltf_model_.skins[gltf_node.skin];
					auto&& gltf_inverse_matrix_acc = gltf_model_.accessors[gltf_skin.inverseBindMatrices];
					auto&& gltf_inverse_matrix_buf_view = gltf_model_.bufferViews[gltf_inverse_matrix_acc.bufferView];

					auto overriden_stride = gltf_inverse_matrix_buf_view.byteStride;
					auto element_size = tinygltf::GetNumComponentsInType(gltf_inverse_matrix_acc.type) * tinygltf::GetComponentSizeInBytes(gltf_inverse_matrix_acc.componentType);

					auto actual_stride = overriden_stride > 0 ? overriden_stride : element_size;
					auto offset = gltf_inverse_matrix_buf_view.byteOffset + gltf_inverse_matrix_acc.byteOffset;

					assert(tinygltf::GetNumComponentsInType(gltf_inverse_matrix_acc.type) == 4 * 4);
					assert(gltf_inverse_matrix_acc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

					std::vector<glm::mat4> inverce_matrices;

					for (int i = 0; i < gltf_inverse_matrix_acc.count; i++)
					{
						glm::mat4* mat_ptr = reinterpret_cast<glm::mat4*>( & (gltf_model_.buffers[gltf_inverse_matrix_buf_view.buffer].data[offset + actual_stride * i]));
						inverce_matrices.push_back(*mat_ptr);
					}

					for (int i = 0; i < gltf_skin.joints.size(); i++)
					{
						meshes.back().joints.push_back(Bone{ nodes[gltf_skin.joints[i]], inverce_matrices[i] });
					}
				}

				for (auto&& anim : gltf_model_.animations)
				{
					Animation animation;

					for (int channel_ind = 0;  channel_ind < anim.channels.size(); channel_ind++)
					{
						int sampler_ind = anim.channels[channel_ind].sampler;

						std::vector<float> times = BuildVectorFromAccessorIndex<float>(anim.samplers[sampler_ind].input);

						if (anim.channels[channel_ind].target_path == "translation")
						{
							std::vector<glm::vec3> values = BuildVectorFromAccessorIndex<glm::vec3>(anim.samplers[sampler_ind].output);
							AnimSampler<glm::vec3> sampler;
							sampler.node_index = anim.channels[channel_ind].target_node;
							sampler.interpolation_type = anim.samplers[sampler_ind].interpolation == "CUBICSPLINE" ? InterpolationType::kCubicSpline : InterpolationType::kLinear;

							for (int i = 0; i < values.size(); i++)
							{
								int ms_time = (sampler.interpolation_type == InterpolationType::kCubicSpline ? times[i / 3] : times[i]) * 1000;
								sampler.frames.push_back({ std::chrono::milliseconds(ms_time), values[i] });
							}

							animation.translations.push_back(sampler);
						}
						else if (anim.channels[channel_ind].target_path == "scale")
						{
							std::vector<glm::vec3> values = BuildVectorFromAccessorIndex<glm::vec3>(anim.samplers[sampler_ind].output);
							AnimSampler<glm::vec3> sampler;
							sampler.node_index = anim.channels[channel_ind].target_node;
							sampler.interpolation_type = anim.samplers[sampler_ind].interpolation == "CUBICSPLINE" ? InterpolationType::kCubicSpline : InterpolationType::kLinear;

							for (int i = 0; i < values.size(); i++)
							{
								int ms_time = (sampler.interpolation_type == InterpolationType::kCubicSpline ? times[i / 3] : times[i]) * 1000;
								sampler.frames.push_back({ std::chrono::milliseconds(ms_time), values[i] });
							}

							animation.scales.push_back(sampler);
						}
						else if (anim.channels[channel_ind].target_path == "rotation")
						{
							std::vector<glm::vec4> values = BuildVectorFromAccessorIndex<glm::vec4>(anim.samplers[sampler_ind].output);
							AnimSampler<glm::quat> sampler;
							sampler.node_index = anim.channels[channel_ind].target_node;
							sampler.interpolation_type = anim.samplers[sampler_ind].interpolation == "CUBICSPLINE" ? InterpolationType::kCubicSpline : InterpolationType::kLinear;

							for (int i = 0; i < values.size(); i++)
							{
								int ms_time = (sampler.interpolation_type == InterpolationType::kCubicSpline ? times[i / 3] : times[i]) * 1000;
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
	else
	{
		valid_ = false;
	}

}

int render::GLTFWrapper::GetBufferViewIndexFromAttributes(const std::map<std::string, int>& attributes, const std::string& name) const
{
	if (auto&& it = attributes.find(name); it != attributes.end())
	{
		return it->second;
	}

	return -1;
}


render::BufferAccessor render::GLTFWrapper::BuildBufferAccessor(int acc_ind) const
{
	if (acc_ind >= 0)
	{
		auto overriden_stride = gltf_model_.bufferViews[gltf_model_.accessors[acc_ind].bufferView].byteStride;
		auto element_size = tinygltf::GetNumComponentsInType(gltf_model_.accessors[acc_ind].type) * tinygltf::GetComponentSizeInBytes(gltf_model_.accessors[acc_ind].componentType);

		auto actual_stride = overriden_stride > 0 ? overriden_stride : element_size;

		auto offset = gltf_model_.bufferViews[gltf_model_.accessors[acc_ind].bufferView].byteOffset + gltf_model_.accessors[acc_ind].byteOffset;

		BufferAccessor buffer_accessor(buffers_[gltf_model_.bufferViews[gltf_model_.accessors[acc_ind].bufferView].buffer], actual_stride, offset, gltf_model_.accessors[acc_ind].count);

		return buffer_accessor;
	}

	return BufferAccessor();
}
