#include "render/render_engine.h"

#include "platform.h"
#include "stl_util.h"


#include <vector>
#include <map>
#include <tuple>
#include <fstream>

#include "vulkan/vulkan.h"
#include "surface.h"

#include <tchar.h>

#include "common.h"
#include "render/vk_util.h"
#include "render/framebuffer.h"
#include "render/graphics_pipeline.h"
#include "render/command_pool.h"
#include "render/frame_handler.h"

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

			if (!InitLogicalDevices())
			{
				LOG(err, "InitLogicalDevices error");
				return;
			}

			std::pair<uint32_t, uint32_t> device_and_queue_indices = DetermineDeviceAndQueueForUse();

			if (device_and_queue_indices.first < 0 || device_and_queue_indices.second < 0)
			{
				LOG(err, "Could not determine valid device or queue");
				return;
			}

			selected_device_index = device_and_queue_indices.first;
			selected_queue_index = device_and_queue_indices.second;

			vkGetDeviceQueue(vk_logical_devices_[selected_device_index], selected_queue_index, 0, &graphics_queue);

			platform::Window window = platform::CreatePlatformWindow(param);

			surface_ptr_ = std::make_unique<Surface>(window, vk_instance_, vk_physical_devices_[device_and_queue_indices.first], selected_queue_index, vk_logical_devices_[selected_device_index]);

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
			if (vkCreateShaderModule(vk_logical_devices_[selected_device_index], &createInfo, nullptr, &shader_module) != VK_SUCCESS) {
				throw std::runtime_error("failed to create shader module!");
			}

			return shader_module;
		}

		void PrepareSwapChain()
		{
			swapchain_frame_buffers_.clear();
			command_pool_ptr->ClearCommandBuffers();
			graphics_pipeline_ptr.reset();
			render_pass_ptr_.reset();

			surface_ptr_->RefreshSwapchain();

			const VkDevice& device = vk_logical_devices_[selected_device_index];
			const VkExtent2D& extent = surface_ptr_->GetSwapchainExtent();

			render_pass_ptr_ = std::make_unique<RenderPass>(device, surface_ptr_->GetSwapchainFormat());

			auto vert_shader_code = readFile("shaders/vert.spv");
			auto frag_shader_code = readFile("shaders/frag.spv");

			VkShaderModule vert_shader_module = createShaderModule(vert_shader_code);
			VkShaderModule frag_shader_module = createShaderModule(frag_shader_code);

			graphics_pipeline_ptr = std::make_unique<GraphicsPipeline>(device, vert_shader_module, frag_shader_module, extent, *render_pass_ptr_);


			swapchain_frame_buffers_.reserve(surface_ptr_->GetImageViews().size());

			for (size_t i = 0; i < surface_ptr_->GetImageViews().size(); i++)
			{
				swapchain_frame_buffers_.emplace_back(device, extent, surface_ptr_->GetImageViews()[i], *render_pass_ptr_);
			}

			command_pool_ptr->CreateCommandBuffers(static_cast<uint32_t>(swapchain_frame_buffers_.size()));

			for (size_t ind = 0; ind < swapchain_frame_buffers_.size(); ind++)
			{
				command_pool_ptr->FillCommandBuffer(ind, *graphics_pipeline_ptr, extent, *render_pass_ptr_, swapchain_frame_buffers_[ind]);
			}

			vkDestroyShaderModule(vk_logical_devices_[selected_device_index], frag_shader_module, nullptr);
			vkDestroyShaderModule(vk_logical_devices_[selected_device_index], vert_shader_module, nullptr);
		}


		void createFramebuffers(const RenderPass& render_pass) {

			swapchain_frame_buffers_.clear();
			swapchain_frame_buffers_.reserve(surface_ptr_->GetImageViews().size());

			for (size_t i = 0; i < surface_ptr_->GetImageViews().size(); i++)
			{
				swapchain_frame_buffers_.emplace_back(vk_logical_devices_[selected_device_index], surface_ptr_->GetSwapchainExtent(), surface_ptr_->GetImageViews()[i], render_pass);
			}

		}

		void createSemaphores() {
			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			if (vkCreateSemaphore(vk_logical_devices_[selected_device_index], &semaphoreInfo, nullptr, &image_available_semaphore) != VK_SUCCESS ||
				vkCreateSemaphore(vk_logical_devices_[selected_device_index], &semaphoreInfo, nullptr, &render_finished_semaphore) != VK_SUCCESS) {

				throw std::runtime_error("failed to create semaphores!");
			}
		}

		void cleanup() {
			//vkDestroyPipelineLayout(vk_logical_devices_[selected_device_index], pipelineLayout, nullptr);
		}

		void ShowWindow()
		{
			platform::ShowWindow(surface_ptr_->GetWindow());

			command_pool_ptr = std::make_unique<CommandPool>(vk_logical_devices_[selected_device_index], selected_queue_index);

			createSemaphores();
			
			
			bool should_refresh_swapchain = true;

			const VkDevice& device = vk_logical_devices_[selected_device_index];

			std::vector<FrameHandler> frames;

			while (should_refresh_swapchain)
			{
				should_refresh_swapchain = false;

				PrepareSwapChain();

				frames.clear();
				frames.reserve(swapchain_frame_buffers_.size());

				for (size_t frame_ind = 0; frame_ind < swapchain_frame_buffers_.size(); frame_ind++)
				{
					frames.emplace_back(device, graphics_queue, surface_ptr_->GetSwapchain(), frame_ind, command_pool_ptr->GetCommandBuffer(frame_ind), render_finished_semaphore);
				}


				while (!platform::IsWindowClosed(surface_ptr_->GetWindow()) && !should_refresh_swapchain)
				{
					uint32_t image_index;
					VkResult result = vkAcquireNextImageKHR(device, surface_ptr_->GetSwapchain(), UINT64_MAX,
						image_available_semaphore, VK_NULL_HANDLE, &image_index);

					if (result != VK_SUCCESS)
					{
						if (result == VK_ERROR_OUT_OF_DATE_KHR)
						{
							should_refresh_swapchain = true;
						}
						continue;
					}

					should_refresh_swapchain = !frames[image_index].Process(image_available_semaphore);

				}

				vkDeviceWaitIdle(device);
			}

			vkDestroySemaphore(device, image_available_semaphore, nullptr);
			vkDestroySemaphore(device, render_finished_semaphore, nullptr);


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
			for (VkPhysicalDevice& physical_device : vk_physical_devices_)
			{
				if (!InitLogicalDevice(physical_device))
					return false;
			}
			return true;
	}

		std::pair<uint32_t, uint32_t> DetermineDeviceAndQueueForUse()
		{
			VkPhysicalDevice physical_device_out = nullptr;

			uint32_t device_queue_famaly_ind = -1;


			for (auto&& [physical_device, queues_properties] : vk_physical_devices_queues_)
			{
				for (uint32_t i = 0; i < queues_properties.size(); i++)
				{
					if (platform::GetPhysicalDevicePresentationSupport(physical_device, i) && (queues_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
					{
						physical_device_out = physical_device;
						device_queue_famaly_ind = i;
						break;
					}
				}

				if (device_queue_famaly_ind != -1)
					break;
			}

			auto physical_device_out_it = std::find(vk_physical_devices_.begin(), vk_physical_devices_.end(), physical_device_out);

			if (physical_device_out_it != vk_physical_devices_.end())
				return { static_cast<uint32_t>(physical_device_out_it - vk_physical_devices_.begin()), device_queue_famaly_ind };

			return { -1, -1 };
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

			auto it = vk_physical_devices_queues_.lower_bound(physical_device);

			if (it == vk_physical_devices_queues_.end() || it->first != physical_device)
			{
				uint32_t device_queue_famaly_prop_cnt;
				vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &device_queue_famaly_prop_cnt, nullptr);

				it = vk_physical_devices_queues_.emplace_hint(it,
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

		bool InitLogicalDevice(const VkPhysicalDevice& physical_device)
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


			//TODO: select and create only graphic queue
			VkDeviceQueueCreateInfo queueu_create_info{};
			queueu_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueu_create_info.flags = 0;
			queueu_create_info.queueFamilyIndex = 0;

			uint32_t device_queue_count = vk_physical_devices_queues_[physical_device][0].queueCount;

			queueu_create_info.queueCount = device_queue_count;

			std::vector<float> queue_priorities(device_queue_count);
			for (auto&& priority : queue_priorities) priority = 1.0f;

			queueu_create_info.pQueuePriorities = queue_priorities.data();
			queueu_create_info.pNext = nullptr;

			logical_device_create_info.queueCreateInfoCount = 1;
			logical_device_create_info.pQueueCreateInfos = &queueu_create_info;

			vk_logical_devices_.emplace_back();

			if (vkCreateDevice(physical_device, &logical_device_create_info, nullptr, &vk_logical_devices_[vk_logical_devices_.size() - 1]) != VK_SUCCESS)
				return false;
			return true;
		}

		bool vk_init_success_;
		VkApplicationInfo application_info_;
		VkInstanceWrapper vk_instance_;



		std::vector<VkPhysicalDevice> vk_physical_devices_;
		std::map<VkPhysicalDevice, VkPhysicalDeviceMemoryProperties> vk_physical_devices_propeties_;
		std::map<VkPhysicalDevice, std::vector<VkQueueFamilyProperties>> vk_physical_devices_queues_;
		std::map<VkPhysicalDevice, VkPhysicalDeviceFeatures> vk_physical_devices_features_;
		std::vector<VkLayerProperties > vk_instance_layers_;
		std::map<VkPhysicalDevice, std::vector<VkLayerProperties>> vk_physical_devices_layers_;
		std::vector<VkExtensionProperties> vk_instance_extensions_;
		std::map<VkPhysicalDevice, std::vector<VkExtensionProperties>> vk_physical_devices_extensions_;
		
		std::vector<VkDeviceWrapper> vk_logical_devices_;


		uint32_t selected_queue_index;
		uint32_t selected_device_index;

		std::vector<render::Framebuffer> swapchain_frame_buffers_;

		std::unique_ptr<Surface> surface_ptr_;
		std::unique_ptr<RenderPass> render_pass_ptr_;
		std::unique_ptr<GraphicsPipeline> graphics_pipeline_ptr;
		std::unique_ptr<CommandPool> command_pool_ptr;

		VkSemaphore image_available_semaphore;
		VkSemaphore render_finished_semaphore;

		VkQueue graphics_queue;

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