#include "graphics_pipeline.h"

#include <array>
#include <vector>

#include "glm/glm/glm.hpp"

#include "common.h"
#include "render/data_types.h"


render::GraphicsPipeline::GraphicsPipeline(const DeviceConfiguration& device_cfg, const VkShaderModule& vert_shader_module, const VkShaderModule& frag_shader_module, const Extent& extent, const render::RenderPass& render_pass, const VertexBindings& bindings):
	RenderObjBase(device_cfg), layout_(VK_NULL_HANDLE), bindings_(bindings)
{
	VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
	vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_shader_stage_info.module = vert_shader_module;
	vert_shader_stage_info.pName = "main"; // entry point in shader

	VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
	frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_shader_stage_info.module = frag_shader_module;
	frag_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo shader_stages[] = { vert_shader_stage_info, frag_shader_stage_info };

	auto binding_description = BuildVertexInputBindingDescriptions();
	auto attribute_descriptions = BuildVertexAttributeDescription();

	VkPipelineVertexInputStateCreateInfo vertex_input_info{};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = binding_description.size();
	vertex_input_info.pVertexBindingDescriptions = binding_description.data(); // Optional
	vertex_input_info.vertexAttributeDescriptionCount = attribute_descriptions.size();
	vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data(); // Optional

	VkPipelineInputAssemblyStateCreateInfo input_assembly{};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = extent.width;
	viewport.height = extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;

	VkPipelineViewportStateCreateInfo viewport_state{};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE; //then geometry never passes through the rasterizer stage. This basically disables any output to the framebuffer.
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional


	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional


	VkPipelineColorBlendAttachmentState color_blend_attachment{};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional


	VkPipelineColorBlendStateCreateInfo color_blending{};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;
	color_blending.blendConstants[0] = 0.0f; // Optional
	color_blending.blendConstants[1] = 0.0f; // Optional
	color_blending.blendConstants[2] = 0.0f; // Optional
	color_blending.blendConstants[3] = 0.0f; // Optional

	VkPipelineDepthStencilStateCreateInfo depth_stencil{};
	depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil.depthTestEnable = VK_TRUE;
	depth_stencil.depthWriteEnable = VK_TRUE;
	depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil.depthBoundsTestEnable = VK_FALSE;
	depth_stencil.minDepthBounds = 0.0f; // Optional
	depth_stencil.maxDepthBounds = 1.0f; // Optional
	depth_stencil.stencilTestEnable = VK_FALSE;
	depth_stencil.front = {}; // Optional
	depth_stencil.back = {}; // Optional

	VkDescriptorSetLayoutBinding ubo_layout_binding{};
	ubo_layout_binding.binding = 0;
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	ubo_layout_binding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding color_sampler_layout_binding{};
	color_sampler_layout_binding.binding = 1;
	color_sampler_layout_binding.descriptorCount = 1;
	color_sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	color_sampler_layout_binding.pImmutableSamplers = nullptr;
	color_sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding env_sampler_layout_binding{};
	env_sampler_layout_binding.binding = 2;
	env_sampler_layout_binding.descriptorCount = 1;
	env_sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	env_sampler_layout_binding.pImmutableSamplers = nullptr;
	env_sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 3> desc_bindings = { ubo_layout_binding, color_sampler_layout_binding, env_sampler_layout_binding };

	VkDescriptorSetLayoutCreateInfo ubo_layout_create_info{};
	ubo_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	ubo_layout_create_info.bindingCount = desc_bindings.size();
	ubo_layout_create_info.pBindings = desc_bindings.data();

	if (vkCreateDescriptorSetLayout(device_cfg_.logical_device, &ubo_layout_create_info, nullptr, &descriptor_set_layot_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}


	VkPushConstantRange vertex_push_constant{};
	vertex_push_constant.offset = 0;
	vertex_push_constant.size = sizeof(VertexPushConstants);
	vertex_push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPushConstantRange fragment_push_constant{};
	fragment_push_constant.offset = vertex_push_constant.size;
	fragment_push_constant.size = sizeof(FragmentPushConstants);
	fragment_push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::vector<VkPushConstantRange> push_constants = { vertex_push_constant, fragment_push_constant };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1; // Optional
	pipelineLayoutInfo.pSetLayouts = &descriptor_set_layot_; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = push_constants.size(); // Optional
	pipelineLayoutInfo.pPushConstantRanges = push_constants.data(); // Optional

	if (vkCreatePipelineLayout(device_cfg_.logical_device, &pipelineLayoutInfo, nullptr, &layout_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;

	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pDepthStencilState = nullptr; // Optional
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.pDepthStencilState = &depth_stencil;
	pipeline_info.pDynamicState = nullptr; // Optional

	pipeline_info.layout = layout_;

	pipeline_info.renderPass = render_pass.GetHandle();
	pipeline_info.subpass = 0;

	pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipeline_info.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(device_cfg_.logical_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &handle_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}

const VkDescriptorSetLayout& render::GraphicsPipeline::GetDescriptorSetLayout() const
{
	return descriptor_set_layot_;
}

const VkPipelineLayout& render::GraphicsPipeline::GetLayout() const
{
	return layout_;
}

render::GraphicsPipeline::~GraphicsPipeline()
{
	if (handle_ != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(device_cfg_.logical_device, handle_, nullptr);

		if (layout_ != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(device_cfg_.logical_device, layout_, nullptr);
		}

		if (layout_ != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(device_cfg_.logical_device, descriptor_set_layot_, nullptr);
		}
	}
}

std::vector<VkVertexInputBindingDescription> render::GraphicsPipeline::BuildVertexInputBindingDescriptions()
{
	std::vector<VkVertexInputBindingDescription> result(bindings_.size());

	for (uint32_t i = 0; i < bindings_.size(); i++)
	{
		result[i].binding = i;
		result[i].stride = bindings_[i].stride_;
		result[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	}

	return result;
}

std::vector<VkVertexInputAttributeDescription> render::GraphicsPipeline::BuildVertexAttributeDescription()
{
	std::vector<VkVertexInputAttributeDescription> result;

	uint32_t location_index = 0;

	for (uint32_t binding_index = 0; binding_index < bindings_.size(); binding_index++)
	{
		for (uint32_t attr_index = 0; attr_index < bindings_[binding_index].attributes_.size(); attr_index++)
		{
			result.push_back({});
			result.back().binding = binding_index;
			result.back().format = static_cast<VkFormat>(bindings_[binding_index].attributes_[attr_index].format_);
			result.back().location = location_index;
			result.back().offset = bindings_[binding_index].attributes_[attr_index].offset_;

			location_index++;
		}
	}

	return result;
}
