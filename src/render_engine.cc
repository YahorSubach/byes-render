#include "render/render_engine.h"

#include "platform.h"
#include "stl_util.h"

#include <tchar.h>


#include <vector>
#include <map>
#include <tuple>
#include <chrono>

#include "vulkan/vulkan.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

#include "common.h"
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
#include "render/sampler.h"
#include "render/swapchain.h"

#include "render/ui/ui.h"

namespace render
{

	class VkDeviceWrapper
	{
	public:
		
		operator const VkDevice&() const { return device_; }

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

		RenderEngineImpl(InitParam param)
		{
			vk_init_success_ = false;

			if (!InitInstanceExtensions())
			{
				LOG(err, "Could not init InstanceExtensions");
				return;
			}

			if (!stl_util::All(platform::GetRequiredExtensions(), vk_instance_extensions_, [](auto&& ext_name, auto&& ext) { return std::strcmp(ext_name, ext.extensionName) == 0; }))
			{
				LOG(err, "Required platform Extension missing");
				return;
			}

			if (!InitInstanceLayers())
			{
				LOG(err, "InitInstanceLayers error");
				return;
			}

			if (!stl_util::All(GetValidationLayers(), vk_instance_layers_, [](auto&& ext_name, auto&& ext) { return std::strcmp(ext_name, ext.layerName) == 0; }))
			{
				LOG(err, "Instance Layers missing");
				return;
			}

			application_info_.apiVersion = VK_MAKE_API_VERSION(0, 1, 1, 0); // CHECK WHAT DOES THIS MEAN!
			application_info_.applicationVersion = 1;
			application_info_.engineVersion = 1;
			application_info_.pApplicationName = "vulkan_concepts";
			application_info_.pEngineName = "vulkan_concepts_engine";
			application_info_.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			application_info_.pNext = nullptr;

			VkInstanceCreateInfo instance_create_info{};
			instance_create_info.enabledExtensionCount = static_cast<uint32_t>(platform::GetRequiredExtensions().size());
			instance_create_info.ppEnabledExtensionNames = platform::GetRequiredExtensions().data();
			instance_create_info.enabledLayerCount = static_cast<uint32_t>(GetValidationLayers().size());
			instance_create_info.ppEnabledLayerNames = GetValidationLayers().data();
			instance_create_info.pApplicationInfo = &application_info_;
			instance_create_info.pNext = nullptr;
			instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;


			if (vkCreateInstance(&instance_create_info, nullptr, &vk_instance_) != VK_SUCCESS)
			{
				LOG(err, "vkCreateInstance error");
				return;
			}


			if (!InitPhysicalDevices())
			{
				LOG(err, "InitPhysicalDevices error");
				return;
			}


			uint32_t selected_device_index = DetermineDeviceForUse();

			if (selected_device_index == -1)
			{
				LOG(err, "Could not determine valid device");
				return;
			}

			device_cfg_.physical_device = vk_physical_devices_[selected_device_index];
			vkGetPhysicalDeviceProperties(device_cfg_.physical_device, &device_cfg_.physical_device_properties);
			auto images_properties = GetImageFormatsProperties();



			device_cfg_.graphics_queue_index = FindDeviceQueueFamalyWithFlag(device_cfg_.physical_device, VK_QUEUE_GRAPHICS_BIT);
			device_cfg_.transfer_queue_index = FindDeviceQueueFamalyWithFlag(device_cfg_.physical_device, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT);
			if (device_cfg_.transfer_queue_index == -1)
			{
				device_cfg_.transfer_queue_index = FindDeviceQueueFamalyWithFlag(device_cfg_.physical_device, VK_QUEUE_TRANSFER_BIT);
			}

			std::vector<uint32_t> queue_indices;
			queue_indices.push_back(device_cfg_.graphics_queue_index);
			if (device_cfg_.graphics_queue_index != device_cfg_.transfer_queue_index)
			{
				queue_indices.push_back(device_cfg_.transfer_queue_index);
			}

			if (!InitLogicalDevice(device_cfg_.physical_device, queue_indices))
			{
				LOG(err, "InitLogicalDevices error");
				return;
			}

			device_cfg_.logical_device = vk_logical_devices_[0];

			vkGetDeviceQueue(device_cfg_.logical_device, device_cfg_.graphics_queue_index, 0, &device_cfg_.graphics_queue);
			if (device_cfg_.graphics_queue_index != device_cfg_.transfer_queue_index)
			{
				vkGetDeviceQueue(device_cfg_.logical_device, device_cfg_.transfer_queue_index, 0, &device_cfg_.transfer_queue);
			}
			else
			{
				device_cfg_.transfer_queue = device_cfg_.graphics_queue;
			}
			
			platform::Window window = platform::CreatePlatformWindow(param);

			surface_ptr_ = std::make_unique<Surface>(window, vk_instance_, device_cfg_);



			graphics_command_pool_ptr_ = std::make_unique<CommandPool>(device_cfg_, CommandPool::PoolType::kGraphics);
			transfer_command_pool_ptr_ = std::make_unique<CommandPool>(device_cfg_, CommandPool::PoolType::kTransfer);

			device_cfg_.graphics_cmd_pool = graphics_command_pool_ptr_.get();
			device_cfg_.transfer_cmd_pool = transfer_command_pool_ptr_.get();

			vk_init_success_ = true;
		}


