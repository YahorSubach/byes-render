#include "render_system.h"

#include "platform.h"

namespace render
{
	
	RenderSystem::RenderSystem(platform::Window window, const std::string& app_name): api_instance_(global_, app_name), surface_(window, api_instance_, global_)
	{
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

		auto physical_device = vk_physical_devices_[selected_device_index];


		selected_graphics_queue_index_ = FindDeviceQueueFamalyWithFlag(physical_device, VK_QUEUE_GRAPHICS_BIT);
		selected_transfer_queue_index_ = FindDeviceQueueFamalyWithFlag(physical_device, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT);
		if (selected_transfer_queue_index_ == -1)
		{
			selected_transfer_queue_index_ = FindDeviceQueueFamalyWithFlag(physical_device, VK_QUEUE_TRANSFER_BIT);
		}

		std::vector<uint32_t> queue_indices_to_use;
		queue_indices_to_use.push_back(selected_graphics_queue_index_);
		if (selected_graphics_queue_index_ != selected_transfer_queue_index_)
		{
			queue_indices_to_use.push_back(selected_transfer_queue_index_);
		}

		if (!InitLogicalDevice(physical_device, queue_indices_to_use))
		{
			LOG(err, "InitLogicalDevices error");
			return;
		}

		graphics_command_pool_ptr_ = std::make_unique<CommandPool>(global_, CommandPool::PoolType::kGraphics);
		transfer_command_pool_ptr_ = std::make_unique<CommandPool>(global_, CommandPool::PoolType::kTransfer);

		FillGlobal();
	}

	void RenderSystem::FillGlobal()
	{
		global_.physical_device = vk_physical_devices_[selected_device_index_];
		global_.physical_device_properties = vk_physical_devices_propeties_.at(global_.physical_device);

		global_.graphics_queue_index = selected_graphics_queue_index_;
		global_.transfer_queue_index = selected_transfer_queue_index_;
		global_.logical_device = vk_logical_devices_[0];

		//TODO: ugly motherfucker. Do smthing with this queue init..

		vkGetDeviceQueue(global_.logical_device, global_.graphics_queue_index, 0, &global_.graphics_queue);
		if (global_.graphics_queue_index != global_.transfer_queue_index)
		{
			vkGetDeviceQueue(global_.logical_device, global_.transfer_queue_index, 0, &global_.transfer_queue);
		}
		else
		{
			global_.transfer_queue = global_.graphics_queue;
		}

		global_.graphics_cmd_pool = graphics_command_pool_ptr_.get();
		global_.transfer_cmd_pool = transfer_command_pool_ptr_.get();
	
		Image error_image(global_, Image::BuiltinImageType::kError);
		global_.error_image = error_image;

		Image default_normal(global_, Image::BuiltinImageType::kNormal);
		global_.default_normal = default_normal;

		global_.presentation_format = surface_.GetSurfaceFormat(global_.physical_device).format;

		for (int i = 0; i < 16; i++)
		{
			global_.mipmap_cnt_to_global_samplers.push_back(Sampler(global_, i));
		}

		global_.nearest_sampler.emplace(Sampler(global_, 0, Sampler::AddressMode::kClampToBorder, true));

		DescriptorSetsManager descriptor_set_manager(global_);
	}

	void RenderSystem::Render()
	{
	}
	bool RenderSystem::InitPhysicalDevices()
	{
		uint32_t physical_devices_count;

		if (vkEnumeratePhysicalDevices(api_instance_.GetHandle(), &physical_devices_count, nullptr) != VK_SUCCESS)
			return false;

		vk_physical_devices_.resize(physical_devices_count);

		if (vkEnumeratePhysicalDevices(api_instance_.GetHandle(), &physical_devices_count, reinterpret_cast<VkPhysicalDevice*>(vk_physical_devices_.data())) != VK_SUCCESS)
			return false;
		//TODO: add device selection
		for (size_t i = 0; i < physical_devices_count; i++)
		{
			InitPhysicalDeviceProperties(vk_physical_devices_[i]);
			InitPhysicalDeviceMemoryProperties(vk_physical_devices_[i]);
			InitPhysicalDeviceQueueFamiliesProperties(vk_physical_devices_[i]);
			InitPhysicalDeviceFeatures(vk_physical_devices_[i]);
			if (!(InitPhysicalDeviceLayers(vk_physical_devices_[i]) &&
				InitPhysicalDeviceExtensions(vk_physical_devices_[i])))
				return false;
		}
		return true;
	}
	void RenderSystem::InitPhysicalDeviceProperties(VkPhysicalDevice physical_device)
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

	void RenderSystem::InitPhysicalDeviceMemoryProperties(VkPhysicalDevice physical_device)
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

	void RenderSystem::InitPhysicalDeviceQueueFamiliesProperties(VkPhysicalDevice physical_device)
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

