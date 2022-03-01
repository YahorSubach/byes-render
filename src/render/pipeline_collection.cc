#include "pipeline_collection.h"

#include <fstream>

static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
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
	render_passes_.push_back(RenderPass(device_cfg_, VkFormat(output_format)));

	VertexBindingDesc position_binding =
	{
	12,

		{
			{ VK_FORMAT_R32G32B32_SFLOAT, 0}
		}
	};

	VertexBindingDesc normal_binding =
	{
	12,

		{
			{ VK_FORMAT_R32G32B32_SFLOAT, 0}
		}
	};

	VertexBindingDesc tex_binding =
	{
	8,

		{
			{ VK_FORMAT_R32G32_SFLOAT, 0}
		}
	};

	VertexBindings vertex_bindings = { position_binding, normal_binding, tex_binding };

	auto vert_shader_code = readFile("../shaders/white_v.spv");
	auto frag_shader_code = readFile("../shaders/white_f.spv");

	VkShaderModule vert_shader_module = CreateShaderModule(vert_shader_code);
	VkShaderModule frag_shader_module = CreateShaderModule(frag_shader_code);

	pipelines_.push_back(GraphicsPipeline(device_cfg, vert_shader_module, frag_shader_module, output_extent, render_passes_.back(), vertex_bindings));

	vkDestroyShaderModule(device_cfg.logical_device, frag_shader_module, nullptr);
	vkDestroyShaderModule(device_cfg.logical_device, vert_shader_module, nullptr);
}

const render::RenderPass& render::PipelineCollection::GetRenderPass() const
{
	return render_passes_.front();
}

const render::GraphicsPipeline& render::PipelineCollection::GetPipeline() const
{
	return pipelines_.front();
}