		bool VKInitSuccess()
		{
			return vk_init_success_;
		}

		void cleanup() {
			//vkDestroyPipelineLayout(vk_logical_devices_[selected_device_index], pipelineLayout, nullptr);
		}

		void ShowWindow()
		{

			static auto start_time = std::chrono::high_resolution_clock::now();

			platform::ShowWindow(surface_ptr_->GetWindow());

			bool should_refresh_swapchain = true;

			std::vector<FrameHandler> frames;

			float yaw = glm::pi<float>() * 5 / 4;
			float pitch = -glm::pi<float>() /4;

			glm::vec3 position(2, 2, 1.7);

			uint32_t current_frame_index = -1;

			Image def_image = Image::FromFile(device_cfg_, "../images/test.jpg");
			device_cfg_.default_image = def_image;

			while (!platform::IsWindowClosed(surface_ptr_->GetWindow()) && should_refresh_swapchain)
			{
				DescriptorPool descriptor_pool(device_cfg_, 2000, 2000);

				device_cfg_.descriptor_pool = &descriptor_pool;

				graphics_command_pool_ptr_->ClearCommandBuffers();
				graphics_command_pool_ptr_->CreateCommandBuffers(kFramesCount);

				should_refresh_swapchain = false;

				Swapchain swapchain(device_cfg_, *surface_ptr_);

				device_cfg_.presentation_extent = swapchain.GetExtent();
				device_cfg_.presentation_format = swapchain.GetFormat();

				ui::UI ui(device_cfg_, swapchain.GetExtent());

				RenderSetup render_setup(device_cfg_);

				std::vector<Framebuffer> swapchain_framebuffers;

				Image depth_image(device_cfg_, device_cfg_.depth_map_format, swapchain.GetExtent());
				ImageView depth_image_view(device_cfg_, depth_image);

				for (int i = 0; i < swapchain.GetImagesCount(); i++)
				{
					//std::vector<std::reference_wrapper<const ImageView>> attachments;
					//
					//attachments.push_back(swapchain.GetImageView(i));
					//attachments.push_back(depth_image_view);

					swapchain_framebuffers.push_back(Framebuffer(device_cfg_, swapchain.GetExtent(), render_setup.GetRenderPass(RenderPassId::kSimpleRenderToScreen)));
					swapchain_framebuffers.back().AddAttachment("swapchain_image", swapchain.GetImageView(i));
					swapchain_framebuffers.back().AddAttachment("depth_image", depth_image_view);
				}

				BatchesManager batches_manager(device_cfg_, kFramesCount, swapchain, descriptor_pool);
				
				frames.clear();
				frames.reserve(kFramesCount);

				for (size_t frame_ind = 0; frame_ind < kFramesCount; frame_ind++)
				{
					frames.push_back(FrameHandler(device_cfg_, swapchain, render_setup, batches_manager, ui));
				}

				auto last_update_time = std::chrono::steady_clock::now();

				glm::vec3 pos_velocity = glm::vec3(0, 0, 0);

				while (!platform::IsWindowClosed(surface_ptr_->GetWindow()) && !should_refresh_swapchain)
				{
					current_frame_index = (current_frame_index + 1) % kFramesCount;

					uint32_t image_index;

					batches_manager.Update();

					VkResult result = vkAcquireNextImageKHR(device_cfg_.logical_device, swapchain.GetHandle(), UINT64_MAX,
						frames[current_frame_index].GetImageAvailableSemaphore(), VK_NULL_HANDLE, &image_index);


					if (result != VK_SUCCESS)
					{
						if (result == VK_ERROR_OUT_OF_DATE_KHR)
						{
							should_refresh_swapchain = true;
						}
						continue;
					}


					int mouse_x_delta;
					int mouse_y_delta;

					platform::GetMouseDelta(mouse_x_delta, mouse_y_delta);
					auto&& buttons = platform::GetButtonState();

					yaw += (1.0f * mouse_x_delta / 500);
					pitch += (-1.0f * mouse_y_delta / 500);
					if (pitch > 1.505f)
						pitch = 1.505f;
					if (pitch < -1.505f)
						pitch = -1.505f;

					glm::vec3 look = glm::normalize(glm::vec3(glm::sin(yaw) * glm::cos(pitch), glm::cos(yaw) * glm::cos(pitch), glm::sin(pitch)));

					glm::vec3 walk = glm::vec3(look.x, look.y, 0);

					auto current_time = std::chrono::steady_clock::now();

					auto time_delta = current_time - last_update_time;
					last_update_time = current_time;

					auto time_delta_s = std::chrono::duration_cast<std::chrono::duration<float>>(time_delta).count();

					glm::vec3 vec = glm::vec3(0,0,0);
					bool update_pos = false;


					if (buttons['w' - 'a'])
					{
						vec += glm::normalize(walk);
						update_pos = true;
					}

					if (buttons['s' - 'a'])
					{
						vec -= glm::normalize(walk);
						update_pos = true;
					}

					if (buttons['d' - 'a'])
					{
						vec += glm::normalize(glm::vec3(glm::sin(yaw + glm::pi<float>() / 2), glm::cos(yaw + glm::pi<float>() / 2), 0));
						update_pos = true;
					}

					if (buttons['a' - 'a'])
					{
						vec -= glm::normalize(glm::vec3(glm::sin(yaw + glm::pi<float>() / 2), glm::cos(yaw + glm::pi<float>() / 2), 0));
						update_pos = true;
					}

					glm::vec3 acc = glm::vec3(0,0,0);



					if (glm::length(pos_velocity) > glm::epsilon<float>())
					{
						acc = 6.0f * glm::normalize(-pos_velocity);

						if (glm::length(acc) * time_delta_s > glm::length(pos_velocity))
						{
							acc *= (glm::length(pos_velocity) / (glm::length(acc) * time_delta_s));
						}
					}

					if (glm::length(vec) > glm::epsilon<float>())
					{
						acc += 12.0f * glm::normalize(vec);
					}

					pos_velocity += time_delta_s * acc;
					position += time_delta_s * pos_velocity;



					should_refresh_swapchain = !frames[current_frame_index].Draw(swapchain_framebuffers[image_index], image_index, render_setup, position, look);



				}

				vkDeviceWaitIdle(device_cfg_.logical_device);
			}

			platform::JoinWindowThread(surface_ptr_->GetWindow());
		}

