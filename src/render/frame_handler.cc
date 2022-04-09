#include "frame_handler.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <utility>

#include "vk_util.h"
#include <render/data_types.h>
#include <render/command_buffer_filler.h>

//struct UniformBufferObject {
//	glm::mat4 model;
//	glm::mat4 view;
//	glm::mat4 proj;
//};

struct CameraUniformBufferObject
{
	glm::vec4 position;
	glm::mat4 project_view_matrix;
};

render::FrameHandler::FrameHandler(const DeviceConfiguration& device_cfg, const Swapchain& swapchain) :
	RenderObjBase(device_cfg), swapchain_(swapchain.GetHandle()), graphics_queue_(device_cfg.graphics_queue),
	command_buffer_(device_cfg.graphics_cmd_pool->GetCommandBuffer()),
	image_available_semaphore_(vk_util::CreateSemaphore(device_cfg.logical_device)),
	render_finished_semaphore_(vk_util::CreateSemaphore(device_cfg.logical_device)),
	cmd_buffer_fence_(vk_util::CreateFence(device_cfg.logical_device)), present_info_{}, submit_info_{}, wait_stages_(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT),
	color_sampler_(device_cfg),
	camera_uniform_buffer_(device_cfg, sizeof(CameraUniformBufferObject))
{

	for (int i = 0; i < 20; i++)
	{
		model_uniform_buffers_.push_back(UniformBuffer(device_cfg, sizeof(glm::mat4)));
	}

	handle_ = (void*)(1);
}

bool render::FrameHandler::FillCommandBuffer(const Framebuffer& swapchain_framebuffer, const PipelineCollection& pipeline_collection, const BatchesManager& batches_manager)
{
	vkResetCommandBuffer(command_buffer_, 0);


	return true;
}

bool render::FrameHandler::Draw(const Framebuffer& swapchain_framebuffer, uint32_t image_index, const PipelineCollection& pipeline_collection, const BatchesManager& batches_manager, glm::vec3 pos, glm::vec3 look)
{
	if (model_descriptor_sets_.size() == 0)
	{
		std::vector<VkDescriptorSet> camera_descriptor_set;
		device_cfg_.descriptor_pool->AllocateSet(pipeline_collection.GetDescriptorSetLayout(PipelineCollection::DescriptorSetLayoutId::kCameraMatrices).GetHandle(), 1, camera_descriptor_set);
		camera_descriptor_set_ = camera_descriptor_set[0];

		model_descriptor_sets_.resize(batches_manager.GetBatches().size());
		device_cfg_.descriptor_pool->AllocateSet(pipeline_collection.GetDescriptorSetLayout(PipelineCollection::DescriptorSetLayoutId::kObjectSet).GetHandle(), batches_manager.GetBatches().size(), model_descriptor_sets_);

		material_descriptor_sets_.resize(batches_manager.GetBatches().size());
		device_cfg_.descriptor_pool->AllocateSet(pipeline_collection.GetDescriptorSetLayout(PipelineCollection::DescriptorSetLayoutId::kMaterialSet).GetHandle(), batches_manager.GetBatches().size(), material_descriptor_sets_);

		for (int i = 0; i < batches_manager.GetBatches().size(); i++)
		{
			UpdateModelDescriptorSet(i, batches_manager.GetBatches()[i]);
			UpdateMaterialDescriptorSet(i, batches_manager.GetBatches()[i], batches_manager.GetEnvImageView());
		}
	}

	CommandBufferFiller command_filler(pipeline_collection, batches_manager);

	submit_info_.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	submit_info_.waitSemaphoreCount = 1;

	submit_info_.pWaitDstStageMask = &wait_stages_;

	submit_info_.commandBufferCount = 1;
	submit_info_.pCommandBuffers = &command_buffer_;

	submit_info_.signalSemaphoreCount = 1;

	submit_info_.pSignalSemaphores = &render_finished_semaphore_;


	present_info_.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	present_info_.waitSemaphoreCount = 1;
	present_info_.pWaitSemaphores = &render_finished_semaphore_;

	present_info_.swapchainCount = 1;
	present_info_.pSwapchains = &swapchain_;

	present_info_.pResults = nullptr; // Optional


	submit_info_.pWaitSemaphores = &image_available_semaphore_;

	vkWaitForFences(device_cfg_.logical_device, 1, &cmd_buffer_fence_, VK_TRUE, UINT64_MAX);
	vkResetFences(device_cfg_.logical_device, 1, &cmd_buffer_fence_);

	UpdateCameraDescriptorSet(pos, look);

	std::vector<std::reference_wrapper<const Framebuffer>> framebuffers =
	{
		swapchain_framebuffer
	};

	std::vector<RenderPassInfo> render_info =
	{
		{
			PipelineCollection::RenderPassId::kScreen,
			FramebufferId::kScreen,
			{
				PipelineCollection::PipelineId::kColor
			}
		}
	};

	DescriptorSets descriptor_sets =
	{
		camera_descriptor_set_,
		model_descriptor_sets_,
		material_descriptor_sets_
	};

	
	command_filler.Fill(command_buffer_, framebuffers, render_info, descriptor_sets);

	/*FillCommandBuffer(swapchain_framebuffer, pipeline_collection, batches_manager);*/

	//if (acquire_semaphore_prev != VK_NULL_HANDLE)
	//{
	//	vkDestroySemaphore(device_cfg_.logical_device, acquire_semaphore_prev, nullptr);
	//}

	//acquire_semaphore_prev = image_acquire_semaphore;

	present_info_.pImageIndices = &image_index;

	if (vkQueueSubmit(graphics_queue_, 1, &submit_info_, cmd_buffer_fence_) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	return vkQueuePresentKHR(graphics_queue_, &present_info_) == VK_SUCCESS;
}

VkSemaphore render::FrameHandler::GetImageAvailableSemaphore() const
{
	return image_available_semaphore_;
}


render::FrameHandler::~FrameHandler()
{
	if (handle_ != nullptr)
	{
		vkDestroyFence(device_cfg_.logical_device, cmd_buffer_fence_, nullptr);
		vkDestroySemaphore(device_cfg_.logical_device, render_finished_semaphore_, nullptr);
	}
}

void render::FrameHandler::UpdateCameraDescriptorSet(glm::vec3 pos, glm::vec3 look)
{
	CameraUniformBufferObject camera_ubo{};

	camera_ubo.position = glm::vec4(pos, 1.0f);
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), 16.0f / 9.f, 0.1f, 100.0f);
	proj[1][1] *= -1;
	camera_ubo.project_view_matrix = proj * glm::lookAt(pos, pos + look, glm::vec3(0.0f, 0.0f, 1.0f));

	void* data;
	vkMapMemory(device_cfg_.logical_device, camera_uniform_buffer_.GetBufferMemory(), 0, sizeof(CameraUniformBufferObject), 0, &data);
	memcpy(data, &camera_ubo, sizeof(camera_ubo));
	vkUnmapMemory(device_cfg_.logical_device, camera_uniform_buffer_.GetBufferMemory());


	VkDescriptorBufferInfo camera_buffer_info{};
	camera_buffer_info.buffer = camera_uniform_buffer_.GetHandle();
	camera_buffer_info.offset = 0;
	camera_buffer_info.range = sizeof(CameraUniformBufferObject);

	std::array<VkWriteDescriptorSet, 1> descriptor_writes{};

	descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_writes[0].dstSet = camera_descriptor_set_;
	descriptor_writes[0].dstBinding = 0;
	descriptor_writes[0].dstArrayElement = 0;
	descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor_writes[0].descriptorCount = 1;
	descriptor_writes[0].pBufferInfo = &camera_buffer_info;

	vkUpdateDescriptorSets(device_cfg_.logical_device, descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
}

