#include "pipeline_collection.h"

#include <fstream>

#include "render/shader_module.h"

//static std::vector<char> readFile(const std::string& filename) {
//	std::ifstream file(filename, std::ios::ate | std::ios::binary);
//
//	if (!file.is_open()) {
//		throw std::runtime_error("failed to open file!");
//	}
//
//	size_t fileSize = (size_t)file.tellg();
//	std::vector<char> buffer(fileSize);
//
//	file.seekg(0);
//	file.read(buffer.data(), fileSize);
//
//	file.close();
//
//	return buffer;
//}

void render::PipelineCollection::InitDescriptorSetLayouts(const DeviceConfiguration& device_cfg)
{
	//descriptor_set_layouts_.resize(static_cast<int32_t>(DescriptorSetLayoutId::kDescriptorSetLayoutsIdCount));

	{ // kCameraMatrices
		std::vector<DescriptorSetLayout::DescriptorSetLayoutBindingDesc> desc_set_layout_desc =
		{
			{
				static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
				static_cast<uint32_t>(VK_SHADER_STAGE_VERTEX_BIT)
			}
		};

		DescriptorSetLayout descriptor_set_layout(device_cfg, desc_set_layout_desc);
		descriptor_set_layouts_.push_back(std::move(descriptor_set_layout));
	}

	{ // kObjectSet
		std::vector<DescriptorSetLayout::DescriptorSetLayoutBindingDesc> desc_set_layout_desc =
		{
			{
				static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
				static_cast<uint32_t>(VK_SHADER_STAGE_VERTEX_BIT)
			}
		};

		DescriptorSetLayout descriptor_set_layout(device_cfg, desc_set_layout_desc);
		descriptor_set_layouts_.push_back(std::move(descriptor_set_layout));
	}

	{ // kMaterialSet
		std::vector<DescriptorSetLayout::DescriptorSetLayoutBindingDesc> desc_set_layout_desc =
		{
			{
				static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
				static_cast<uint32_t>(VK_SHADER_STAGE_FRAGMENT_BIT)
			},
			{
				static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
				static_cast<uint32_t>(VK_SHADER_STAGE_FRAGMENT_BIT)
			},
		};

		DescriptorSetLayout descriptor_set_layout(device_cfg, desc_set_layout_desc);
		descriptor_set_layouts_.push_back(std::move(descriptor_set_layout));
	}

}

VkShaderModule render::PipelineCollection::CreateShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shader_module;
	if (vkCreateShaderModule(device_cfg_.logical_device, &createInfo, nullptr, &shader_module) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shader_module;
}

render::PipelineCollection::PipelineCollection(const DeviceConfiguration& device_cfg, Extent output_extent, uint32_t output_format): RenderObjBase(device_cfg)
{
	InitDescriptorSetLayouts(device_cfg);

	auto render_pass_desc = RenderPass::BuildRenderPassDesc(RenderPass::RenderPassType::kDraw, (VkFormat)output_format, VK_FORMAT_D32_SFLOAT);
	render_passes_.push_back(RenderPass(device_cfg_, render_pass_desc));

	//auto vert_shader_code = readFile("../shaders/white_v.spv");
	//auto frag_shader_code = readFile("../shaders/white_f.spv");

	ShaderModule vert_shader_module(device_cfg, "color.vert.spv");
	ShaderModule frag_shader_module(device_cfg, "color.frag.spv");

	GraphicsPipelineCreateInfo pipeline_create_info;
	pipeline_create_info.extent = output_extent;
	pipeline_create_info.shader_modules = { vert_shader_module , frag_shader_module };
	pipeline_create_info.descriptor_set_layouts =
	{
		GetDescriptorSetLayout(DescriptorSetLayoutId::kCameraMatrices),
		GetDescriptorSetLayout(DescriptorSetLayoutId::kObjectSet),
		GetDescriptorSetLayout(DescriptorSetLayoutId::kMaterialSet),
	};

	pipelines_.push_back(GraphicsPipeline(device_cfg, render_passes_.back(), pipeline_create_info));


	auto depth_render_pass_desc = RenderPass::BuildRenderPassDesc(RenderPass::RenderPassType::kDepth, (VkFormat)output_format, VK_FORMAT_D32_SFLOAT);
	render_passes_.push_back(RenderPass(device_cfg_, depth_render_pass_desc));

}

const render::RenderPass& render::PipelineCollection::GetRenderPass(RenderPassId renderpass_id) const
{
	return render_passes_[static_cast<uint32_t>(renderpass_id)];
}

const render::DescriptorSetLayout& render::PipelineCollection::GetDescriptorSetLayout(DescriptorSetLayoutId descriptor_set_layout_id) const
{
	return descriptor_set_layouts_[static_cast<uint32_t>(descriptor_set_layout_id)];
}

const render::GraphicsPipeline& render::PipelineCollection::GetPipeline(PipelineId pipeline_id) const
{
	return pipelines_[static_cast<uint32_t>(pipeline_id)];
}