		~RenderEngineImpl()
		{

		}

	private:

		const std::vector<const char*>& GetRequiredExtensions()
		{
			static const std::vector<const char*> extensions{ "VK_KHR_swapchain" };
			return extensions;
		}

		const std::vector<const char*>& GetValidationLayers()
		{
#ifndef NDEBUG
			static const std::vector<const char*> layers{ "VK_LAYER_KHRONOS_validation" };
			return layers;
#else
			static const std::vector<const char*> layers{};
			return layers;
#endif // !NDEBUG
		}

		bool InitPhysicalDevices()
		{
			uint32_t physical_devices_count;

			if (vkEnumeratePhysicalDevices(vk_instance_, &physical_devices_count, nullptr) != VK_SUCCESS)
				return false;

			vk_physical_devices_.resize(physical_devices_count);

			if (vkEnumeratePhysicalDevices(vk_instance_, &physical_devices_count, reinterpret_cast<VkPhysicalDevice*>(vk_physical_devices_.data())) != VK_SUCCESS)
				return false;
			//TODO: add device selection
			for (size_t i = 0; i < physical_devices_count; i++)
			{
				InitPhysicalDeviceProperties(vk_physical_devices_[i]);
				InitPhysicalDeviceMemoryProperties(vk_physical_devices_[i]);
				InitPhysicalDeviceQueueFamaliesProperties(vk_physical_devices_[i]);
				InitPhysicalDeviceFeatures(vk_physical_devices_[i]);
				if (!(InitPhysicalDeviceLayers(vk_physical_devices_[i]) &&
					InitPhysicalDeviceExtensions(vk_physical_devices_[i])))
					return false;
			}
			return true;
		}