	void RenderSystem::InitPhysicalDeviceFeatures(const VkPhysicalDevice& physical_device)
	{
		using PhDevPropMapType = std::map<VkPhysicalDevice, VkPhysicalDeviceFeatures>;

		auto it = vk_physical_devices_features_.lower_bound(physical_device);

		if (it == vk_physical_devices_features_.end() || it->first != physical_device)
		{
			it = vk_physical_devices_features_.emplace_hint(it,
				std::piecewise_construct, std::forward_as_tuple(physical_device), std::forward_as_tuple());
			vkGetPhysicalDeviceFeatures(physical_device, &it->second);

			VkPhysicalDeviceFeatures2 features2;
			//VkPhysicalDeviceImagelessFramebufferFeatures imageless_features;
			VkPhysicalDeviceSynchronization2Features synchronization2_features;
			VkPhysicalDeviceVulkan13Features vk13_features;

			features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			features2.pNext = &synchronization2_features;

			//imageless_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES;
			//imageless_features.pNext = &synchronization2_features;

			synchronization2_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
			synchronization2_features.pNext = &vk13_features;

			vk13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
			vk13_features.pNext = nullptr;


			vkGetPhysicalDeviceFeatures2(physical_device, &features2);
			int a = 1;
		}
	}

	bool RenderSystem::InitPhysicalDeviceLayers(const VkPhysicalDevice& physical_device)
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

	bool RenderSystem::InitPhysicalDeviceExtensions(const VkPhysicalDevice& physical_device)
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

	uint32_t RenderSystem::DetermineDeviceForUse()
	{
		VkPhysicalDevice physical_device_out = nullptr;

		for (auto&& [physical_device, queues_properties] : vk_physical_devices_to_queues_)
		{
			for (uint32_t i = 0; i < queues_properties.size(); i++)
			{
				if (platform::GetPhysicalDevicePresentationSupport(physical_device, i) && (queues_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
				{
					if (physical_device_out == nullptr ||
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


	bool RenderSystem::InitLogicalDevice(const VkPhysicalDevice& physical_device, const std::vector<uint32_t> queue_famaly_indices)
	{
		VkDeviceCreateInfo logical_device_create_info{};

		std::vector<const char*> device_extensions;

		for (auto&& req : GetRequiredDeviceExtensions())
		{
			auto find_it = std::find_if(vk_physical_devices_extensions_[physical_device].begin(), vk_physical_devices_extensions_[physical_device].end(), [&](auto&& ext) { return std::strcmp(req, ext.extensionName) == 0; });
			if (find_it != vk_physical_devices_extensions_[physical_device].end())
			{
				device_extensions.push_back(req);
			}
			else
			{
				using namespace std::chrono_literals;
				std::cout << "Missing extension: " << req << std::endl;
				//std::this_thread::sleep_for(5000ms);
			}
		}

		logical_device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
		logical_device_create_info.ppEnabledExtensionNames = device_extensions.data();

		//if (util::All(GetRequiredDeviceExtensions(), vk_physical_devices_extensions_[physical_device], [](auto&& ext_name, auto&& ext) { return std::strcmp(ext_name, ext.extensionName) == 0; }))
		//{
		//	logical_device_create_info.enabledExtensionCount = static_cast<uint32_t>(GetRequiredDeviceExtensions().size());
		//	logical_device_create_info.ppEnabledExtensionNames = GetRequiredDeviceExtensions().data();
		//}
		//else return false;

		//VkPhysicalDeviceImagelessFramebufferFeatures imageless_features;
		//imageless_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES;
		//imageless_features.pNext = nullptr;
		//imageless_features.imagelessFramebuffer = VK_TRUE;
		//VkPhysicalDeviceSynchronization2Features vk_synchronization2_features = {};
		VkPhysicalDeviceVulkan13Features vk13_features = {};

		//vk_synchronization2_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
		//vk_synchronization2_features.synchronization2 = VK_TRUE;
		//vk_synchronization2_features.pNext = &vk13_features;

		vk13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		vk13_features.synchronization2 = VK_TRUE;
		vk13_features.pNext = nullptr;

		logical_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		logical_device_create_info.pNext = &vk13_features;
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

	uint32_t RenderSystem::FindDeviceQueueFamalyWithFlag(const VkPhysicalDevice& physical_deivce, VkQueueFlags enabled_flags, VkQueueFlags disabled_flags)
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

	const std::vector<const char*>& RenderSystem::GetRequiredDeviceExtensions()
	{
#ifndef NDEBUG1
		static const std::vector<const char*> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_EXT_DEBUG_MARKER_EXTENSION_NAME };
#else
		static const std::vector<const char*> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME};
#endif // !NDEBUG

		return extensions;
	}


	
}
