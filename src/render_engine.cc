#include "render/render_engine.h"

#include "platform.h"
#include "stl_util.h"

#include <tchar.h>


#include <vector>
#include <stack>
#include <map>
#include <tuple>
#include <chrono>
#include <sstream>

#include "vulkan/vulkan.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

#include "common.h"

#include "render/global.h"

#include "surface.h"
#include "render/vk_util.h"
#include "render/data_types.h"

#include "render/batch.h"
#include "render/batches_manager.h"
#include "render/buffer.h"
#include "render/command_pool.h"
#include "render/descriptor_pool.h"
#include "render/framebuffer.h"
#include "render/frame_handler.h"
#include "render/graphics_pipeline.h"
#include "render/image.h"
#include "render/image_view.h"
#include "render/render_setup.h"
#include "render/sampler.h"
#include "render/scene.h"
#include "render/swapchain.h"
#include "render/render_system.h"

#include "render/ui/panel.h"

#include "sync_util.h"

namespace render
{
	class VkDeviceWrapper
	{
	public:

		operator const VkDevice& () const { return device_; }

		VkDevice* operator&() { return &device_; }

		~VkDeviceWrapper()
		{
			vkDeviceWaitIdle(device_);
			vkDestroyDevice(device_, nullptr);
		}
	private:
		VkDevice device_;
	};

	class VkInstanceWrapper
	{
	public:
		operator const VkInstance& () const { return instance_; }

		VkInstance* operator&() { return &instance_; }

		~VkInstanceWrapper()
		{
			vkDestroyInstance(instance_, nullptr);
		}
	private:
		VkInstance instance_;
	};

	class RenderEngine::RenderEngineImpl
	{
	public:

		RenderEngineImpl(const RenderEngineImpl&) = delete;
		RenderEngineImpl& operator=(const RenderEngineImpl&) = delete;

		RenderEngineImpl(InitParam param, const std::string& app_name) : external_command_queue_(32), last_object_id_(0), render_system_(platform::CreatePlatformWindow(param), app_name)
		{

		}

		void cleanup() {
			//vkDestroyPipelineLayout(vk_logical_devices_[selected_device_index], pipelineLayout, nullptr);
		}

		void StartRenderThread()
		{
			ready = false;
			render_thread_ = std::thread(&RenderEngine::RenderEngineImpl::RenderLoop, this);
			while (!ready)
			{
				std::this_thread::yield();
			}
		}

