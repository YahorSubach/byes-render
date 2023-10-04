#include "render_system.h"

#include "platform.h"

PFN_vkCmdDebugMarkerBeginEXT pfnCmdDebugMarkerBegin;
PFN_vkCmdDebugMarkerEndEXT pfnCmdDebugMarkerEnd;


namespace render
{
	RenderSystem::RenderSystem(platform::Window window, const std::string& app_name): 
		render_api_(global_, app_name), render_setup_(global_),
		surface_(window, render_api_.GetInstance(), global_)
	{
		render_api_.FillGlobal(global_);

		pfnCmdDebugMarkerBegin = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(global_.logical_device, "vkCmdDebugMarkerBeginEXT");
		pfnCmdDebugMarkerEnd = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(global_.logical_device, "vkCmdDebugMarkerEndEXT");

		descriptor_set_manager_.emplace(global_);

		formats_ =
		{
			surface_.GetSurfaceFormat(global_.physical_device).format,
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_FORMAT_D32_SFLOAT
		};

		render_setup_.BuildRenderPasses(formats_);
	}


	void RenderSystem::Render(uint32_t frame_index, const Scene& scene)
	{
		if (!swapchain_)
		{
			global_.graphics_cmd_pool->ClearCommandBuffers();
			global_.graphics_cmd_pool->CreateCommandBuffers(kFramesCount);

			swapchain_.emplace(global_, surface_);

			auto&& swapchain = swapchain_.value();

			auto swapchain_extent = swapchain.GetExtent();
			extents_[int(ExtentType::kPresentation)] = swapchain_extent;
			extents_[int(ExtentType::kViewport)] = swapchain_extent;
			extents_[int(ExtentType::kShadowMap)] = {512, 512};

			render_setup_.InitPipelines(descriptor_set_manager_.value(), extents_);

			for (int i = 0; i < swapchain.GetImagesCount(); i++)
			{
				Framebuffer::ConstructParams params
				{
					render_setup_.GetSwapchainRenderPass(),
					swapchain_extent
				};

				params.attachments.push_back(swapchain.GetImageView(i));

				swapchain_framebuffers_[i].emplace(global_, params);
			}

			formats_[int(FormatType::kSwapchain)] = surface_.GetSurfaceFormat(global_.physical_device).format;

			for (int i = 0; i < kFramesCount; i++)
			{
				frames_[i].emplace(global_, swapchain, render_setup_, extents_, formats_, descriptor_set_manager_.value());
			}

			for (auto&& callback : on_swapchain_update_callbacks)
			{
				std::invoke(callback, swapchain);
			}
		}

		auto&& swapchain = swapchain_.value();


		global_.frame_ind = frame_index;

		uint32_t swapchain_image_index;

		auto&& frame = frames_[frame_index].value();

		VkResult result = vkAcquireNextImageKHR(global_.logical_device, swapchain.GetHandle(), UINT64_MAX,
			frame.GetImageAvailableSemaphore(), VK_NULL_HANDLE, &swapchain_image_index);

		if (result != VK_SUCCESS)
		{
			if (result == VK_ERROR_OUT_OF_DATE_KHR)
			{
				swapchain_.reset();
				vkDeviceWaitIdle(global_.logical_device);
				return;
			}
			
			DebugBreak();
		}

		FrameInfo frame_info
		{
			swapchain_framebuffers_[swapchain_image_index].value(),
			swapchain.GetImage(swapchain_image_index),
			swapchain_image_index,
			frame_index
		};

		auto swapchain_extent = swapchain_.value().GetExtent();
		float aspect_ratio = 1.0f * swapchain_extent.width / swapchain_extent.height;


		if (!frame.Draw(frame_info, scene))
		{
			swapchain_.reset();
			vkDeviceWaitIdle(global_.logical_device);
			return;
		}
	}

	const Global& RenderSystem::GetGlobal() const
	{
		return global_;
	}

	DescriptorSetsManager& RenderSystem::GetDescriptorSetsManager()
	{
		return descriptor_set_manager_.value();
	}

	void RenderSystem::AddOnSwapchainUpdateCallback(std::function<void(const Swapchain&)> callback)
	{
		on_swapchain_update_callbacks.push_back(callback);
	}

	bool RenderSystem::ShouldRender() const
	{
		return !surface_.Closed();
	}



}
