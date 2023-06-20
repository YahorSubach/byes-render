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

#include "global.h"

render::FrameHandler::FrameHandler(const Global& global, const Swapchain& swapchain, const RenderSetup& render_setup,
	const std::array<Extent, kExtentTypeCnt>& extents, DescriptorSetsManager& descriptor_set_manager, 
	const ui::UI& ui, const SceneImpl& scene) :
	RenderObjBase(global), swapchain_(swapchain.GetHandle()), graphics_queue_(global.graphics_queue),
	command_buffer_(global.graphics_cmd_pool->GetCommandBuffer()),
	image_available_semaphore_(vk_util::CreateSemaphore(global.logical_device)),
	render_finished_semaphore_(vk_util::CreateSemaphore(global.logical_device)),
	cmd_buffer_fence_(vk_util::CreateFence(global.logical_device)), present_info_{}, submit_info_{}, wait_stages_(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT),
	//model_scene_(global, scene),
	render_setup_(render_setup),
	render_graph_handler_(global, render_setup.GetRenderGraph(), extents, descriptor_set_manager),
	//ui_scene_(global, ui),
	ui_(ui)
{

	//model_scene_.UpdateData();
	//ui_scene_.UpdateData();

	//model_scene_.AttachDescriptorSets(descriptor_set_manager);
	//ui_scene_.AttachDescriptorSets(descriptor_set_manager);

	handle_ = (void*)(1);
}

//void render::FrameHandler::AddModel(const render::Mesh& model)
//{
//	model_scene_.AddModel(model);
//}

