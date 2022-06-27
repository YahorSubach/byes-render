#ifndef RENDER_ENGINE_GLTFWRAPPER_H_
#define RENDER_ENGINE_GLTFWRAPPER_H_

#include <string>
#include <vector>
#include <optional>

#include "tinygltf/tiny_gltf.h"

#include "common.h"
#include "render/buffer.h"
#include "render/image.h"
#include "render/image_view.h"

namespace render
{
	class GLTFWrapper: ValidationBase
	{
	public:

		struct Node
		{
			glm::mat4 node_matrix;
			//std::map<int, Node&> children;

			std::optional<std::reference_wrapper<Node>> parent;
		};

		struct Mesh
		{
			Node& node;

			struct Primitive
			{
				BufferAccessor indices;
				BufferAccessor positions;
				BufferAccessor normals;
				BufferAccessor tex_coords;
				BufferAccessor joints;
				BufferAccessor weights;

				bool emit;

				const Image& color_tex;
			};

			std::vector<Primitive> primitives;
		};

		struct Bone
		{
			Node& node;
		};


		GLTFWrapper(const DeviceConfiguration& device_cfg, const std::string& path);

		std::vector<Node> nodes;
		std::vector<Mesh> meshes;
		std::vector<Bone> bones;

	private:

		BufferAccessor BuildBufferAccessor(int acc_ind);

		tinygltf::Model gltf_model;

		std::vector<GPULocalBuffer> buffers_;
		std::vector<Image> images_;
		std::vector<ImageView> images_views_;
	};
}

#endif  // RENDER_ENGINE_GLTFWRAPPER_H_