		std::map<VkFormat, VkImageFormatProperties> GetImageFormatsProperties()
		{
			std::map<VkFormat, VkImageFormatProperties> res;

			for (int i = 0; i < 125; i++)
			{
				VkImageFormatProperties prop;

				VkResult vk_res = vkGetPhysicalDeviceImageFormatProperties(
					device_cfg_.physical_device,
					static_cast<VkFormat>(i),
					VK_IMAGE_TYPE_2D,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					0,
					&prop);

				if (vk_res == VK_SUCCESS)
				{
					res.emplace(static_cast<VkFormat>(i), prop);
				}
			}

			return res;
		}


		uint32_t FindDeviceQueueFamalyWithFlag(const VkPhysicalDevice& physical_deivce, VkQueueFlags enabled_flags, VkQueueFlags disabled_flags = 0)
		{
			auto&& queue_famalies_properties = vk_physical_devices_to_queues_[physical_deivce];


			for (uint32_t i = 0; i < queue_famalies_properties.size(); i++)
			{
				if ((queue_famalies_properties[i].queueFlags & enabled_flags) == enabled_flags 
					&& (queue_famalies_properties[i].queueFlags & disabled_flags) == 0)
				{
					return i;
				}
			}

			return -1;
		}

		uint32_t DetermineDeviceForUse()
		{
			VkPhysicalDevice physical_device_out = nullptr;

			for (auto&& [physical_device, queues_properties] : vk_physical_devices_to_queues_)
			{
				for (uint32_t i = 0; i < queues_properties.size(); i++)
				{
					if (platform::GetPhysicalDevicePresentationSupport(physical_device, i) && (queues_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
					{
						if(physical_device_out == nullptr || 
							vk_physical_devices_propeties_[physical_device].deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
							)

						physical_device_out = physical_device;
					}
				}
			}

			auto physical_device_out_it = std::find(vk_physical_devices_.begin(), vk_physical_devices_.end(), physical_device_out);

			if (physical_device_out_it != vk_physical_devices_.end())
			{
				return static_cast<uint32_t>(physical_device_out_it - vk_physical_devices_.begin());
			}

			return -1;
		}


		void InitPhysicalDeviceMemoryProperties(VkPhysicalDevice physical_device)
		{
			using PhDevPropMapType = std::map<VkPhysicalDevice, VkPhysicalDeviceMemoryProperties>;

			auto it = vk_physical_devices_memory_propeties_.lower_bound(physical_device);

			if (it == vk_physical_devices_memory_propeties_.end() || it->first != physical_device)
			{
				it = vk_physical_devices_memory_propeties_.emplace_hint(it,
					std::piecewise_construct, std::forward_as_tuple(physical_device), std::forward_as_tuple());
				vkGetPhysicalDeviceMemoryProperties(physical_device, &it->second);
			}
		}

		void InitPhysicalDeviceProperties(VkPhysicalDevice physical_device)
		{
			using PhDevPropMapType = std::map<VkPhysicalDevice, VkPhysicalDeviceMemoryProperties>;

			auto it = vk_physical_devices_propeties_.lower_bound(physical_device);

			if (it == vk_physical_devices_propeties_.end() || it->first != physical_device)
			{
				it = vk_physical_devices_propeties_.emplace_hint(it,
					std::piecewise_construct, std::forward_as_tuple(physical_device), std::forward_as_tuple());
				vkGetPhysicalDeviceProperties(physical_device, &it->second);
			}
		}

		void InitPhysicalDeviceQueueFamaliesProperties(VkPhysicalDevice physical_device)
		{
			using PhDevFamQMapType = std::map<VkPhysicalDevice, std::vector<VkQueueFamilyProperties>>;

			auto it = vk_physical_devices_to_queues_.lower_bound(physical_device);

			if (it == vk_physical_devices_to_queues_.end() || it->first != physical_device)
			{
				uint32_t device_queue_famaly_prop_cnt;
				vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &device_queue_famaly_prop_cnt, nullptr);

				it = vk_physical_devices_to_queues_.emplace_hint(it,
					std::piecewise_construct, std::forward_as_tuple(physical_device), std::forward_as_tuple());

				it->second.resize(device_queue_famaly_prop_cnt);

				vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &device_queue_famaly_prop_cnt, it->second.data());
			}
		}