		void RenderLoop()
		{

			std::vector<ModelPack> model_packs;
			std::unordered_map<std::string, uint32_t> model_packs_name_to_index;

			static auto start_time = std::chrono::high_resolution_clock::now();
			static auto start_time_fps = std::chrono::high_resolution_clock::now();


			platform::ShowWindow(surface_ptr_->GetWindow());

			bool should_refresh_swapchain = true;

			std::vector<FrameHandler> frames;

			glm::vec3 position(2, 2, 1.7);

			uint32_t current_frame_index = -1;





			DescriptorSetsManager descriptor_set_manager(global_);
			RenderSetup render_setup(global_);

			model_packs.push_back(ModelPack(global_, descriptor_set_manager));

			DebugGeometry dg(global_, descriptor_set_manager);

			scenes_.emplace_back(global_, descriptor_set_manager, dg); // TODO fix scene impl move

			ready = true;
			uint32_t frame_cnt = 0;

			ui::UI ui(global_);
			std::shared_ptr<ui::Panel> screen_panel;
			std::shared_ptr<ui::TextBlock> block;
			while (!platform::IsWindowClosed(surface_ptr_->GetWindow()) && should_refresh_swapchain)
			{
				//descriptor_set_manager.FreeAll();

				graphics_command_pool_ptr_->ClearCommandBuffers();
				graphics_command_pool_ptr_->CreateCommandBuffers(kFramesCount);

				should_refresh_swapchain = false;

				Swapchain swapchain(global_, *surface_ptr_);

				auto swapchain_extent = swapchain.GetExtent();

				std::array<Extent, kExtentTypeCnt> extents
				{
					swapchain_extent,
						swapchain_extent,
					{ 512,512 }
				};

				scenes_[0].aspect = 1.0f * swapchain_extent.width / swapchain_extent.height;

				render_setup.InitPipelines(descriptor_set_manager, extents);



				std::vector<Framebuffer> swapchain_framebuffers;

				for (int i = 0; i < swapchain.GetImagesCount(); i++)
				{
					Framebuffer::ConstructParams params
					{
						render_setup.GetSwapchainRenderPass(),
							swapchain.GetExtent()
					};

					params.attachments.push_back(swapchain.GetImageView(i));

					swapchain_framebuffers.push_back(Framebuffer(global_, params));
				}

				frames.clear();
				frames.reserve(kFramesCount);

				if (screen_panel)
				{
					screen_panel->SetWidth(swapchain_extent.width);
					screen_panel->SetHeight(swapchain_extent.height);
				}

				for (size_t frame_ind = 0; frame_ind < kFramesCount; frame_ind++)
				{
					frames.push_back(FrameHandler(global_, swapchain, render_setup, extents, descriptor_set_manager, ui, scenes_[0]));
				}



				while (!platform::IsWindowClosed(surface_ptr_->GetWindow()) && !should_refresh_swapchain)
				{

					frame_cnt++;

					current_frame_index = (current_frame_index + 1) % kFramesCount;
					global_.frame_ind = current_frame_index;

					uint32_t image_index;

					VkResult result = vkAcquireNextImageKHR(global_.logical_device, swapchain.GetHandle(), UINT64_MAX,
						frames[current_frame_index].GetImageAvailableSemaphore(), VK_NULL_HANDLE, &image_index);


					if (result != VK_SUCCESS)
					{
						if (result == VK_ERROR_OUT_OF_DATE_KHR)
						{
							should_refresh_swapchain = true;
						}
						continue;
					}

					FrameInfo frame_info
					{
						swapchain_framebuffers[image_index],
						swapchain.GetImage(image_index),
						image_index,
						current_frame_index
					};

					should_refresh_swapchain = !frames[current_frame_index].Draw(frame_info, scenes_[0]);

					int command_count_to_execute = external_command_queue_.Size();



					if (block)
					{
						std::chrono::duration<float> dur = std::chrono::high_resolution_clock::now() - start_time_fps;

						std::string s = std::to_string(1.0f * frame_cnt / dur.count());
						std::basic_string<char32_t> us(s.data(), s.data() + s.length());

						if (dur.count() > 3)
						{
							start_time_fps = std::chrono::high_resolution_clock::now();
							frame_cnt = 0;
						}

						block->SetText(us, 30);
					}

					while (command_count_to_execute-- > 0)
					{
						auto&& command = external_command_queue_.Pop();

						if (std::holds_alternative<command::Load>(command))
						{
							model_packs.push_back(ModelPack(global_, descriptor_set_manager));

							auto&& specified_command = std::get<command::Load>(command);
							model_packs.back().AddGLTF(*specified_command.model);
							model_packs_name_to_index.emplace(specified_command.pack_name, (uint32_t)(model_packs.size() - 1));
						}

						if (std::holds_alternative<command::Geometry>(command))
						{
							auto&& specified_command = std::get<command::Geometry>(command);

							model_packs[0].AddSimpleMesh(specified_command.faces, PrimitiveProps::kDebugPos);

							auto node_id = scenes_[0].AddNode();
							auto&& scene_node = scenes_[0].GetNode(node_id);
							scenes_[0].AddModel(scene_node, model_packs[0].meshes[0]);
						}

						if (std::holds_alternative<command::AddObject<ObjectType::StaticModel>>(command))
						{
							auto&& specified_command = std::get<command::AddObject<ObjectType::StaticModel>>(command);

							auto&& pack = model_packs[model_packs_name_to_index.at(specified_command.desc.pack_name)];
							auto&& pack_model = pack.models[specified_command.desc.model_name];

							auto node_id = scenes_[0].AddNode();
							auto&& node = scenes_[0].GetNode(node_id);

							RegisterObject(ObjectType::Node, specified_command.object_id, node_id);

							scenes_[0].AddModel(node, *pack_model.mesh);
						}

						if (std::holds_alternative<command::AddObject<ObjectType::Node>>(command))
						{
							auto&& specified_command = std::get<command::AddObject<ObjectType::Node>>(command);

							auto node_id = scenes_[0].AddNode();
							auto&& node = scenes_[0].GetNode(node_id);

							RegisterObject(ObjectType::Node, specified_command.object_id, node_id);

						}


						if (std::holds_alternative<command::AddObject<ObjectType::DbgPoints>>(command))
						{
							auto&& specified_command = std::get<command::AddObject<ObjectType::DbgPoints>>(command);

							model_packs[0].AddSimpleMesh(specified_command.desc.points, PrimitiveProps::kDebugPoints);
							std::get<primitive::Geometry>(model_packs[0].meshes.back().primitives.back()).material.color = specified_command.desc.color;

							auto node_id = scenes_[0].AddNode();
							auto&& node = scenes_[0].GetNode(node_id);

							RegisterObject(ObjectType::Node, specified_command.object_id, node_id);

							scenes_[0].AddModel(node, model_packs[0].meshes.back());
						}

						if (std::holds_alternative<command::AddObject<ObjectType::Camera>>(command))
						{
							auto&& specified_command = std::get<command::AddObject<ObjectType::Camera>>(command);
						}

						if (std::holds_alternative<command::SetActiveCameraNode>(command))
						{
							auto&& specified_command = std::get<command::SetActiveCameraNode>(command);

							auto&& info = object_id_to_scene_object_id_[specified_command.node_id.value];
							scenes_[0].camera_node_id_ = { info.id };

							screen_panel = std::make_shared<ui::Panel>(scenes_[0], 0, 0, extents[u32(ExtentType::kPresentation)].width, extents[u32(ExtentType::kPresentation)].height);
							block = std::make_shared<ui::TextBlock>(ui, scenes_[0], descriptor_set_manager, 30, 30);
							block->SetText(U"Жопич", 30);
							screen_panel->AddChild(block);
						}

						if (std::holds_alternative<command::ObjectsUpdate>(command))
						{
							auto&& specified_command = std::get<command::ObjectsUpdate>(command);

							for (auto&& [id, transform] : specified_command.updates)
							{
								if (object_id_to_scene_object_id_.size() > id)
								{
									if (object_id_to_scene_object_id_[id].type == ObjectType::Node)
									{
										NodeId node_id = object_id_to_scene_object_id_[id].id;
										auto&& node = scenes_[0].GetNode({ node_id });
										node.local_transform = transform;
									}
								}
							}

							//model_packs[0].AddSimpleMesh(specified_command.desc.points, PrimitiveProps::kDebugPoints);
							//model_packs[0].meshes.back().primitives.back().material.color = specified_command.desc.color;

							//Node node{};
							//node.local_transform = glm::identity<glm::mat4>();
							//auto&& scene_node = scenes_[0].AddNode(node);
							//scenes_[0].AddModel(scene_node, model_packs[0].meshes.back());
						}

					}
				}

				vkDeviceWaitIdle(global_.logical_device);
			}

			platform::JoinWindowThread(surface_ptr_->GetWindow());
		}


