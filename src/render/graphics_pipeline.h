#ifndef RENDER_ENGINE_RENDER_GRAPHICS_PIPELINE_H_
#define RENDER_ENGINE_RENDER_GRAPHICS_PIPELINE_H_

#include <vector>

#include "vulkan/vulkan.h"

#include "render/descriptor_set_layout.h"
#include "render/object_base.h"
#include "render/render_graph.h"
#include "render/shader_module.h"

namespace render
{
	struct VertexPushConstants {
		glm::mat4 project_matrix;
		glm::mat4 view_model_matrix;
	};

	struct FragmentPushConstants {
		float metallic;
		float roughness;
	};

	struct GraphicsPipelineCreateInfo
	{
		Extent extent;

		std::vector<std::reference_wrapper<const ShaderModule>> shader_modules;
		std::vector<std::reference_wrapper<const DescriptorSetLayout>> descriptor_set_layouts;
	};

	class GraphicsPipeline : public RenderObjBase<VkPipeline>
	{
	public:

		enum class
		EParams
		{
			kDisableDepthTest,
			kLineTopology
		};

		using Params = util::enums::Flags<EParams>;

		GraphicsPipeline(const Global& global, const RenderNode& render_node, const ShaderModule& vertex_shader_module, const ShaderModule& fragment_shader_module,
			const std::array<Extent, kExtentTypeCnt>& extents, Params params = {});

		GraphicsPipeline(const GraphicsPipeline&) = delete;
		GraphicsPipeline(GraphicsPipeline&&) = default;

		GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
		GraphicsPipeline& operator=(GraphicsPipeline&&) = default;

		const std::map<uint32_t, const DescriptorSetLayout&>& GetDescriptorSetLayouts() const;
		const std::map<uint32_t, ShaderModule::VertexBindingDesc>& GetVertexBindingsDescs() const;

		const VkPipelineLayout& GetLayout() const;

		uint32_t GetVertexBindingsCount() const;

		virtual ~GraphicsPipeline() override;
	private:

		std::vector<VkVertexInputBindingDescription> BuildVertexInputBindingDescriptions(const std::map<uint32_t, render::ShaderModule::VertexBindingDesc>& vertex_bindings_descs);
		std::vector<VkVertexInputAttributeDescription> BuildVertexAttributeDescription(const std::map<uint32_t, render::ShaderModule::VertexBindingDesc>& vertex_bindings_descs);
		
		std::map<uint32_t, ShaderModule::VertexBindingDesc> vertex_bindings_descs_;
		std::map<uint32_t, const DescriptorSetLayout&> descriptor_sets_;

		VkPipelineLayout layout_;

		uint32_t vertex_bindings_count_;
	};
}
#endif  // RENDER_ENGINE_RENDER_GRAPHICS_PIPELINE_H_