		void InitPhysicalDeviceFeatures(const VkPhysicalDevice& physical_device)
		{

			using PhDevPropMapType = std::map<VkPhysicalDevice, VkPhysicalDeviceFeatures>;

			auto it = vk_physical_devices_features_.lower_bound(physical_device);

			if (it == vk_physical_devices_features_.end() || it->first != physical_device)
			{
				it = vk_physical_devices_features_.emplace_hint(it,
					std::piecewise_construct, std::forward_as_tuple(physical_device), std::forward_as_tuple());
				vkGetPhysicalDeviceFeatures(physical_device, &it->second);

				VkPhysicalDeviceFeatures2 features2;
				VkPhysicalDeviceImagelessFramebufferFeatures imageless_features;
				imageless_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES;
				imageless_features.pNext = nullptr;

				features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
				features2.pNext = &imageless_features;
				vkGetPhysicalDeviceFeatures2(physical_device, &features2);
				auto next = features2.pNext;
			}
		}

		bool InitInstanceLayers()
		{
			uint32_t count;
			if (vkEnumerateInstanceLayerProperties(&count, nullptr) == VK_SUCCESS)
			{
				vk_instance_layers_.resize(count);
				if (vkEnumerateInstanceLayerProperties(&count, vk_instance_layers_.data()) == VK_SUCCESS)
					return true;
			}
			return false;
		}

		bool InitPhysicalDeviceLayers(const VkPhysicalDevice& physical_device)
		{
			using PhDevFamQMapType = std::map<VkPhysicalDevice, std::vector<VkLayerProperties>>;

			auto it = vk_physical_devices_layers_.lower_bound(physical_device);

			if (it == vk_physical_devices_layers_.end() || it->first != physical_device)
			{
				uint32_t device_layers_cnt;
				if (vkEnumerateDeviceLayerProperties(physical_device, &device_layers_cnt, nullptr) != VK_SUCCESS)
					return false;

				it = vk_physical_devices_layers_.emplace_hint(it,
					std::piecewise_construct, std::forward_as_tuple(physical_device), std::forward_as_tuple());

				it->second.resize(device_layers_cnt);

				if (vkEnumerateDeviceLayerProperties(physical_device, &device_layers_cnt, it->second.data()) != VK_SUCCESS)
					return false;
			}
			return true;
		}

		bool InitInstanceExtensions()
		{
			uint32_t count;
			if (vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr) == VK_SUCCESS)
			{
				vk_instance_extensions_.resize(count);
				if (vkEnumerateInstanceExtensionProperties(nullptr, &count, vk_instance_extensions_.data()) == VK_SUCCESS)
					return true;
			}
			return false;
		}

