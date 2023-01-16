#include "batches_manager.h"

#include <fstream>

#include "gltf_wrapper.h"

#include "command_pool.h"

namespace render
{


	BatchesManager::BatchesManager(const Global& global) : RenderObjBase(global)
	{

		//images_.push_back(Image::FromFile(global, "../images/textures/CaveEnv.png"));
		//image_views_.push_back(ImageView(global, images_.back()));

	//	const std::vector<Vertex> vertices = {
	//{{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
	//{{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
	//{{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
	//{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
	//
	//{{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
	//{{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
	//{{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
	//{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
	//	};
	//
	//	const std::vector<Face> faces = {
	//		{0, 1, 2}, {2, 3, 0},
	//		{4, 5, 6}, {6, 7, 4}
	//	};

		gltf_wrappers_.reserve(16);
#if 0


		{

			gltf_wrappers_.push_back(GLTFWrapper(global, "../blender/old_chair/old_chair_with_cube.glb"));

			auto&& wrapper = gltf_wrappers_.back();


			for (auto&& mesh : wrapper.meshes)
			{
				meshes_.push_back(mesh);
			}
		}

		{

			gltf_wrappers_.push_back(GLTFWrapper(global, "../blender/phil/phil.glb"));

			auto&& wrapper = gltf_wrappers_.back();


			for (auto&& mesh : wrapper.meshes)
			{
				meshes_.push_back(mesh);
			}

			//animators_.push_back(Animator(gltf_wrappers_.back().animations["sit"], gltf_wrappers_.back().nodes));
			//animators_.back().Start();

			animators_.push_back(Animator(gltf_wrappers_.back().animations.at("breath "), gltf_wrappers_.back().nodes));
			animators_.back().Start();

			//animators_.push_back(Animator(gltf_wrappers_.back().animations["test_move"], gltf_wrappers_.back().nodes));
			//animators_.back().Start();
		}

		{

			gltf_wrappers_.push_back(GLTFWrapper(global, "../blender/room/room.glb"));

			auto&& wrapper = gltf_wrappers_.back();


			for (auto&& mesh : wrapper.meshes)
			{
				meshes_.push_back(mesh);
			}
		}

		{

			gltf_wrappers_.push_back(GLTFWrapper(global, "../blender/sky/sky.glb"));

			auto&& wrapper = gltf_wrappers_.back();


			for (auto&& mesh : wrapper.meshes)
			{
				meshes_.push_back(mesh);
			}
		}
#endif
		//{
		//	GPULocalBuffer vertices_gpu_buffer(global, sizeof(vertices[0]) * vertices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, queue_indices);
		//	GPULocalBuffer faces_gpu_buffer(global, sizeof(faces[0]) * faces.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, queue_indices);
		//	
		//	CopyToGPUBuffer(global, vertices_gpu_buffer, static_cast<const void*>(vertices.data()), sizeof(vertices[0]) * vertices.size());
		//	CopyToGPUBuffer(global, faces_gpu_buffer, static_cast<const void*>(faces.data()), sizeof(faces[0]) * faces.size());

		//	auto vert_shader_code = readFile("../shaders/vert.spv");
		//	auto frag_shader_code = readFile("../shaders/frag.spv");

		//	VkShaderModule vert_shader_module = CreateShaderModule(vert_shader_code);
		//	VkShaderModule frag_shader_module = CreateShaderModule(frag_shader_code);

		//	VertexBindingDesc binding =
		//	{
		//		sizeof(Vertex),
		//		{
		//			{ VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)},
		//			{ VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)},
		//			{ VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, tex_coord)},
		//		}
		//	};

		//	VertexBindings vertex_bindings = { binding };

		//	GraphicsPipeline pipeline(global.logical_device, vert_shader_module, frag_shader_module, swapchain.GetExtent(), render_pass, vertex_bindings);
		//
		//	vkDestroyShaderModule(global.logical_device, frag_shader_module, nullptr);
		//	vkDestroyShaderModule(global.logical_device, vert_shader_module, nullptr);


		//	std::vector<VkDescriptorSet> descriptor_sets;

		//	descriptor_pool.AllocateSet(pipeline.GetDescriptorSetLayout(), frames_cnt, descriptor_sets);

		//	std::vector<Buffer> uniform_buffers;

		//	Image image = Image::FromFile(global, "../images/test.jpg");
		//	ImageView image_view(global, image);
		//	Sampler sampler(global);

		//	for (uint32_t i = 0; i < frames_cnt; i++)
		//	{
		//		uniform_buffers.emplace_back(global, sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, std::vector<uint32_t>());

		//		VkDescriptorBufferInfo buffer_info{};
		//		buffer_info.buffer = uniform_buffers.back().GetHandle();
		//		buffer_info.offset = 0;
		//		buffer_info.range = sizeof(UniformBufferObject);

		//		VkDescriptorImageInfo image_info{};
		//		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		//		image_info.imageView = image_view.GetHandle();
		//		image_info.sampler = sampler.GetSamplerHandle();

		//		std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

		//		descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//		descriptor_writes[0].dstSet = descriptor_sets[i];
		//		descriptor_writes[0].dstBinding = 0;
		//		descriptor_writes[0].dstArrayElement = 0;
		//		descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		//		descriptor_writes[0].descriptorCount = 1;
		//		descriptor_writes[0].pBufferInfo = &buffer_info;

		//		descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//		descriptor_writes[1].dstSet = descriptor_sets[i];
		//		descriptor_writes[1].dstBinding = 1;
		//		descriptor_writes[1].dstArrayElement = 0;
		//		descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		//		descriptor_writes[1].descriptorCount = 1;
		//		descriptor_writes[1].pImageInfo = &image_info;

		//		vkUpdateDescriptorSets(global.logical_device, 2, descriptor_writes.data(), 0, nullptr);
		//	}

		//	images_.push_back(std::move(image));
		//	image_views_.push_back(std::move(image_view));
		//	samplers_.push_back(std::move(sampler));

		//	buffers_.push_back(std::move(vertices_gpu_buffer));
		//	auto&& vert_buf = buffers_.back();

		//	buffers_.push_back(std::move(faces_gpu_buffer));
		//	auto&& faces_buf = buffers_.back();

		//	Batch batch(std::move(pipeline), { vert_buf }, faces_buf, std::move(uniform_buffers), descriptor_sets, 3 * faces.size());

		//	batches_.emplace_back(std::move(batch));
		//}

		//{
		//	GPULocalBuffer vertices_gpu_buffer(global, sizeof(vertices[0]) * vertices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, queue_indices);
		//	GPULocalBuffer faces_gpu_buffer(global, sizeof(faces[0]) * faces.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, queue_indices);

		//	CopyToGPUBuffer(global, vertices_gpu_buffer, static_cast<const void*>(vertices.data()), sizeof(vertices[0]) * vertices.size());
		//	CopyToGPUBuffer(global, faces_gpu_buffer, static_cast<const void*>(faces.data()), sizeof(faces[0]) * faces.size());

		//	auto vert_shader_code = readFile("../shaders/vert_2.spv");
		//	auto frag_shader_code = readFile("../shaders/frag_2.spv");

		//	VkShaderModule vert_shader_module = CreateShaderModule(vert_shader_code);
		//	VkShaderModule frag_shader_module = CreateShaderModule(frag_shader_code);

		//	VertexBindingDesc binding =
		//	{
		//		sizeof(Vertex),
		//		{
		//			{ VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)},
		//			{ VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)},
		//			{ VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, tex_coord)},
		//		}
		//	};

		//	VertexBindings vertex_bindings = { binding };

		//	GraphicsPipeline pipeline(global.logical_device, vert_shader_module, frag_shader_module, swapchain.GetExtent(), render_pass, vertex_bindings);

		//	vkDestroyShaderModule(global.logical_device, frag_shader_module, nullptr);
		//	vkDestroyShaderModule(global.logical_device, vert_shader_module, nullptr);


		//	std::vector<VkDescriptorSet> descriptor_sets;

		//	descriptor_pool.AllocateSet(pipeline.GetDescriptorSetLayout(), frames_cnt, descriptor_sets);

		//	std::vector<Buffer> uniform_buffers;

		//	Image image = Image::FromFile(global, "../images/test_2.jpg");
		//	ImageView image_view(global, image);


		//	for (uint32_t i = 0; i < frames_cnt; i++)
		//	{
		//		uniform_buffers.emplace_back(global, sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, std::vector<uint32_t>());

		//		VkDescriptorBufferInfo buffer_info{};
		//		buffer_info.buffer = uniform_buffers.back().GetHandle();
		//		buffer_info.offset = 0;
		//		buffer_info.range = sizeof(UniformBufferObject);

		//		VkDescriptorImageInfo image_info{};
		//		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		//		image_info.imageView = image_view.GetHandle();
		//		image_info.sampler = sampler.GetSamplerHandle();

		//		std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

		//		descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//		descriptor_writes[0].dstSet = descriptor_sets[i];
		//		descriptor_writes[0].dstBinding = 0;
		//		descriptor_writes[0].dstArrayElement = 0;
		//		descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		//		descriptor_writes[0].descriptorCount = 1;
		//		descriptor_writes[0].pBufferInfo = &buffer_info;

		//		descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//		descriptor_writes[1].dstSet = descriptor_sets[i];
		//		descriptor_writes[1].dstBinding = 1;
		//		descriptor_writes[1].dstArrayElement = 0;
		//		descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		//		descriptor_writes[1].descriptorCount = 1;
		//		descriptor_writes[1].pImageInfo = &image_info;

		//		vkUpdateDescriptorSets(global.logical_device, 2, descriptor_writes.data(), 0, nullptr);
		//	}

		//	images_.push_back(std::move(image));
		//	image_views_.push_back(std::move(image_view));
		//	samplers_.push_back(std::move(sampler));

		//	buffers_.push_back(std::move(vertices_gpu_buffer));
		//	auto&& vert_buf = buffers_.back();

		//	buffers_.push_back(std::move(faces_gpu_buffer));
		//	auto&& faces_buf = buffers_.back();

		//	Batch batch(std::move(pipeline), { vert_buf }, faces_buf, std::move(uniform_buffers), descriptor_sets, 3 * faces.size());

		//	batches_.emplace_back(std::move(batch));
		//}

	}

	//const std::vector<std::reference_wrapper<Mesh>>& BatchesManager::GetMeshes() const
	//{
	//	//std::vector<Batch> result;

	//	//for (auto&& batch : batches_)
	//	//{
	//	//	result.push_back(batch);
	//	//}

	//	return meshes_;
	//}

	
	std::vector<ModelPack>& BatchesManager::GetModelPacks()
	{
		return gltf_wrappers_;
	}

	const ImageView& BatchesManager::GetEnvImageView() const
	{
		return image_views_.front();
	}

	void BatchesManager::Update()
	{
		for (auto&& animator : animators_)
		{
			animator.Update();
		}
	}

	void BatchesManager::Add(const tinygltf::Model& model, DescriptorSetsManager& manager, const RenderSetup& render_setup)
	{
		gltf_wrappers_.push_back(GLTFWrapper(global_, model, manager, render_setup));

		auto&& wrapper = gltf_wrappers_.back();


		for (auto&& mesh : wrapper.meshes)
		{
			meshes_.push_back(mesh);
		}
	}

	glm::mat4 Node::GetGlobalTransformMatrix() const
	{
		if (parent)
			return parent->GetGlobalTransformMatrix() * local_transform;

		return local_transform;
	}
}