bool render::FrameHandler::Draw(const FrameInfo& frame_info, /*Scene::*/SceneImpl& scene)
{
	//scene.Update(frame_info.swapchain_image_index);
	//CommandBufferFiller command_filler(render_setup, framebuffer_collection);

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
	//model_scene_.UpdateCameraData(scene pos, look, 1.0f * swapchain_framebuffer.GetExtent().width / swapchain_framebuffer.GetExtent().height);

	//model_scene_.UpdateData();
	//ui_scene_.UpdateData();


//	std::vector<RenderPassInfo> render_info =
//	{
//		{
//			RenderPassId::kBuildDepthmap,
//			FramebufferId::kDepth,
//			{
//				{
//					PipelineId::kDepth,
//					RenderModelType::kStatic,
//					model_scene_.GetRenderNode()
//				},
//				{
//					PipelineId::kDepthSkinned,
//					RenderModelType::kSkinned,
//					model_scene_.GetRenderNode()
//				}
//			}
//		},
//
//		//{
//		//	RenderPassId::kSimpleRenderToScreen,
//		//	FramebufferId::kScreen,
//		//	{
//		//		{
//		//			PipelineId::kColor,
//		//			RenderModelType::kStatic,
//		//			model_scene_.GetRenderNode()
//		//		},
//		//		{
//		//			PipelineId::kColorSkinned,
//		//			RenderModelType::kSkinned,
//		//			model_scene_.GetRenderNode()
//		//		},
//		//		{
//		//			PipelineId::kUI,
//		//			RenderModelType::kStatic,
//		//			ui_scene_.GetRenderNode()
//		//		},
//		//	}
//		//},
//
//		{
//			RenderPassId::kBuildGBuffers,
//			FramebufferId::kGBuffers,
//			{
//				{
//					PipelineId::kBuildGBuffers,
//					RenderModelType::kStatic,
//					model_scene_.GetRenderNode()
//				}
//			}
//		},
//
//		{
//			RenderPassId::kCollectGBuffers,
//			FramebufferId::kScreen,
//			{
//				{
//					PipelineId::kCollectGBuffers,
//					RenderModelType::kStatic,
//					model_scene_.GetRenderNode()
//				}
//			}
//		},
////
////		{
////	RenderPassId::kSimpleRenderToScreen,
////	FramebufferId::kScreen,
////	{
////		{
////			PipelineId::kUI,
////			RenderModelType::kStatic,
////			ui_scene_.GetRenderNode()
////		}
////	}
////},
//	};

	//std::vector<render::RenderModel> render_models;
	//render_models.reserve(64);
	//for (auto&& model : model_scene_.GetRenderNode().GetChildren())
	//{
	//	for (auto&& primitive : model.GetPrimitives())
	//	{
	//		render_models.push_back(RenderModel
	//			{
	//				RenderModelCategory::kRenderModel,
	//				render_setup_.GetPipeline(PipelineId::kBuildGBuffers),
	//				u32(primitive.indices.count),
	//				model.GetDescriptorSets(),
	//			{},
	//			{},
	//			std::make_pair(primitive.indices.buffer->GetHandle(), primitive.indices.offset)
	//			});

	//		for (auto&& vertex_buffer : primitive.vertex_buffers)
	//		{
	//			if (vertex_buffer)
	//			{
	//				render_models.back().vertex_buffers.push_back(vertex_buffer->buffer->GetHandle());
	//				render_models.back().vertex_buffers_offsets.push_back(vertex_buffer->offset);
	//			}
	//		}
	//	}
	//}

	//for (auto&& model : ui_scene_.GetRenderNode().GetChildren())
	//{
	//	for (auto&& primitive : model.GetPrimitives())
	//	{
	//		render_models.push_back(RenderModel
	//			{
	//				RenderModelCategory::kUIShape,
	//				render_setup_.GetPipeline(PipelineId::kUI),
	//				u32(primitive.indices.count),
	//				model.GetDescriptorSets(),
	//			{},
	//			{},
	//			std::make_pair(primitive.indices.buffer->GetHandle(), primitive.indices.offset),

	//			});

	//		for (auto&& vertex_buffer : primitive.vertex_buffers)
	//		{
	//			if (vertex_buffer)
	//			{
	//				render_models.back().vertex_buffers.push_back(vertex_buffer->buffer->GetHandle());
	//				render_models.back().vertex_buffers_offsets.push_back(vertex_buffer->offset);
	//			}
	//		}
	//	}
	//}

	//render_models.push_back(RenderModel
	//	{
	//		RenderModelCategory::kViewport,
	//		render_setup_.GetPipeline(PipelineId::kCollectGBuffers),
	//		6,
	//	{},
	//	{},
	//	{},
	//	{}
	//	}
	//);

	//render_models.back().vertex_buffers.push_back(viewport_vertex_buffer_.GetHandle());
	//render_models.back().vertex_buffers_offsets.push_back(0);

	//debug_geometry_.Update();

	//render_models.push_back(RenderModel
	//	{
	//		RenderModelCategory::kViewport,
	//		render_setup_.GetPipeline(PipelineId::kDebugLines),
	//		debug_geometry_.coords_lines_vertex_cnt,
	//		{}
	//	}
	//);

	//render_models.back().vertex_buffers.push_back(debug_geometry_.coords_lines_position_buffer_.GetHandle());
	//render_models.back().vertex_buffers.push_back(debug_geometry_.coords_lines_color_buffer_.GetHandle());
	//render_models.back().vertex_buffers_offsets.push_back(0);
	//render_models.back().vertex_buffers_offsets.push_back(0);


	//render_models.push_back(RenderModel
	//	{
	//		RenderModelCategory::kViewport,
	//		render_setup_.GetPipeline(PipelineId::kDebugLines),
	//		debug_geometry_.debug_lines_vertex_cnt,
	//		{}
	//	}
	//);

	//render_models.back().vertex_buffers.push_back(debug_geometry_.debug_lines_position_buffer_.GetHandle());
	//render_models.back().vertex_buffers.push_back(debug_geometry_.debug_lines_color_buffer_.GetHandle());
	//render_models.back().vertex_buffers_offsets.push_back(0);
	//render_models.back().vertex_buffers_offsets.push_back(0);


	//auto scene_descriptor_sets = model_scene_.GetRenderNode().GetDescriptorSets();
	//scene_descriptor_sets.insert(ui_scene_.GetDescriptorSets().begin(), ui_scene_.GetDescriptorSets().end());

scene.Update(frame_info.frame_index);
for (auto&& model : scene.models_)
{
	model.UpdateAndTryFillWrites(frame_info.frame_index);
	for (auto&& primitive : model.mesh->primitives)
	{
		primitive.UpdateAndTryFillWrites(frame_info.frame_index);
	}
}

	render_graph_handler_.FillCommandBuffer(command_buffer_, frame_info, render_setup_.GetPipelines(), scene);

	
	present_info_.pImageIndices = &frame_info.swapchain_image_index;

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
		vkDestroyFence(global_.logical_device, cmd_buffer_fence_, nullptr);
		vkDestroySemaphore(global_.logical_device, render_finished_semaphore_, nullptr);
	}
}