		bool InitPhysicalDeviceExtensions(const VkPhysicalDevice& physical_device)
		{
			using PhDevFamQMapType = std::map<VkPhysicalDevice, std::vector<VkExtensionProperties>>;

			auto it = vk_physical_devices_extensions_.lower_bound(physical_device);

			if (it == vk_physical_devices_extensions_.end() || it->first != physical_device)
			{
				uint32_t device_extensions_cnt;
				if (vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extensions_cnt, nullptr) != VK_SUCCESS)
					return false;

				it = vk_physical_devices_extensions_.emplace_hint(it,
					std::piecewise_construct, std::forward_as_tuple(physical_device), std::forward_as_tuple(device_extensions_cnt));

				if (vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extensions_cnt, it->second.data()) != VK_SUCCESS)
					return false;
			}
			return true;
		}

		bool InitLogicalDevice(const VkPhysicalDevice& physical_device, const std::vector<uint32_t> queue_famaly_indices)
		{
			VkDeviceCreateInfo logical_device_create_info{};

			if (stl_util::All(GetRequiredExtensions(), vk_physical_devices_extensions_[physical_device], [](auto&& ext_name, auto&& ext) { return std::strcmp(ext_name, ext.extensionName) == 0; }))
			{
				logical_device_create_info.enabledExtensionCount = static_cast<uint32_t>(GetRequiredExtensions().size());
				logical_device_create_info.ppEnabledExtensionNames = GetRequiredExtensions().data();
			}
			else return false;

			VkPhysicalDeviceImagelessFramebufferFeatures imageless_features;
			imageless_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES;
			imageless_features.pNext = nullptr;
			imageless_features.imagelessFramebuffer = VK_TRUE;

			logical_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			logical_device_create_info.pNext = &imageless_features;
			logical_device_create_info.flags = 0;
			logical_device_create_info.enabledLayerCount = 0;
			logical_device_create_info.ppEnabledLayerNames = nullptr;
			logical_device_create_info.pEnabledFeatures = &vk_physical_devices_features_[physical_device];


			std::vector<VkDeviceQueueCreateInfo> queue_create_infos(queue_famaly_indices.size());
			std::vector<std::vector<float>> queue_priorities(queue_famaly_indices.size());
			for (auto&& queue_famaly_index : queue_famaly_indices)
			{
				queue_create_infos[queue_famaly_index].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queue_create_infos[queue_famaly_index].flags = 0;
				queue_create_infos[queue_famaly_index].queueFamilyIndex = queue_famaly_index;

				uint32_t device_queue_count = vk_physical_devices_to_queues_[physical_device][queue_famaly_index].queueCount;

				queue_create_infos[queue_famaly_index].queueCount = device_queue_count;

				queue_priorities[queue_famaly_index].resize(device_queue_count);
				
				for (auto&& priority : queue_priorities[queue_famaly_index]) 
					priority = 1.0f;

				queue_create_infos[queue_famaly_index].pQueuePriorities = queue_priorities[queue_famaly_index].data();
				queue_create_infos[queue_famaly_index].pNext = nullptr;
			}



			logical_device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
			logical_device_create_info.pQueueCreateInfos = queue_create_infos.data();

			vk_logical_devices_.emplace_back();

			if (vkCreateDevice(physical_device, &logical_device_create_info, nullptr, &vk_logical_devices_[vk_logical_devices_.size() - 1]) != VK_SUCCESS)
				return false;
			return true;
		}




		bool vk_init_success_;
		VkApplicationInfo application_info_;
		VkInstanceWrapper vk_instance_;



		std::vector<VkPhysicalDevice> vk_physical_devices_;
		std::map<VkPhysicalDevice, VkPhysicalDeviceMemoryProperties> vk_physical_devices_memory_propeties_;
		std::map<VkPhysicalDevice, VkPhysicalDeviceProperties> vk_physical_devices_propeties_;
		std::map<VkPhysicalDevice, std::vector<VkQueueFamilyProperties>> vk_physical_devices_to_queues_;
		std::map<VkPhysicalDevice, VkPhysicalDeviceFeatures> vk_physical_devices_features_;
		std::vector<VkLayerProperties > vk_instance_layers_;
		std::map<VkPhysicalDevice, std::vector<VkLayerProperties>> vk_physical_devices_layers_;
		std::vector<VkExtensionProperties> vk_instance_extensions_;
		std::map<VkPhysicalDevice, std::vector<VkExtensionProperties>> vk_physical_devices_extensions_;
		
		std::vector<VkDeviceWrapper> vk_logical_devices_;

		DeviceConfiguration device_cfg_;

		std::unique_ptr<Surface> surface_ptr_;

		std::unique_ptr<DescriptorPool> descriptor_pool_ptr_;
		std::unique_ptr<CommandPool> graphics_command_pool_ptr_;
		std::unique_ptr<CommandPool> transfer_command_pool_ptr_;

		const uint32_t kFramesCount = 4;
		//VkSemaphore image_available_semaphore_;
		//VkSemaphore render_finished_semaphore_;
};

	RenderEngine::RenderEngine(InitParam param)
	{
		impl_ = std::make_unique<RenderEngineImpl>(param);
	}

	RenderEngine::~RenderEngine() = default;

	bool RenderEngine::VKInitSuccess()
	{
		return impl_->VKInitSuccess();
	}

	void RenderEngine::ShowWindow()
	{
		impl_->ShowWindow();
	}
}