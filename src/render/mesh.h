#ifndef RENDER_ENGINE_RENDER_MESH_H_
#define RENDER_ENGINE_RENDER_MESH_H_

#include <vector>
#include <array>
#include <chrono>

#include "vulkan/vulkan.h"
#include "glm/glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

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

	struct Material
	{
		stl_util::NullableRef<const Image> albedo;
		stl_util::NullableRef<const Image> metallic_roughness;
		stl_util::NullableRef<const Image> normal_map;

		uint32_t flags = 0;
	};

	struct Primitive
	{
		BufferAccessor indices;

		BufferAccessor positions;
		BufferAccessor normals;
		BufferAccessor tangents;
		BufferAccessor tex_coords;
		BufferAccessor joints;
		BufferAccessor weights;

		Material material;

		std::vector<BufferAccessor> vertex_buffers;

		RenderModelType type = RenderModelType::kStatic;
	};

	struct Node
	{
		glm::vec3 translation;
		glm::quat rotation;
		glm::vec3 scale;
		
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

	template<typename ValueType>
	struct FrameValue
	{
		std::chrono::milliseconds time;
		ValueType value;
	};

	enum class InterpolationType
	{
		kLinear,
		kCubicSpline
	};

	template<typename ValueType>
	struct AnimSampler // TODO move to namespace
	{
		uint32_t node_index;

		InterpolationType interpolation_type;
		std::vector<FrameValue<ValueType>> frames;
	};

	struct Animation
	{
		std::vector<AnimSampler<glm::vec3>> translations;
		std::vector<AnimSampler<glm::vec3>> scales;
		std::vector<AnimSampler<glm::quat>> rotations;
	};
}

#endif  // RENDER_ENGINE_RENDER_RENDER_PASS_H_