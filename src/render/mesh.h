#ifndef RENDER_ENGINE_RENDER_MESH_H_
#define RENDER_ENGINE_RENDER_MESH_H_

#include <vector>
#include <array>

#include "vulkan/vulkan.h"
#include "glm/glm/glm.hpp"

#include "common.h"

#include "buffer.h"
#include "image.h"
#include "stl_util.h"

namespace render
{

	enum class RenderModelType
	{
		kStatic,
		kSkinned
	};

	struct Primitive
	{
		BufferAccessor indices;

		BufferAccessor positions;
		BufferAccessor normals;
		BufferAccessor tex_coords;
		BufferAccessor joints;
		BufferAccessor weights;

		stl_util::NullableRef<const Image> color_tex;

		std::vector<BufferAccessor> vertex_buffers;

		RenderModelType type;

		bool emit;
	};

	struct Node
	{
		glm::mat4 node_matrix;
		stl_util::NullableRef<Node> parent;

		glm::mat4 GetGlobalTransformMatrix() const;
	};

	struct Bone
	{
		Node& node;

		glm::mat4 inverse_bind_matrix;
	};

	struct Mesh
	{
		Node& node;

		std::vector<Bone> joints;

		std::vector<Primitive> primitives;
	};



}

#endif  // RENDER_ENGINE_RENDER_RENDER_PASS_H_