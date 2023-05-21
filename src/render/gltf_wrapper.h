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
		std::vector<Model> models;
		std::vector<Mesh> meshes;
		std::vector<Skin> skins;

		std::map<std::string, Animation> animations;

	private:
		const Global& global_;
		DescriptorSetsManager& desc_set_manager_;

		std::vector<GPULocalBuffer> buffers_;
		std::vector<Image> images_;
		std::vector<ImageView> images_views_;

		template<typename ElementType>
		static std::span<const ElementType> BuildVectorFromAccessorIndex(const tinygltf::Model& gltf_model, int acc_ind)
		{
			auto&& buffer_acc = gltf_model.accessors[acc_ind];
			auto&& buffer_view = gltf_model.bufferViews[buffer_acc.bufferView];

			auto overriden_stride = buffer_view.byteStride;
			auto element_size = tinygltf::GetNumComponentsInType(buffer_acc.type) * tinygltf::GetComponentSizeInBytes(buffer_acc.componentType);

			assert(sizeof(ElementType) == element_size);

			auto actual_stride = overriden_stride > 0 ? overriden_stride : element_size;

			auto offset = buffer_view.byteOffset + buffer_acc.byteOffset;


			const ElementType* begin = reinterpret_cast<const ElementType*>(gltf_model.buffers[buffer_view.buffer].data.data() + offset);
			const ElementType* end = begin + (buffer_acc.count);

			std::span<const ElementType> result(begin, end);

			return result;
		}

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