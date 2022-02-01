#include "batches_manager.h"

#include <fstream>

#include "command_pool.h"

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

render::BatchesManager::BatchesManager(const DeviceConfiguration& device_cfg, uint32_t frames_cnt, const Swapchain& swapchain, const RenderPass& render_pass, DescriptorPool& descriptor_pool):RenderObjBase(device_cfg.logical_device)
{
	const std::vector<Vertex> vertices = {
{{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
{{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
{{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

{{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
{{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
{{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
	};

	const std::vector<Face> faces = {
		{0, 1, 2}, {2, 3, 0},
		{4, 5, 6}, {6, 7, 4}
	};

	std::vector<uint32_t> queue_indices = { device_cfg.graphics_queue_index, device_cfg.transfer_queue_index };

	{
		Buffer vertices_gpu_buffer(device_cfg, sizeof(vertices[0]) * vertices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, queue_indices);
		Buffer faces_gpu_buffer(device_cfg, sizeof(faces[0]) * faces.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, queue_indices);
		
		CopyToGPUBuffer(device_cfg, vertices_gpu_buffer, static_cast<const void*>(vertices.data()), sizeof(vertices[0]) * vertices.size());
		CopyToGPUBuffer(device_cfg, faces_gpu_buffer, static_cast<const void*>(faces.data()), sizeof(faces[0]) * faces.size());

		auto vert_shader_code = readFile("../shaders/vert.spv");
		auto frag_shader_code = readFile("../shaders/frag.spv");

		VkShaderModule vert_shader_module = CreateShaderModule(vert_shader_code);
		VkShaderModule frag_shader_module = CreateShaderModule(frag_shader_code);

		GraphicsPipeline pipeline(device_cfg.logical_device, vert_shader_module, frag_shader_module, swapchain.GetExtent(), render_pass);
	
		vkDestroyShaderModule(device_cfg.logical_device, frag_shader_module, nullptr);
		vkDestroyShaderModule(device_cfg.logical_device, vert_shader_module, nullptr);


		std::vector<VkDescriptorSet> descriptor_sets;

		descriptor_pool.AllocateSet(pipeline.GetDescriptorSetLayout(), frames_cnt, descriptor_sets);

		std::vector<Buffer> uniform_buffers;

		Image image = Image::FromFile(device_cfg, "../images/test.jpg");
		ImageView image_view(device_cfg, image);
		Sampler sampler(device_cfg);

		for (uint32_t i = 0; i < frames_cnt; i++)
		{
			uniform_buffers.emplace_back(device_cfg, sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, std::vector<uint32_t>());

			VkDescriptorBufferInfo buffer_info{};
			buffer_info.buffer = uniform_buffers.back().GetHandle();
			buffer_info.offset = 0;
			buffer_info.range = sizeof(UniformBufferObject);

			VkDescriptorImageInfo image_info{};
			image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			image_info.imageView = image_view.GetHandle();
			image_info.sampler = sampler.GetSamplerHandle();

			std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

			descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[0].dstSet = descriptor_sets[i];
			descriptor_writes[0].dstBinding = 0;
			descriptor_writes[0].dstArrayElement = 0;
			descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptor_writes[0].descriptorCount = 1;
			descriptor_writes[0].pBufferInfo = &buffer_info;

			descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[1].dstSet = descriptor_sets[i];
			descriptor_writes[1].dstBinding = 1;
			descriptor_writes[1].dstArrayElement = 0;
			descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptor_writes[1].descriptorCount = 1;
			descriptor_writes[1].pImageInfo = &image_info;

			vkUpdateDescriptorSets(device_cfg.logical_device, 2, descriptor_writes.data(), 0, nullptr);
		}

		images_.push_back(std::move(image));
		image_views_.push_back(std::move(image_view));
		samplers_.push_back(std::move(sampler));

		batches_.emplace_back(std::move(pipeline), std::move(vertices_gpu_buffer), std::move(faces_gpu_buffer), std::move(uniform_buffers), descriptor_sets, 3* faces.size());
	}

	{
		Buffer vertices_gpu_buffer(device_cfg, sizeof(vertices[0]) * vertices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, queue_indices);
		Buffer faces_gpu_buffer(device_cfg, sizeof(faces[0]) * faces.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, queue_indices);

		CopyToGPUBuffer(device_cfg, vertices_gpu_buffer, static_cast<const void*>(vertices.data()), sizeof(vertices[0]) * vertices.size());
		CopyToGPUBuffer(device_cfg, faces_gpu_buffer, static_cast<const void*>(faces.data()), sizeof(faces[0]) * faces.size());

		auto vert_shader_code = readFile("../shaders/vert_2.spv");
		auto frag_shader_code = readFile("../shaders/frag_2.spv");

		VkShaderModule vert_shader_module = CreateShaderModule(vert_shader_code);
		VkShaderModule frag_shader_module = CreateShaderModule(frag_shader_code);

		GraphicsPipeline pipeline(device_cfg.logical_device, vert_shader_module, frag_shader_module, swapchain.GetExtent(), render_pass);

		vkDestroyShaderModule(device_cfg.logical_device, frag_shader_module, nullptr);
		vkDestroyShaderModule(device_cfg.logical_device, vert_shader_module, nullptr);


		std::vector<VkDescriptorSet> descriptor_sets;

		descriptor_pool.AllocateSet(pipeline.GetDescriptorSetLayout(), frames_cnt, descriptor_sets);

		std::vector<Buffer> uniform_buffers;

		Image image = Image::FromFile(device_cfg, "../images/test_2.jpg");
		ImageView image_view(device_cfg, image);
		Sampler sampler(device_cfg);

		for (uint32_t i = 0; i < frames_cnt; i++)
		{
			uniform_buffers.emplace_back(device_cfg, sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, std::vector<uint32_t>());

			VkDescriptorBufferInfo buffer_info{};
			buffer_info.buffer = uniform_buffers.back().GetHandle();
			buffer_info.offset = 0;
			buffer_info.range = sizeof(UniformBufferObject);

			VkDescriptorImageInfo image_info{};
			image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			image_info.imageView = image_view.GetHandle();
			image_info.sampler = sampler.GetSamplerHandle();

			std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

			descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[0].dstSet = descriptor_sets[i];
			descriptor_writes[0].dstBinding = 0;
			descriptor_writes[0].dstArrayElement = 0;
			descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptor_writes[0].descriptorCount = 1;
			descriptor_writes[0].pBufferInfo = &buffer_info;

			descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[1].dstSet = descriptor_sets[i];
			descriptor_writes[1].dstBinding = 1;
			descriptor_writes[1].dstArrayElement = 0;
			descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptor_writes[1].descriptorCount = 1;
			descriptor_writes[1].pImageInfo = &image_info;

			vkUpdateDescriptorSets(device_cfg.logical_device, 2, descriptor_writes.data(), 0, nullptr);
		}

		images_.push_back(std::move(image));
		image_views_.push_back(std::move(image_view));
		samplers_.push_back(std::move(sampler));

		batches_.emplace_back(std::move(pipeline), std::move(vertices_gpu_buffer), std::move(faces_gpu_buffer), std::move(uniform_buffers), descriptor_sets, 3 * faces.size());
	}

}

std::vector<std::reference_wrapper<render::Batch>> render::BatchesManager::GetBatches()
{
	std::vector<std::reference_wrapper<render::Batch>> result;

	for (auto&& batch : batches_)
	{
		result.push_back(batch);
	}

	return result;
}

uint32_t render::BatchesManager::GetUniformSetCnt()
{
	return 2;
}

uint32_t render::BatchesManager::GetSamplerSetCnt()
{
	return 2;
}

void render::BatchesManager::CopyToGPUBuffer(const DeviceConfiguration& device_cfg, const Buffer & dst_buffer, const void * data, uint64_t size)
{
	Buffer staging_buffer(device_cfg, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {});

	staging_buffer.LoadData(data, size);

	device_cfg.transfer_cmd_pool->ExecuteOneTimeCommand([size, &staging_buffer, &dst_buffer](VkCommandBuffer command_buffer) {

		VkBufferCopy copy_region{};
		copy_region.srcOffset = 0; // Optional
		copy_region.dstOffset = 0; // Optional
		copy_region.size = size;
		vkCmdCopyBuffer(command_buffer, staging_buffer.GetHandle(), dst_buffer.GetHandle(), 1, &copy_region);

		});
}

VkShaderModule render::BatchesManager::CreateShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shader_module;
	if (vkCreateShaderModule(device_, &createInfo, nullptr, &shader_module) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shader_module;
}
