#ifndef RENDER_ENGINE_GLTFWRAPPER_H_
#define RENDER_ENGINE_GLTFWRAPPER_H_

#include <string>
#include <vector>
#include <span>
#include <optional>

#pragma warning(push, 0)
#include "tinygltf/tiny_gltf.h"
#pragma warning(pop)

#include "common.h"
#include "data_types.h"
#include "mesh.h"
#include "render/buffer.h"
#include "render/image.h"
#include "render/image_view.h"
#include "render/vertex_buffer.h"

namespace render
{
	class ModelPack
	{
	public:
		ModelPack(const Global& global, DescriptorSetsManager& manager);
		ModelPack(const ModelPack&) = delete;
		ModelPack(ModelPack&&) = default;

		void AddGLTF(const tinygltf::Model& gltf_model);
		void AddSimpleMesh(const std::vector<glm::vec3>& faces, PrimitiveFlags primitive_flags);

		std::vector<Node> nodes;
		std::unordered_map<std::string, Model> models;
		std::vector<Mesh> meshes;
		std::vector<Skin> skins;

		std::map<std::string, Animation> animations;

	private:
		const Global& global_;
		DescriptorSetsManager& desc_set_manager_;

		std::vector<GPULocalBuffer> buffers_;
		std::vector<Image> images_;
		std::vector<ImageView> images_views_;



		int GetBufferViewIndexFromAttributes(const std::map<std::string, int>& attributes, VertexBufferType vertex_buffer_type, int index = -1) const;
		BufferAccessor BuildBufferAccessor(const tinygltf::Model& gltf_model, int acc_ind) const;
	};




	class RenderSetup;
	class GLTFWrapper: ValidationBase
	{
	public:

		GLTFWrapper(const Global& global, const tinygltf::Model& gltf_model, DescriptorSetsManager& manager);
		GLTFWrapper(const GLTFWrapper&) = delete;
		GLTFWrapper(GLTFWrapper&&) = default;


		std::vector<Node> nodes;


		std::vector<Model> models;


		
	private:


		



	};
}

#endif  // RENDER_ENGINE_GLTFWRAPPER_H_