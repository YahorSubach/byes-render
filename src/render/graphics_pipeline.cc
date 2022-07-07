#include "graphics_pipeline.h"

#include <array>
#include <vector>

#include "glm/glm/glm.hpp"

#include "common.h"
#include "render/data_types.h"


render::GraphicsPipeline::GraphicsPipeline(const DeviceConfiguration& device_cfg, Extent extent, const RenderPass& render_pass, const ShaderModule& vertex_shader_module, const ShaderModule& fragment_shader_module, bool enable_depth_test):
	RenderObjBase(device_cfg), layout_(VK_NULL_HANDLE)
{
	std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_infos;
	std::vector<VkVertexInputBindingDescription> vertex_input_bindings_descs;
	std::vector<VkVertexInputAttributeDescription> vertex_input_attr_descs;

	{
		VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
		vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		vert_shader_stage_info.module = vertex_shader_module.GetHandle();
		vert_shader_stage_info.pName = "main"; // entry point in shader

		if (!vertex_shader_module.GetVertexBindingsDescs().empty())
		{
			vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;

			vertex_input_bindings_descs = BuildVertexInputBindingDescriptions(vertex_shader_module.GetVertexBindingsDescs());
			vertex_input_attr_descs = BuildVertexAttributeDescription(vertex_shader_module.GetVertexBindingsDescs());
			vertex_bindings_count_ = vertex_input_bindings_descs.size();
		}

		shader_stage_create_infos.push_back(vert_shader_stage_info);
	}

	{
		VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
		frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_stage_info.module = fragment_shader_module.GetHandle();
		frag_shader_stage_info.pName = "main"; // entry point in shader

		shader_stage_create_infos.push_back(frag_shader_stage_info);
	}

	VkPipelineVertexInputStateCreateInfo vertex_input_info{};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = u32(vertex_input_bindings_descs.size());
	vertex_input_info.pVertexBindingDescriptions = vertex_input_bindings_descs.data(); // Optional
	vertex_input_info.vertexAttributeDescriptionCount = u32(vertex_input_attr_descs.size());
	vertex_input_info.pVertexAttributeDescriptions = vertex_input_attr_descs.data(); // Optional

	VkPipelineInputAssemblyStateCreateInfo input_assembly{};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
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

	//TODO: configure blend

	VkPipelineColorBlendAttachmentState color_blend_attachment{};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_TRUE;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // Optional
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
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
	depth_stencil.depthTestEnable = enable_depth_test;
	depth_stencil.depthWriteEnable = VK_TRUE;
	depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil.depthBoundsTestEnable = VK_FALSE;
	depth_stencil.minDepthBounds = -1.0f; // Optional
	depth_stencil.maxDepthBounds = 1.0f; // Optional
	depth_stencil.stencilTestEnable = VK_FALSE;
	depth_stencil.front = {}; // Optional
	depth_stencil.back = {}; // Optional


	VkPushConstantRange vertex_push_constant{};
	vertex_push_constant.offset = 0;
	vertex_push_constant.size = sizeof(VertexPushConstants);
	vertex_push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPushConstantRange fragment_push_constant{};
	fragment_push_constant.offset = vertex_push_constant.size;
	fragment_push_constant.size = sizeof(FragmentPushConstants);
	fragment_push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::vector<VkPushConstantRange> push_constants = { vertex_push_constant, fragment_push_constant };

	for (auto&& [set_index, set_layout] : vertex_shader_module.GetDescriptorSets())
	{
		if (auto&& existing_set = descriptor_sets_.find(set_index); existing_set != descriptor_sets_.end())
		{
			if (existing_set->second.GetType() != set_layout.GetType())
			{
				throw std::runtime_error("invalid descripton set combination");
			}
		}

		descriptor_sets_.emplace(set_index, set_layout);
	}

	for (auto&& [set_index, set_layout] : fragment_shader_module.GetDescriptorSets())
	{
		if (auto&& existing_set = descriptor_sets_.find(set_index); existing_set != descriptor_sets_.end())
		{
			if (existing_set->second.GetType() != set_layout.GetType())
			{
				throw std::runtime_error("invalid descripton set combination");
			}
		}

		descriptor_sets_.emplace(set_index, set_layout);
	}

	std::vector<VkDescriptorSetLayout> descriptor_sets_layouts;

	int processed_desc_layouts = 0;

	for (int i = 0; processed_desc_layouts < descriptor_sets_.size(); i++)
	{
		if (descriptor_sets_.find(i) != descriptor_sets_.end())
		{
			descriptor_sets_layouts.push_back(descriptor_sets_.at(i).GetHandle());
			processed_desc_layouts++;
		}
		else
		{
			//descriptor_sets_layouts.push_back(VK_NULL_HANDLE);
		}
	}


	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = u32(descriptor_sets_layouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptor_sets_layouts.data(); // Optional
	pipelineLayoutInfo.pushConstantRangeCount = u32(push_constants.size()); // Optional
	pipelineLayoutInfo.pPushConstantRanges = push_constants.data(); // Optional

	if (vkCreatePipelineLayout(device_cfg_.logical_device, &pipelineLayoutInfo, nullptr, &layout_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = shader_stage_create_infos.size();
	pipeline_info.pStages = shader_stage_create_infos.data();

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

const std::map<uint32_t, const render::DescriptorSetLayout&>& render::GraphicsPipeline::GetDescriptorSets() const
{
	return descriptor_sets_;
}

const VkPipelineLayout& render::GraphicsPipeline::GetLayout() const
{
	return layout_;
}

uint32_t render::GraphicsPipeline::GetVertesBindingsCount() const
{
	return vertex_bindings_count_;
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
	}
}

std::vector<VkVertexInputBindingDescription> render::GraphicsPipeline::BuildVertexInputBindingDescriptions(const std::vector<render::ShaderModule::VertexBindingDesc>& vertex_bindings_descs)
{
	std::vector<VkVertexInputBindingDescription> result(vertex_bindings_descs.size());

	for (uint32_t i = 0; i < vertex_bindings_descs.size(); i++)
	{
		result[i].binding = i;
		result[i].stride = vertex_bindings_descs[i].stride;
		result[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	}

	return result;
}

std::vector<VkVertexInputAttributeDescription> render::GraphicsPipeline::BuildVertexAttributeDescription(const std::vector<render::ShaderModule::VertexBindingDesc>& vertex_bindings_descs)
{
	std::vector<VkVertexInputAttributeDescription> result;

	uint32_t location_index = 0;

	for (uint32_t binding_index = 0; binding_index < vertex_bindings_descs.size(); binding_index++)
	{
		for (uint32_t attr_index = 0; attr_index < vertex_bindings_descs[binding_index].attributes.size(); attr_index++)
		{
			result.push_back({});
			result.back().binding = binding_index;
			result.back().format = static_cast<VkFormat>(vertex_bindings_descs[binding_index].attributes[attr_index].format);
			result.back().location = location_index;
			result.back().offset = vertex_bindings_descs[binding_index].attributes[attr_index].offset;

			location_index++;
		}
	}

	return result;
}
