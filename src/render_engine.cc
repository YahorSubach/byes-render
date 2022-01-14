#include "render/render_engine.h"

#include "platform.h"
#include "stl_util.h"

#include <tchar.h>


#include <vector>
#include <map>
#include <tuple>
#include <fstream>

#include "vulkan/vulkan.h"


#include "common.h"
#include "surface.h"
#include "render/vk_util.h"
#include "render/descriptor_pool.h"
#include "render/framebuffer.h"
#include "render/graphics_pipeline.h"
#include "render/command_pool.h"
#include "render/frame_handler.h"
#include "render/buffer.h"
#include "render/data_types.h"

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

			selected_device_index_ = DetermineDeviceForUse();

			selected_physical_device_ = vk_physical_devices_[selected_device_index_];

			selected_graphics_queue_index_ = FindDeviceQueueFamalyWithFlag(selected_physical_device_, VK_QUEUE_GRAPHICS_BIT);
			selected_transfer_queue_index_ = FindDeviceQueueFamalyWithFlag(selected_physical_device_, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT);

			if (!InitLogicalDevice(selected_physical_device_, { selected_graphics_queue_index_ , selected_transfer_queue_index_}))
			{
				LOG(err, "InitLogicalDevices error");
				return;
			}

			selected_logical_device_ = vk_logical_devices_[selected_device_index_];



			if (selected_device_index_ == -1)
			{
				LOG(err, "Could not determine valid device");
				return;
			}

			vkGetDeviceQueue(vk_logical_devices_[selected_device_index_], selected_graphics_queue_index_, 0, &graphics_queue);
			vkGetDeviceQueue(vk_logical_devices_[selected_device_index_], selected_transfer_queue_index_, 0, &transfer_queue);

			platform::Window window = platform::CreatePlatformWindow(param);

			surface_ptr_ = std::make_unique<Surface>(window, vk_instance_, vk_physical_devices_[selected_device_index_], selected_graphics_queue_index_, vk_logical_devices_[selected_device_index_]);

			std::vector<uint32_t> queue_indices = { selected_graphics_queue_index_ , selected_transfer_queue_index_ };

			vertex_buffer_ptr_ = std::make_unique<Buffer>(selected_logical_device_, selected_physical_device_, sizeof(vertices_[0]) * vertices_.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, queue_indices);
			index_buffer_ptr_ = std::make_unique<Buffer>(selected_logical_device_, selected_physical_device_, sizeof(faces_[0]) * faces_.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, queue_indices);

			graphics_command_pool_ptr_ = std::make_unique<CommandPool>(selected_logical_device_, selected_graphics_queue_index_);
			transfer_command_pool_ptr_ = std::make_unique<CommandPool>(selected_logical_device_, selected_transfer_queue_index_);

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
			if (vkCreateShaderModule(vk_logical_devices_[selected_device_index_], &createInfo, nullptr, &shader_module) != VK_SUCCESS) {
				throw std::runtime_error("failed to create shader module!");
			}

			return shader_module;
		}

		void PrepareSwapChain()
		{
			swapchain_frame_buffers_.clear();
			graphics_command_pool_ptr_->ClearCommandBuffers();
			graphics_pipeline_ptr_.reset();
			descriptor_pool_ptr_.reset();

			surface_ptr_->RefreshSwapchain();

			render_pass_ptr_ = std::make_unique<RenderPass>(selected_logical_device_, surface_ptr_->GetSwapchainFormat());

			const VkExtent2D& extent = surface_ptr_->GetSwapchainExtent();

			auto vert_shader_code = readFile("shaders/vert.spv");
			auto frag_shader_code = readFile("shaders/frag.spv");

			VkShaderModule vert_shader_module = createShaderModule(vert_shader_code);
			VkShaderModule frag_shader_module = createShaderModule(frag_shader_code);

			graphics_pipeline_ptr_ = std::make_unique<GraphicsPipeline>(selected_logical_device_, vert_shader_module, frag_shader_module, extent, *render_pass_ptr_);


			swapchain_frame_buffers_.reserve(surface_ptr_->GetImageViews().size());

			descriptor_pool_ptr_ = std::make_unique<DescriptorPool>(selected_logical_device_, surface_ptr_->GetImageViews().size(), graphics_pipeline_ptr_->GetDescriptorSetLayout());

			for (size_t i = 0; i < surface_ptr_->GetImageViews().size(); i++)
			{
				swapchain_frame_buffers_.emplace_back(selected_logical_device_, extent, surface_ptr_->GetImageViews()[i], *render_pass_ptr_);
			}

			graphics_command_pool_ptr_->CreateCommandBuffers(static_cast<uint32_t>(swapchain_frame_buffers_.size()));

			vkDestroyShaderModule(selected_logical_device_, frag_shader_module, nullptr);
			vkDestroyShaderModule(selected_logical_device_, vert_shader_module, nullptr);
		}


		void createSemaphores() {
			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			if (vkCreateSemaphore(vk_logical_devices_[selected_device_index_], &semaphoreInfo, nullptr, &image_available_semaphore_) != VK_SUCCESS ||
				vkCreateSemaphore(vk_logical_devices_[selected_device_index_], &semaphoreInfo, nullptr, &render_finished_semaphore_) != VK_SUCCESS) {

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

				PrepareSwapChain();

				frames.clear();
				frames.reserve(swapchain_frame_buffers_.size());

				for (size_t frame_ind = 0; frame_ind < swapchain_frame_buffers_.size(); frame_ind++)
				{
					frames.emplace_back(selected_logical_device_, selected_physical_device_, graphics_queue, surface_ptr_->GetSwapchain(), frame_ind, graphics_command_pool_ptr_->GetCommandBuffer(frame_ind), render_finished_semaphore_);

					VkDescriptorBufferInfo buffer_info{};
					buffer_info.buffer = frames[frame_ind].GetUniformBuffer().GetBuffer();
					buffer_info.offset = 0;
					buffer_info.range = sizeof(UniformBufferObject);

					VkWriteDescriptorSet descriptor_write{};
					descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptor_write.dstSet = descriptor_pool_ptr_->GetDescriptorSet(frame_ind);
					descriptor_write.dstBinding = 0;
					descriptor_write.dstArrayElement = 0;

					descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					descriptor_write.descriptorCount = 1;

					descriptor_write.pBufferInfo = &buffer_info;
					descriptor_write.pImageInfo = nullptr; // Optional
					descriptor_write.pTexelBufferView = nullptr; // Optional

					vkUpdateDescriptorSets(selected_logical_device_, 1, &descriptor_write, 0, nullptr);

					FillGraphicsBuffer(graphics_command_pool_ptr_->GetCommandBuffer(frame_ind), swapchain_frame_buffers_[frame_ind], surface_ptr_->GetSwapchainExtent(), frame_ind);
				}


				while (!platform::IsWindowClosed(surface_ptr_->GetWindow()) && !should_refresh_swapchain)
				{
					uint32_t image_index;
					VkResult result = vkAcquireNextImageKHR(selected_logical_device_, surface_ptr_->GetSwapchain(), UINT64_MAX,
						image_available_semaphore_, VK_NULL_HANDLE, &image_index);


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

				vkDeviceWaitIdle(selected_logical_device_);
			}

			vkDestroySemaphore(selected_logical_device_, image_available_semaphore_, nullptr);
			vkDestroySemaphore(selected_logical_device_, render_finished_semaphore_, nullptr);


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



			logical_device_create_info.queueCreateInfoCount = queue_create_infos.size();
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

			VkClearValue clear_color = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
			render_pass_info.clearValueCount = 1;
			render_pass_info.pClearValues = &clear_color;

			vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_ptr_->GetPipelineHandle());

			VkBuffer vertexBuffers[] = { vertex_buffer_ptr_->GetBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(command_buffer, index_buffer_ptr_->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);
			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_ptr_->GetLayout(), 0, 1, &descriptor_pool_ptr_->GetDescriptorSet(image_index), 0, nullptr);

			vkCmdDrawIndexed(command_buffer, faces_.size() * 3, 1, 0, 0, 0);

			vkCmdEndRenderPass(command_buffer);

			if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}

			return false;
		}

		bool FillTransferBuffer(VkCommandBuffer command_buffer, VkBuffer src_buffer, VkBuffer dst_buffer,  uint64_t size)
		{
			VkCommandBufferBeginInfo begin_info{};
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkBufferCopy copy_region{};
			copy_region.srcOffset = 0; // Optional
			copy_region.dstOffset = 0; // Optional
			copy_region.size = size;
			vkCmdCopyBuffer(command_buffer, src_buffer ,dst_buffer, 1, &copy_region);

			vkEndCommandBuffer(command_buffer);

			return true;
		}

		bool CopyToGPUBuffer(const Buffer& dst_buffer, const void* data, uint64_t size)
		{
			if (transfer_command_pool_ptr_->CreateCommandBuffers(1))
			{
				std::vector<uint32_t> queue_indices = { selected_graphics_queue_index_ , selected_transfer_queue_index_ };

				Buffer staging_buffer(selected_logical_device_, selected_physical_device_, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, queue_indices);

				void* mapped_data;
				vkMapMemory(selected_logical_device_, staging_buffer.GetBufferMemory(), 0, size, 0, &mapped_data);

				memcpy(mapped_data, data, size);

				vkUnmapMemory(selected_logical_device_, staging_buffer.GetBufferMemory());

				FillTransferBuffer(transfer_command_pool_ptr_->GetCommandBuffer(0), staging_buffer.GetBuffer(), dst_buffer.GetBuffer(), size);

				VkSubmitInfo submit_info{};
				submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submit_info.commandBufferCount = 1;
				submit_info.pCommandBuffers = &transfer_command_pool_ptr_->GetCommandBuffer(0);

				if (vkQueueSubmit(transfer_queue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS || vkQueueWaitIdle(transfer_queue) != VK_SUCCESS)
					return false;

				transfer_command_pool_ptr_->ClearCommandBuffers();
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

		uint32_t selected_device_index_;
		VkPhysicalDevice selected_physical_device_;
		VkDevice selected_logical_device_;
		uint32_t selected_graphics_queue_index_;
		uint32_t selected_transfer_queue_index_;

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

		VkQueue graphics_queue;
		VkQueue transfer_queue;

		const std::vector<Vertex> vertices_ = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
		};

		const std::vector<Face> faces_ = {
			{0, 1, 2}, {2, 3, 0}
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