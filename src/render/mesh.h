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
#include "render/data_types.h"
#include "render/vertex_buffer.h"
#include "render/descriptor_set_holder.h"

namespace render
{

	class GraphicsPipeline;
	struct Material
	{
		PipelineId pipeline_type;

		util::NullableRef<const Image> albedo;
		util::NullableRef<const Image> metallic_roughness;
		util::NullableRef<const Image> normal_map;

		uint32_t flags = 0;
	};

	struct Primitive;
	using PrimitiveDescriptorSetHolder = descriptor_sets_holder::Holder<Primitive, DescriptorSetType::kMaterial>;

	struct Primitive: public PrimitiveDescriptorSetHolder
	{
		Primitive(const Global& global, DescriptorSetsManager& manager);

		RenderModelCategory category;
		Material material;

		std::optional<BufferAccessor> indices;
		std::array<std::optional<BufferAccessor>, kVertexBufferTypesCount> vertex_buffers;

		void FillData(const Primitive& scene, render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<0>::Data& data) override;
		void FillData(const Primitive& scene, render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<1>::Data& data) override;
		void FillData(const Primitive& scene, render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<2>::Data& data) override;
		void FillData(const Primitive& scene, render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<3>::Data& data) override;
	};

	struct Node
	{
		glm::vec3 translation;
		glm::quat rotation;
		glm::vec3 scale;
		
		glm::mat4 local_transform;
		
		util::NullableRef<Node> parent;

		glm::mat4 GetGlobalTransformMatrix() const;
	};



	struct Joint
	{
		Node node;
		glm::mat4 inverse_bind_matrix;
	};

	struct Skin
	{
		std::vector<Joint> joints;
	};

	struct Mesh
	{
		//std::vector<Bone> joints;
		std::vector<Primitive> primitives;
	};

	struct Model;
	using ModelDescriptorSetHolder = descriptor_sets_holder::Holder<Model, DescriptorSetType::kModelMatrix, DescriptorSetType::kSkeleton>;

	struct Model: public ModelDescriptorSetHolder
	{
		Model(const Global& global, DescriptorSetsManager& manager, Node& node_in, Mesh& mesh_in);

		Node& node;

		util::NullableRef<Mesh> mesh;
		util::NullableRef<Skin> skin;

		void FillData(const Model& scene, render::DescriptorSet<render::DescriptorSetType::kSkeleton>::Binding<0>::Data& data) override;
		void FillData(const Model& scene, render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data) override;
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