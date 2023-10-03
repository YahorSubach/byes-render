#include "frame_handler.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <utility>

#include "vk_util.h"
#include <render/data_types.h>

#include "global.h"
namespace render
{
	FrameHandler::FrameHandler(const Global& global, const Swapchain& swapchain, const RenderSetup& render_setup,
		const std::array<Extent, kExtentTypeCnt>& extents, DescriptorSetsManager& descriptor_set_manager) :
		RenderObjBase(global), swapchain_(swapchain.GetHandle()), graphics_queue_(global.graphics_queue),
		command_buffer_(global.graphics_cmd_pool->GetCommandBuffer()),
		image_available_semaphore_(vk_util::CreateSemaphore(global.logical_device)),
		render_finished_semaphore_(vk_util::CreateSemaphore(global.logical_device)),
		cmd_buffer_fence_(vk_util::CreateFence(global.logical_device)), present_info_{}, submit_info_{}, wait_stages_(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT),
		render_setup_(render_setup),
		render_graph_handler_(global, render_setup.GetRenderGraph(), extents, descriptor_set_manager),
		descriptor_set_manager_(descriptor_set_manager)
	{
		handle_ = (void*)(1);
	}

	extern void FreeMemory(VkDevice logical_device, OffsettedMemory memory);

	bool FrameHandler::Draw(const FrameInfo& frame_info, const Scene& scene)
	{
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

		vkWaitForFences(global_.logical_device, 1, &cmd_buffer_fence_, VK_TRUE, UINT64_MAX);
		vkResetFences(global_.logical_device, 1, &cmd_buffer_fence_);

		{

			std::vector<std::pair<uint32_t, std::variant<VkBuffer, OffsettedMemory, VkImageView, VkDescriptorSet>>> new_delete_list;

			for (auto&& [frame_ind, handle_variant] : global_.delete_list)
			{
				if (frame_ind == global_.frame_ind)
				{
					if (void* handle = std::get_if<VkBuffer>(&handle_variant))
					{
						if (*(VkBuffer*)handle == (VkBuffer)0x41862000000117e)
						{
							int a = 1;
						}

						vkDestroyBuffer(global_.logical_device, *(VkBuffer*)handle, nullptr);
					}
					else if (void* handle = std::get_if<OffsettedMemory>(&handle_variant))
					{
						//vkFreeMemory(global_.logical_device, *(VkDeviceMemory*)handle, nullptr);
						FreeMemory(global_.logical_device, *(OffsettedMemory*)handle);
					}
					else if (void* handle = std::get_if<VkImageView>(&handle_variant))
					{
						vkDestroyImageView(global_.logical_device, *(VkImageView*)handle, nullptr);
					}
					else if (void* handle = std::get_if<VkDescriptorSet>(&handle_variant))
					{
						descriptor_set_manager_.FreeDescriptorSet(*(VkDescriptorSet*)handle);
					}
				}
				else
				{
					new_delete_list.push_back({ frame_ind, handle_variant });
				}
			}

			global_.delete_list = std::move(new_delete_list);

		}


		render_graph_handler_.FillCommandBuffer(command_buffer_, frame_info, scene);


		present_info_.pImageIndices = &frame_info.swapchain_image_index;

		if (vkQueueSubmit(graphics_queue_, 1, &submit_info_, cmd_buffer_fence_) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		return vkQueuePresentKHR(graphics_queue_, &present_info_) == VK_SUCCESS;
	}

	VkSemaphore FrameHandler::GetImageAvailableSemaphore() const
	{
		return image_available_semaphore_;
	}


	FrameHandler::~FrameHandler()
	{
		if (handle_ != nullptr)
		{
			vkWaitForFences(global_.logical_device, 1, &cmd_buffer_fence_, VK_TRUE, UINT64_MAX);
			vkDestroyFence(global_.logical_device, cmd_buffer_fence_, nullptr);
			vkDestroySemaphore(global_.logical_device, render_finished_semaphore_, nullptr);
		}
	}
}