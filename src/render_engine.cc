#include "render/render_engine.h"

#include "platform.h"
#include "stl_util.h"

#include <tchar.h>


#include <vector>
#include <map>
#include <tuple>
#include <fstream>
#include <chrono>

#include "vulkan/vulkan.h"


#include "common.h"
#include "surface.h"
#include "render/vk_util.h"
#include "render/data_types.h"

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

namespace render
{


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

			application_info_.apiVersion = VK_API_VERSION_1_0; // CHECK WHAT DOES THIS MEAN!
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


			device_cfg_.graphics_queue_index = FindDeviceQueueFamalyWithFlag(device_cfg_.physical_device, VK_QUEUE_GRAPHICS_BIT);
			device_cfg_.transfer_queue_index = FindDeviceQueueFamalyWithFlag(device_cfg_.physical_device, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT);

			if (!InitLogicalDevice(device_cfg_.physical_device, { device_cfg_.graphics_queue_index , device_cfg_.transfer_queue_index }))
			{
				LOG(err, "InitLogicalDevices error");
				return;
			}

			device_cfg_.logical_device = vk_logical_devices_[0];

			vkGetDeviceQueue(device_cfg_.logical_device, device_cfg_.graphics_queue_index, 0, &device_cfg_.graphics_queue);
			vkGetDeviceQueue(device_cfg_.logical_device, device_cfg_.transfer_queue_index, 0, &device_cfg_.transfer_queue);

			platform::Window window = platform::CreatePlatformWindow(param);

			surface_ptr_ = std::make_unique<Surface>(window, vk_instance_, device_cfg_);

			std::vector<uint32_t> queue_indices = { device_cfg_.graphics_queue_index, device_cfg_.transfer_queue_index };

			vertex_buffer_ptr_ = std::make_unique<Buffer>(device_cfg_, sizeof(vertices_[0]) * vertices_.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, queue_indices);
			index_buffer_ptr_ = std::make_unique<Buffer>(device_cfg_, sizeof(faces_[0]) * faces_.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, queue_indices);

			graphics_command_pool_ptr_ = std::make_unique<CommandPool>(device_cfg_, CommandPool::PoolType::kGraphics);
			transfer_command_pool_ptr_ = std::make_unique<CommandPool>(device_cfg_, CommandPool::PoolType::kTransfer);

			device_cfg_.graphics_cmd_pool = graphics_command_pool_ptr_.get();
			device_cfg_.transfer_cmd_pool = transfer_command_pool_ptr_.get();

			CopyToGPUBuffer(*vertex_buffer_ptr_, static_cast<const void*>(vertices_.data()), sizeof(vertices_[0]) * vertices_.size());
			CopyToGPUBuffer(*index_buffer_ptr_, static_cast<const void*>(faces_.data()), sizeof(faces_[0]) * faces_.size());

