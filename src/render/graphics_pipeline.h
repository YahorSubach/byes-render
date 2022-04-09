#ifndef RENDER_ENGINE_RENDER_GRAPHICS_PIPELINE_H_
#define RENDER_ENGINE_RENDER_GRAPHICS_PIPELINE_H_

#include <vector>

#include "vulkan/vulkan.h"

#include "render/descriptor_set_layout.h"
#include "render/object_base.h"
#include "render/render_pass.h"
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
		GraphicsPipeline(const DeviceConfiguration& device_cfg, const RenderPass& render_pass, const GraphicsPipelineCreateInfo& create_info);

		GraphicsPipeline(const GraphicsPipeline&) = delete;
		GraphicsPipeline(GraphicsPipeline&&) = default;

		GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
		GraphicsPipeline& operator=(GraphicsPipeline&&) = default;

		const VkPipelineLayout& GetLayout() const;

		virtual ~GraphicsPipeline() override;
	private:

		std::vector<VkVertexInputBindingDescription> BuildVertexInputBindingDescriptions(const std::vector<render::ShaderModule::VertexBindingDesc>& vertex_bindings_descs);
		std::vector<VkVertexInputAttributeDescription> BuildVertexAttributeDescription(const std::vector<render::ShaderModule::VertexBindingDesc>& vertex_bindings_descs);

		VkPipelineLayout layout_;
	};
}
#endif  // RENDER_ENGINE_RENDER_GRAPHICS_PIPELINE_H_