void render::FrameHandler::UpdateModelDescriptorSet(uint32_t model_descriptor_set_index, const Batch& batch)
{
	VkDescriptorBufferInfo model_buffer_info{};
	model_buffer_info.buffer = model_uniform_buffers_[model_descriptor_set_index].GetHandle();
	model_buffer_info.offset = 0;
	model_buffer_info.range = sizeof(glm::mat4);

	std::array<VkWriteDescriptorSet, 1> descriptor_writes{};

	descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_writes[0].dstSet = model_descriptor_sets_[model_descriptor_set_index];
	descriptor_writes[0].dstBinding = 0;
	descriptor_writes[0].dstArrayElement = 0;
	descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor_writes[0].descriptorCount = 1;
	descriptor_writes[0].pBufferInfo = &model_buffer_info;

	vkUpdateDescriptorSets(device_cfg_.logical_device, descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);

	void* data;
	vkMapMemory(device_cfg_.logical_device, model_uniform_buffers_[model_descriptor_set_index].GetBufferMemory(), 0, sizeof(glm::mat4), 0, &data);
	memcpy(data, &(batch.GetModelMatrix()), sizeof(glm::mat4));
	vkUnmapMemory(device_cfg_.logical_device, model_uniform_buffers_[model_descriptor_set_index].GetBufferMemory());
}

void render::FrameHandler::UpdateMaterialDescriptorSet(uint32_t material_descriptor_set_index, const Batch& batch, const render::ImageView& env_image_view)
{
	VkDescriptorImageInfo color_image_info{};
	color_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	color_image_info.imageView = batch.GetColorImageView().GetHandle();
	color_image_info.sampler = color_sampler_.GetHandle();

	VkDescriptorImageInfo env_image_info{};
	env_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	env_image_info.imageView = env_image_view.GetHandle();
	env_image_info.sampler = color_sampler_.GetHandle();

	std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

	descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_writes[0].dstSet = material_descriptor_sets_[material_descriptor_set_index];
	descriptor_writes[0].dstBinding = 0;
	descriptor_writes[0].dstArrayElement = 0;
	descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptor_writes[0].descriptorCount = 1;
	descriptor_writes[0].pImageInfo = &color_image_info;

	descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_writes[1].dstSet = material_descriptor_sets_[material_descriptor_set_index];
	descriptor_writes[1].dstBinding = 1;
	descriptor_writes[1].dstArrayElement = 0;
	descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptor_writes[1].descriptorCount = 1;
	descriptor_writes[1].pImageInfo = &env_image_info;

	vkUpdateDescriptorSets(device_cfg_.logical_device, descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
}

