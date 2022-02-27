#ifndef RENDER_ENGINE_GLTFWRAPPER_H_
#define RENDER_ENGINE_GLTFWRAPPER_H_

#include <string>
#include <vector>

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

		struct Mesh
		{
			struct Primitive
			{
				BufferAccessor indices;
				BufferAccessor positions;
				BufferAccessor normals;
				BufferAccessor tex_coords;

				const ImageView& color_tex;
			};

			std::vector<Primitive> primitives;
		};

		struct Node
		{
			Mesh mesh;
			glm::mat4 node_matrix;
		};


		GLTFWrapper(const DeviceConfiguration& device_cfg, const std::string& path);

		std::vector<Node> nodes;

	private:

		BufferAccessor BuildBufferAccessor(int acc_ind);

		tinygltf::Model model;

		std::vector<GPULocalBuffer> buffers_;
		std::vector<Image> images_;
		std::vector<ImageView> images_views_;
	};
}

#endif  // RENDER_ENGINE_GLTFWRAPPER_H_