			vk_init_success_ = true;
		}


		bool VKInitSuccess()
		{
			return vk_init_success_;
		}

		VkShaderModule createShaderModule(const std::vector<char>& code) 
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

		void PrepareSwapChain(const Swapchain& swapchain, const ImageView& depth_view)
		{
			swapchain_frame_buffers_.clear();
			graphics_command_pool_ptr_->ClearCommandBuffers();
			graphics_pipeline_ptr_.reset();
			descriptor_pool_ptr_.reset();

			render_pass_ptr_ = std::make_unique<RenderPass>(device_cfg_.logical_device, swapchain.GetFormat());

			auto vert_shader_code = readFile("../shaders/vert.spv");
			auto frag_shader_code = readFile("../shaders/frag.spv");

			VkShaderModule vert_shader_module = createShaderModule(vert_shader_code);
			VkShaderModule frag_shader_module = createShaderModule(frag_shader_code);

			graphics_pipeline_ptr_ = std::make_unique<GraphicsPipeline>(device_cfg_.logical_device, vert_shader_module, frag_shader_module, swapchain.GetExtent(), *render_pass_ptr_);


			swapchain_frame_buffers_.reserve(swapchain.GetImagesCount());

			descriptor_pool_ptr_ = std::make_unique<DescriptorPool>(device_cfg_.logical_device, swapchain.GetImagesCount(), graphics_pipeline_ptr_->GetDescriptorSetLayout());

			for (size_t i = 0; i < swapchain.GetImagesCount(); i++)
			{
				swapchain_frame_buffers_.emplace_back(device_cfg_.logical_device, swapchain.GetExtent(), swapchain.GetImageView(i), depth_view, *render_pass_ptr_);
			}

			graphics_command_pool_ptr_->CreateCommandBuffers(static_cast<uint32_t>(swapchain_frame_buffers_.size()));

			vkDestroyShaderModule(device_cfg_.logical_device, frag_shader_module, nullptr);
			vkDestroyShaderModule(device_cfg_.logical_device, vert_shader_module, nullptr);
		}


		void createSemaphores() {
			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			if (vkCreateSemaphore(device_cfg_.logical_device, &semaphoreInfo, nullptr, &image_available_semaphore_) != VK_SUCCESS ||
				vkCreateSemaphore(device_cfg_.logical_device, &semaphoreInfo, nullptr, &render_finished_semaphore_) != VK_SUCCESS) {

				throw std::runtime_error("failed to create semaphores!");
			}
		}

		void cleanup() {
			//vkDestroyPipelineLayout(vk_logical_devices_[selected_device_index], pipelineLayout, nullptr);
		}

		void ShowWindow()
		{
			platform::ShowWindow(surface_ptr_->GetWindow());

			createSemaphores();
			
			bool should_refresh_swapchain = true;

			std::vector<FrameHandler> frames;

			while (should_refresh_swapchain)
			{
				should_refresh_swapchain = false;

				Swapchain swapchain(device_cfg_, *surface_ptr_);

				Image image = Image::FromFile(device_cfg_, "../images/test.jpg");
				ImageView image_view(device_cfg_, image);
				Sampler sampler(device_cfg_);

				VkFormat depth_format = VK_FORMAT_D32_SFLOAT;
				Image depth_image(device_cfg_, depth_format, swapchain.GetExtent().width, swapchain.GetExtent().height, Image::ImageType::kDepthImage);
				ImageView depth_image_view(device_cfg_, depth_image);

				PrepareSwapChain(swapchain, depth_image_view);

				frames.clear();
				frames.reserve(swapchain_frame_buffers_.size());

				for (size_t frame_ind = 0; frame_ind < swapchain_frame_buffers_.size(); frame_ind++)
				{
					frames.emplace_back(device_cfg_, swapchain, frame_ind, graphics_command_pool_ptr_->GetCommandBuffer(frame_ind), render_finished_semaphore_);

					VkDescriptorBufferInfo buffer_info{};
					buffer_info.buffer = frames[frame_ind].GetUniformBuffer().GetHandle();
					buffer_info.offset = 0;
					buffer_info.range = sizeof(UniformBufferObject);

					VkDescriptorImageInfo image_info{};
					image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					image_info.imageView = image_view.GetHandle();
					image_info.sampler = sampler.GetSamplerHandle();

					std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

					descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptor_writes[0].dstSet = descriptor_pool_ptr_->GetDescriptorSet(frame_ind);
					descriptor_writes[0].dstBinding = 0;
					descriptor_writes[0].dstArrayElement = 0;
					descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					descriptor_writes[0].descriptorCount = 1;
					descriptor_writes[0].pBufferInfo = &buffer_info;

					descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptor_writes[1].dstSet = descriptor_pool_ptr_->GetDescriptorSet(frame_ind);
					descriptor_writes[1].dstBinding = 1;
					descriptor_writes[1].dstArrayElement = 0;
					descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptor_writes[1].descriptorCount = 1;
					descriptor_writes[1].pImageInfo = &image_info;

					vkUpdateDescriptorSets(device_cfg_.logical_device, 2, descriptor_writes.data(), 0, nullptr);

					FillGraphicsBuffer(graphics_command_pool_ptr_->GetCommandBuffer(frame_ind), swapchain_frame_buffers_[frame_ind], swapchain.GetExtent(), frame_ind);
				}

				std::chrono::high_resolution_clock clock;
				std::vector<std::pair<int, long long>> durations;
				auto global_start = clock.now();
				int frames_cnt = 0;
				while (!platform::IsWindowClosed(surface_ptr_->GetWindow()) && !should_refresh_swapchain)
				{
					uint32_t image_index;

					auto start = clock.now();

					VkResult result = vkAcquireNextImageKHR(device_cfg_.logical_device, swapchain.GetHandle(), UINT64_MAX,
						image_available_semaphore_, VK_NULL_HANDLE, &image_index);

					auto end = clock.now();

					auto duration = end - start;

					if (durations.size() < 10000)
					{
						durations.push_back(std::make_pair(image_index, duration.count()));
					}
					frames_cnt++;
					if (std::chrono::duration_cast<std::chrono::milliseconds>(clock.now() - global_start).count() >= 1000)
					{
						std::cout<< frames_cnt <<std::endl;
						frames_cnt = 0;
						global_start = clock.now();
					}

					if (result != VK_SUCCESS)
					{
						if (result == VK_ERROR_OUT_OF_DATE_KHR)
						{
							should_refresh_swapchain = true;
						}
						continue;
					}

					should_refresh_swapchain = !frames[image_index].Process(image_available_semaphore_);

					int a = 1;

				}

				vkDeviceWaitIdle(device_cfg_.logical_device);
			}

			vkDestroySemaphore(device_cfg_.logical_device, image_available_semaphore_, nullptr);
			vkDestroySemaphore(device_cfg_.logical_device, render_finished_semaphore_, nullptr);


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
				InitPhysicalDeviceQueueFamaliesProperties(vk_physical_devices_[i]);
				InitPhysicalDeviceFeatures(vk_physical_devices_[i]);
				if (!(InitPhysicalDeviceLayers(vk_physical_devices_[i]) &&
					InitPhysicalDeviceExtensions(vk_physical_devices_[i])))
					return false;
			}
			return true;
		}

		bool InitLogicalDevices()
		{
			//TODO: add device selection
			//for (VkPhysicalDevice& physical_device : vk_physical_devices_)
			//{
			//	if (!InitLogicalDevice(physical_device))
			//		return false;
			//}
			//return true;
			return false;
	}

		uint32_t FindDeviceQueueFamalyWithFlag(const VkPhysicalDevice& physical_deivce, VkQueueFlags flags, VkQueueFlags flags_to_check_mask = 0)
		{
			auto&& queue_famalies_properties = vk_physical_devices_to_queues_[physical_deivce];

			if (flags_to_check_mask == 0)
			{
				flags_to_check_mask = flags;
			}

			for (uint32_t i = 0; i < queue_famalies_properties.size(); i++)
			{
				if ((queue_famalies_properties[i].queueFlags & flags_to_check_mask) == flags)
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
						physical_device_out = physical_device;
						break;
					}
				}

				if (physical_device_out != nullptr)
					break;
			}

			auto physical_device_out_it = std::find(vk_physical_devices_.begin(), vk_physical_devices_.end(), physical_device_out);

			if (physical_device_out_it != vk_physical_devices_.end())
			{
				return static_cast<uint32_t>(physical_device_out_it - vk_physical_devices_.begin());
			}

			return -1;
		}


		void InitPhysicalDeviceProperties(VkPhysicalDevice physical_device)
		{
			using PhDevPropMapType = std::map<VkPhysicalDevice, VkPhysicalDeviceMemoryProperties>;

			auto it = vk_physical_devices_propeties_.lower_bound(physical_device);

			if (it == vk_physical_devices_propeties_.end() || it->first != physical_device)
			{
				it = vk_physical_devices_propeties_.emplace_hint(it,
					std::piecewise_construct, std::forward_as_tuple(physical_device), std::forward_as_tuple());
				vkGetPhysicalDeviceMemoryProperties(physical_device, &it->second);

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



			logical_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			logical_device_create_info.pNext = nullptr;
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

		bool FillGraphicsBuffer(VkCommandBuffer command_buffer, const Framebuffer& framebuffer, const VkExtent2D& extent, uint32_t image_index)
		{
			VkCommandBufferBeginInfo begin_info{};
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Optional
			begin_info.pInheritanceInfo = nullptr; // Optional

			if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo render_pass_info{};
			render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_info.renderPass = render_pass_ptr_->GetRenderPassHandle();
			render_pass_info.framebuffer = framebuffer.GetFramebufferHandle();

			render_pass_info.renderArea.offset = { 0, 0 };
			render_pass_info.renderArea.extent = extent;

			std::array<VkClearValue, 2> clear_color;
			clear_color[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
			clear_color[1].depthStencil = {1.0f, 0};
			render_pass_info.clearValueCount = clear_color.size();
			render_pass_info.pClearValues = clear_color.data();

			vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_ptr_->GetHandle());

			VkBuffer vertexBuffers[] = { vertex_buffer_ptr_->GetHandle() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(command_buffer, index_buffer_ptr_->GetHandle(), 0, VK_INDEX_TYPE_UINT16);
			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_ptr_->GetLayout(), 0, 1, &descriptor_pool_ptr_->GetDescriptorSet(image_index), 0, nullptr);

			vkCmdDrawIndexed(command_buffer, faces_.size() * 3, 1, 0, 0, 0);

			vkCmdEndRenderPass(command_buffer);

			if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}

			return false;
		}

		bool CopyToGPUBuffer(const Buffer& dst_buffer, const void* data, uint64_t size)
		{
			if (transfer_command_pool_ptr_->CreateCommandBuffers(1))
			{
				std::vector<uint32_t> queue_indices = { device_cfg_.graphics_queue_index , device_cfg_.transfer_queue_index };

				Buffer staging_buffer(device_cfg_, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, queue_indices);

				staging_buffer.LoadData(data, size);

				transfer_command_pool_ptr_->ExecuteOneTimeCommand([size, &staging_buffer, &dst_buffer](VkCommandBuffer command_buffer) {

					VkBufferCopy copy_region{};
					copy_region.srcOffset = 0; // Optional
					copy_region.dstOffset = 0; // Optional
					copy_region.size = size;
					vkCmdCopyBuffer(command_buffer, staging_buffer.GetHandle(), dst_buffer.GetHandle(), 1, &copy_region);

					});

				return true;
			}

			return false;
		}


		bool vk_init_success_;
		VkApplicationInfo application_info_;
		VkInstanceWrapper vk_instance_;



		std::vector<VkPhysicalDevice> vk_physical_devices_;
		std::map<VkPhysicalDevice, VkPhysicalDeviceMemoryProperties> vk_physical_devices_propeties_;
		std::map<VkPhysicalDevice, std::vector<VkQueueFamilyProperties>> vk_physical_devices_to_queues_;
		std::map<VkPhysicalDevice, VkPhysicalDeviceFeatures> vk_physical_devices_features_;
		std::vector<VkLayerProperties > vk_instance_layers_;
		std::map<VkPhysicalDevice, std::vector<VkLayerProperties>> vk_physical_devices_layers_;
		std::vector<VkExtensionProperties> vk_instance_extensions_;
		std::map<VkPhysicalDevice, std::vector<VkExtensionProperties>> vk_physical_devices_extensions_;
		
		std::vector<VkDeviceWrapper> vk_logical_devices_;

		DeviceConfiguration device_cfg_;


		std::vector<Framebuffer> swapchain_frame_buffers_;

		std::unique_ptr<Surface> surface_ptr_;
		std::unique_ptr<Buffer> vertex_buffer_ptr_;
		std::unique_ptr<Buffer> index_buffer_ptr_;
		std::unique_ptr<RenderPass> render_pass_ptr_;
		std::unique_ptr<GraphicsPipeline> graphics_pipeline_ptr_;
		std::unique_ptr<DescriptorPool> descriptor_pool_ptr_;
		std::unique_ptr<CommandPool> graphics_command_pool_ptr_;
		std::unique_ptr<CommandPool> transfer_command_pool_ptr_;

		VkSemaphore image_available_semaphore_;
		VkSemaphore render_finished_semaphore_;

		const std::vector<Vertex> vertices_ = {
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
	{{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

	{{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
	{{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
		};

		const std::vector<Face> faces_ = {
			{0, 1, 2}, {2, 3, 0},
			{4, 5, 6}, {6, 7, 4}
		};

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