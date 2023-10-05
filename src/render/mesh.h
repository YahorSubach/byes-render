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

#include "render/ui/ui.h"

#include "byes-reference-to-movable\reference_to_movable.h"

namespace render
{

	class GraphicsPipeline;
	struct Material
	{
		glm::vec4 color;

		util::NullableRef<const Image> albedo;
		util::NullableRef<const Image> metallic_roughness;
		util::NullableRef<const Image> normal_map;

		uint32_t flags = 0;
	};




	
	namespace primitive
	{
		using GeometryDescriptorSetHolder = descriptor_sets_holder::Holder<DescriptorSetType::kMaterial, DescriptorSetType::kColor>;

		struct Base
		{
			PrimitiveFlags flags;

			std::optional<BufferAccessor> indices;
			std::array<std::optional<BufferAccessor>, kVertexBufferTypesCount> vertex_buffers;

			Base(PrimitiveFlags flags) :flags(flags) {}
		};

		struct Geometry : Base, public GeometryDescriptorSetHolder
		{
			Geometry(const Global& global, DescriptorSetsManager& manager, PrimitiveFlags flags);

			Material material;

			bool FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<0>::Data& data) override;
			bool FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<1>::Data& data) override;
			bool FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<2>::Data& data) override;
			bool FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<3>::Data& data) override;

			bool FillData(render::DescriptorSet<render::DescriptorSetType::kColor>::Binding<0>::Data& data) override;
		};

		using BitmapDescriptorSetHolder = descriptor_sets_holder::Holder<DescriptorSetType::kBitmapAtlas, DescriptorSetType::kTexture>;

		struct Bitmap : Base, public BitmapDescriptorSetHolder
		{
			Bitmap(const Global& global, DescriptorSetsManager& manager, const ui::UI& ui, glm::vec2 atlas_position, glm::vec2 atlas_width_height);
			Bitmap(const Global& global, DescriptorSetsManager& manager, const render::ui::UI& ui, const ui::Glyph glyph);
			Bitmap(const Global& global, DescriptorSetsManager& manager, const render::ui::UI& ui, const Image& image);
			const Image& atlas;

			glm::vec2 atlas_position; 
			glm::vec2 atlas_width_height;

			bool FillData(render::DescriptorSet<render::DescriptorSetType::kBitmapAtlas>::Binding<0>::Data& data) override;
			bool FillData(render::DescriptorSet<render::DescriptorSetType::kTexture>::Binding<0>::Data& data) override;
		};
	}

	using Primitive = std::variant<primitive::Geometry, primitive::Bitmap>;

	struct Node: byes::RM<Node>
	{
		//glm::vec3 translation;
		//glm::quat rotation;
		//glm::vec3 scale;
		
		glm::mat4 local_transform = glm::identity<glm::mat4>();

		byes::RTM<Node> parent;

		glm::mat4 GetGlobalTransformMatrix() const;
	};

	using NodeId = util::container::ErVec<Node>::Id;

	struct NodeTree
	{
		std::vector<Node> nodes;
		//util::NullableRef<Node> NodeParent(const Node& child_node) { if (child_node.parent_node_index < std::numeric_limits<unsigned short>::max()) return nodes[child_node.parent_node_index]; else return std::nullopt; }
	};


	struct Skin
	{
		std::vector<Node> nodes;
		std::vector<glm::mat4> inverse_bind_matrices;
		std::vector<short> parent_indices;
	};

	struct Mesh: byes::RM<Mesh>
	{
		Mesh() = default;
		Mesh(const std::string name, Primitive&& primitive);
		Mesh(const std::string& name) : name(name) {};
		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&) = default;

		std::string name;
		//std::vector<Bone> joints;
		std::vector<Primitive> primitives;
	};

	using ModelDescriptorSetHolder = descriptor_sets_holder::Holder<DescriptorSetType::kModelMatrix, DescriptorSetType::kSkeleton>;

	struct ModelInfo
	{
		Node& node;
		util::NullableRef<Mesh> mesh;
		util::NullableRef<Skin> skin;
	};

	//struct NodeRef
	//{
	//	std::reference_wrapper<NodeTree>& node_tree;
	//	unsigned node_index;
	//	Node& Get() { return node_tree.get().nodes[node_index]; }
	//};

	struct Model
	{
		util::NullableRef<Node> node;
		util::NullableRef<Mesh> mesh;
		util::NullableRef<Skin> skin;
	};

	using RenderModelDescriptorSetHolder = descriptor_sets_holder::Holder<DescriptorSetType::kModelMatrix>;

	struct RenderModel : byes::RM<RenderModel>, RenderModelDescriptorSetHolder
	{
		RenderModel(const Global& global, DescriptorSetsManager& manager, Node& node_in, Mesh& mesh_in);
		RenderModel(const RenderModel&) = delete;
		RenderModel(RenderModel&&) = default;
		RenderModel& operator=(const RenderModel&) = delete;
		RenderModel& operator=(RenderModel&&) = default;
		byes::RTM<Node> node;
		byes::RTM<Mesh> mesh;
		util::NullableRef<Skin> skin;

		bool FillData(render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data) override;
	};

	using RenderModelId = util::container::ErVec<RenderModel>::Id;

	struct ModelInstance: public ModelDescriptorSetHolder
	{
		ModelInstance(const Global& global, DescriptorSetsManager& manager, Node& node_in, Mesh& mesh_in);

	
		util::NullableRef<Mesh> mesh;
		util::NullableRef<Skin> skin;

		bool FillData(render::DescriptorSet<render::DescriptorSetType::kSkeleton>::Binding<0>::Data& data) override;
		bool FillData(render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data) override;
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