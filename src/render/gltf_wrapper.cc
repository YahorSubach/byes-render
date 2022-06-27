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

	bool load_result = loader.LoadBinaryFromFile(&gltf_model, &err, &wrn, path);

	if (load_result)
	{
		std::vector<uint32_t> queue_indices = { device_cfg.graphics_queue_index, device_cfg.transfer_queue_index };

		for (auto&& buffer : gltf_model.buffers)
		{
			buffers_.push_back(GPULocalBuffer(device_cfg, gltf_model.buffers[0].data.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, queue_indices));
			buffers_.back().LoadData(buffer.data.data(), buffer.data.size());
		}

		for (auto&& image : gltf_model.images)
		{
			images_.push_back(Image(device_cfg, VK_FORMAT_R8G8B8A8_SRGB, image.width, image.height, image.image.data()));
			images_views_.push_back(ImageView(device_cfg, images_.back()));
		}

		nodes.resize(gltf_model.nodes.size());

		for (int i = 0; i < gltf_model.nodes.size(); i++)
		{
			auto&& node = gltf_model.nodes[i];

			nodes[i].node_matrix = glm::identity<glm::mat4>();

			if (node.translation.size() == 3)
			{
				nodes[i].node_matrix = glm::translate(nodes[i].node_matrix, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
			}

			if (node.rotation.size() == 4)
			{
				//glm::rotate(glm::mat4(1.0f), (1 * (1.0f + 1 * 1.37f)) * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				nodes.back().node_matrix = glm::rotate(nodes[i].node_matrix, (float)node.rotation[3], glm::vec3(node.rotation[0], node.rotation[1], node.rotation[2]));
				//nodes.back().node_matrix = glm::rotate(nodes.back().node_matrix, 1. glm::vec3());
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

		for (int i = 0; i < gltf_model.nodes.size(); i++)
		{
			auto&& gltf_node = gltf_model.nodes[i];

			if (gltf_node.mesh != -1)
			{

				tinygltf::Mesh& gltf_mesh = gltf_model.meshes[gltf_node.mesh];

				meshes.push_back(Mesh{ nodes[i] });

				for (auto&& gltf_primitive : gltf_mesh.primitives)
				{
					int diffuse_tex_index = gltf_model.materials[gltf_primitive.material].pbrMetallicRoughness.baseColorTexture.index;
					int emit_tex_index = gltf_model.materials[gltf_primitive.material].emissiveTexture.index;

					Mesh::Primitive primitive
					{
						BuildBufferAccessor(gltf_primitive.indices),
						BuildBufferAccessor(gltf_primitive.attributes["POSITION"]),
						BuildBufferAccessor(gltf_primitive.attributes["NORMAL"]),
						BuildBufferAccessor(gltf_primitive.attributes["TEXCOORD_0"]),
						BuildBufferAccessor(gltf_primitive.attributes["JOINTS_0"]),
						BuildBufferAccessor(gltf_primitive.attributes["WEIGHT_0"]),
						emit_tex_index >= 0,
						images_[
							gltf_model.textures[
								diffuse_tex_index >= 0 ? diffuse_tex_index : emit_tex_index
							].source
						],
					};

					meshes.back().primitives.push_back(primitive);
				}

				if(gltf_node.skin != -1)
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
						glm::mat4* mat_ptr = reinterpret_cast<glm::mat4*>( & (gltf_model.buffers[gltf_inverse_matrix_buf_view.buffer].data[offset + actual_stride * i]));
						inverce_matrices.push_back(*mat_ptr);
					}
				}
			}
		}

	}
	else
	{
		valid_ = false;
	}

}


render::BufferAccessor render::GLTFWrapper::BuildBufferAccessor(int acc_ind)
{
	if (acc_ind >= 0)
	{
		auto overriden_stride = gltf_model.bufferViews[gltf_model.accessors[acc_ind].bufferView].byteStride;
		auto element_size = tinygltf::GetNumComponentsInType(gltf_model.accessors[acc_ind].type) * tinygltf::GetComponentSizeInBytes(gltf_model.accessors[acc_ind].componentType);

		auto actual_stride = overriden_stride > 0 ? overriden_stride : element_size;

		auto offset = gltf_model.bufferViews[gltf_model.accessors[acc_ind].bufferView].byteOffset + gltf_model.accessors[acc_ind].byteOffset;

		BufferAccessor buffer_accessor(buffers_[gltf_model.bufferViews[gltf_model.accessors[acc_ind].bufferView].buffer], actual_stride, offset, gltf_model.accessors[acc_ind].count);

		return buffer_accessor;
	}

	return BufferAccessor();
}
