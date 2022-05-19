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

	bool load_result = loader.LoadBinaryFromFile(&model, &err, &wrn, path);

	if (load_result)
	{
		std::vector<uint32_t> queue_indices = { device_cfg.graphics_queue_index, device_cfg.transfer_queue_index };

		for (auto&& buffer : model.buffers)
		{
			buffers_.push_back(GPULocalBuffer(device_cfg, model.buffers[0].data.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, queue_indices));
			buffers_.back().LoadData(buffer.data.data(), buffer.data.size());
		}

		for (auto&& image : model.images)
		{
			images_.push_back(Image(device_cfg, VK_FORMAT_R8G8B8A8_SRGB, image.width, image.height, image.image.data()));
			images_views_.push_back(ImageView(device_cfg, images_.back()));
		}

		for (auto&& node : model.nodes)
		{
			nodes.push_back(Node());

			nodes.back().node_matrix = glm::identity<glm::mat4>();

			if (node.translation.size() == 3)
			{
				nodes.back().node_matrix = glm::translate(nodes.back().node_matrix, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
			}

			if (node.rotation.size() == 4)
			{
				//glm::rotate(glm::mat4(1.0f), (1 * (1.0f + 1 * 1.37f)) * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				nodes.back().node_matrix = glm::rotate(nodes.back().node_matrix, (float)node.rotation[3], glm::vec3(node.rotation[0], node.rotation[1], node.rotation[2]));
				//nodes.back().node_matrix = glm::rotate(nodes.back().node_matrix, 1. glm::vec3());
			}


			tinygltf::Mesh& mesh = model.meshes[node.mesh];

			
			nodes.back().mesh = Mesh();

			for (auto&& gltf_primitive : mesh.primitives)
			{
				Mesh::Primitive primitive
				{
					BuildBufferAccessor(gltf_primitive.indices),
					BuildBufferAccessor(gltf_primitive.attributes["POSITION"]),
					BuildBufferAccessor(gltf_primitive.attributes["NORMAL"]),
					BuildBufferAccessor(gltf_primitive.attributes["TEXCOORD_0"]),
					images_[
						model.textures[
							model.materials[gltf_primitive.material].pbrMetallicRoughness.baseColorTexture.index
						].source
					],
				};

				nodes.back().mesh.primitives.push_back(primitive);
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
	auto overriden_stride = model.bufferViews[model.accessors[acc_ind].bufferView].byteStride;
	auto element_size = tinygltf::GetNumComponentsInType(model.accessors[acc_ind].type) * tinygltf::GetComponentSizeInBytes(model.accessors[acc_ind].componentType);

	auto actual_stride = overriden_stride > 0 ? overriden_stride : element_size;

	auto offset = model.bufferViews[model.accessors[acc_ind].bufferView].byteOffset + model.accessors[acc_ind].byteOffset;

	BufferAccessor buffer_accessor(buffers_[model.bufferViews[model.accessors[acc_ind].bufferView].buffer], actual_stride, offset, model.accessors[acc_ind].count);

	return buffer_accessor;
}
