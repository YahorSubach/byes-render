#ifndef RENDER_ENGINE_RENDER_GRAPHICS_PIPELINE_H_
#define RENDER_ENGINE_RENDER_GRAPHICS_PIPELINE_H_

#include <vector>

#include "vulkan/vulkan.h"

#include "render/object_base.h"
#include "render/render_pass.h"

namespace render
{
	struct VertexBindingAttributeDesc
	{
		uint32_t format_;
		uint32_t offset_;
	};

	struct VertexBindingDesc
	{
		uint64_t stride_;
		std::vector<VertexBindingAttributeDesc> attributes_;
	};

	struct VertexPushConstants {
		glm::mat4 project_matrix;
		glm::mat4 view_model_matrix;
	};

	struct FragmentPushConstants {
		float metallic;
		float roughness;
	};

	using VertexBindings = std::vector<VertexBindingDesc>;

	class GraphicsPipeline : public RenderObjBase<VkPipeline>
	{
	public:
		GraphicsPipeline(const DeviceConfiguration& device_cfg, const VkShaderModule& vert_shader_module, const VkShaderModule& frag_shader_module, const VkExtent2D& extent, const RenderPass& render_pass, const VertexBindings& bindings);

		GraphicsPipeline(const GraphicsPipeline&) = delete;
		GraphicsPipeline(GraphicsPipeline&&) = default;

		GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
		GraphicsPipeline& operator=(GraphicsPipeline&&) = default;

		const VkDescriptorSetLayout& GetDescriptorSetLayout() const;
		const VkPipelineLayout& GetLayout() const;

		virtual ~GraphicsPipeline() override;
	private:

		std::vector<VkVertexInputBindingDescription> BuildVertexInputBindingDescriptions();
		std::vector<VkVertexInputAttributeDescription> BuildVertexAttributeDescription();

		VertexBindings bindings_;

		VkDescriptorSetLayout descriptor_set_layot_;
		VkPipelineLayout layout_;
	};
}
#endif  // RENDER_ENGINE_RENDER_GRAPHICS_PIPELINE_H_