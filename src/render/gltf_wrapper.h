#ifndef RENDER_ENGINE_GLTFWRAPPER_H_
#define RENDER_ENGINE_GLTFWRAPPER_H_

#include <string>
#include <vector>
#include <optional>

#include "tinygltf/tiny_gltf.h"

#include "common.h"
#include "data_types.h"
#include "mesh.h"
#include "render/buffer.h"
#include "render/image.h"
#include "render/image_view.h"

namespace render
{
	class GLTFWrapper: ValidationBase
	{
	public:

		GLTFWrapper(const DeviceConfiguration& device_cfg, const std::string& path);

		std::vector<Node> nodes;
		std::vector<Mesh> meshes;
		
	private:

		int GetBufferViewIndexFromAttributes(const std::map<std::string, int>& attributes, const std::string& name) const;
		BufferAccessor BuildBufferAccessor(int acc_ind) const;
		
		template<typename ElementType>
		std::vector<ElementType> BuildVectorFromAccessorIndex(int acc_ind) const
		{
			auto&& buffer_acc = gltf_model_.accessors[acc_ind];
			auto&& buffer_view = gltf_model_.bufferViews[buffer_acc.bufferView];

			auto overriden_stride = buffer_view.byteStride;
			auto element_size = tinygltf::GetNumComponentsInType(buffer_acc.type) * tinygltf::GetComponentSizeInBytes(buffer_acc.componentType);

			assert(sizeof(ElementType) == element_size);

			auto actual_stride = overriden_stride > 0 ? overriden_stride : element_size;

			auto offset = buffer_view.byteOffset + buffer_acc.byteOffset;


			const ElementType* begin = reinterpret_cast<const ElementType*>(gltf_model_.buffers[buffer_view.buffer].data.data() + offset);
			const ElementType* end = begin + (buffer_acc.count);

			std::vector<ElementType> result(begin, end);

			return result;
		}


		tinygltf::Model gltf_model_;

		std::vector<GPULocalBuffer> buffers_;
		std::vector<Image> images_;
		std::vector<ImageView> images_views_;
	};
}

#endif  // RENDER_ENGINE_GLTFWRAPPER_H_