		void RegisterObject(ObjectType type, uint32_t external_id, util::UniId internal_id)
		{

			if (external_id >= object_id_to_scene_object_id_.size())
			{
				object_id_to_scene_object_id_.resize(1.3 * external_id + 1);
			}

			object_id_to_scene_object_id_[external_id].type = type;
			object_id_to_scene_object_id_[external_id].id = internal_id;

		}


		void SetDebugLines(const std::vector<std::pair<DebugPoint, DebugPoint>> lines)
		{
			std::vector<DebugGeometry::Line> debug_lines;

			for (auto&& line : lines)
			{
				debug_lines.push_back({
					{line.first.position,line.first.color},
					{line.second.position,line.second.color}
					}
				);
			}

			scenes_[0].debug_geometry_.SetDebugLines(debug_lines);
		}

		template<ObjectType Type>
		ObjectId<Type> AddObject(const ObjectDescription<Type>& desc)
		{
			uint32_t id = -1;

			if (!free_ids_.empty())
			{
				id = free_ids_.top();
				free_ids_.pop();
			}
			else
			{
				id = last_object_id_;
				last_object_id_++;
			}

			ObjectId<Type> result{ desc.name, id};
			external_command_queue_.Push(render::command::AddObject{ id, desc });

			return result;
		}

		~RenderEngineImpl()
		{

		}

		byes::sync_util::ConcQueue1P1C<command::Command> external_command_queue_;
		std::vector<Scene> scenes_;
	private:









		std::vector<VkDeviceWrapper> vk_logical_devices_;



		std::unique_ptr<Surface> surface_ptr_;

		std::unique_ptr<DescriptorPool> descriptor_pool_ptr_;




		std::thread render_thread_;



		bool ready;

		uint32_t last_object_id_;


		struct ObjectInfo
		{
			ObjectType type;
			util::UniId id;
		};

		std::stack<uint32_t> free_ids_;
		std::vector<ObjectInfo> object_id_to_scene_object_id_;

		RenderSystem render_system_;
	};

	RenderEngine::RenderEngine(InitParam param, const std::string& app_name)
	{
		impl_ = std::make_unique<RenderEngineImpl>(param, app_name);
	}

	void RenderEngine::SetDebugLines(const std::vector<std::pair<DebugPoint, DebugPoint>>& lines)
	{
		impl_->SetDebugLines(lines);
	}

	void RenderEngine::QueueCommand(const command::Command& render_command)
	{
		impl_->external_command_queue_.Push(render_command);
	}

	RenderEngine::~RenderEngine() = default;

	bool RenderEngine::VKInitSuccess()
	{
		return impl_->VKInitSuccess();
	}

	void RenderEngine::StartRender()
	{
		impl_->StartRenderThread();
	}

	const InputState& RenderEngine::GetInputState()
	{
		return platform::GetInputState();
	}

#define RENDER_ENGINE_OBJECT(x)																					\
	template<>																									\
	ObjectId<ObjectType::x> RenderEngine::AddObject(const ObjectDescription<ObjectType::x>& desc)				\
	{																											\
		return impl_->AddObject<ObjectType::x>(desc);															\
	}																											

#include "render\render_engine_objects.inl"

}


// SOME SHEET

//render::ui::TextBlockProxy::TextBlock(render::RenderEngine::RenderEngineImpl& reimpl)
//{
//}




		//std::map<VkFormat, VkImageFormatProperties> GetImageFormatsProperties()
		//{
		//	std::map<VkFormat, VkImageFormatProperties> res;

		//	for (int i = 0; i < 125; i++)
		//	{
		//		VkImageFormatProperties prop;

		//		VkResult vk_res = vkGetPhysicalDeviceImageFormatProperties(
		//			global_.physical_device,
		//			static_cast<VkFormat>(i),
		//			VK_IMAGE_TYPE_2D,
		//			VK_IMAGE_TILING_OPTIMAL,
		//			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		//			0,
		//			&prop);

		//		if (vk_res == VK_SUCCESS)
		//		{
		//			res.emplace(static_cast<VkFormat>(i), prop);
		//		}
		//	}

		//	return res;
		//}