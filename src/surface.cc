
#include "common.h"
#include "surface.h"

#include "stl_util.h"

#include <functional>

namespace render
{

	RenderApiInstance::RenderApiInstance(const Global& global, const std::string& app_name) :RenderObjBase<VkInstance>(global)
	{
		valid_ = false;

		if (!InitInstanceExtensions())
		{
			LOG(err, "Could not init InstanceExtensions");
			return;
		}

		if (!util::All(platform::GetRequiredInstanceExtensions(), vk_instance_extensions_, [](auto&& ext_name, auto&& ext) { return std::strcmp(ext_name, ext.extensionName) == 0; }))
		{
			LOG(err, "Required platform Extension missing");
			return;
		}

		if (!InitInstanceLayers())
		{
			LOG(err, "InitInstanceLayers error");
			return;
		}

		if (!util::All(GetValidationLayers(), vk_instance_layers_, [](auto&& ext_name, auto&& ext) { return std::strcmp(ext_name, ext.layerName) == 0; }))
		{
			LOG(err, "Instance Layers missing");
			return;
		}

		VkApplicationInfo application_info;

		application_info.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0); // CHECK WHAT DOES THIS MEAN!
		application_info.applicationVersion = 1;
		application_info.engineVersion = 1;
		application_info.pApplicationName = app_name.c_str();
		application_info.pEngineName = "byes_render_engine";
		application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		application_info.pNext = nullptr;

		VkInstanceCreateInfo instance_create_info{};
		instance_create_info.enabledExtensionCount = static_cast<uint32_t>(platform::GetRequiredInstanceExtensions().size());
		instance_create_info.ppEnabledExtensionNames = platform::GetRequiredInstanceExtensions().data();
		instance_create_info.enabledLayerCount = static_cast<uint32_t>(GetValidationLayers().size());
		instance_create_info.ppEnabledLayerNames = GetValidationLayers().data();
		instance_create_info.pApplicationInfo = &application_info;
		instance_create_info.pNext = nullptr;
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;


		if (vkCreateInstance(&instance_create_info, nullptr, &handle_) != VK_SUCCESS)
		{
			LOG(err, "vkCreateInstance error");
			return;
		}

	}
	RenderApiInstance::~RenderApiInstance()
	{
		if (handle_ != VK_NULL_HANDLE)
		{
			vkDestroyInstance(handle_, nullptr);
		}
	}

	const std::vector<const char*>& RenderApiInstance::GetValidationLayers()
	{
#ifndef NDEBUG1
		static const std::vector<const char*> layers{ "VK_LAYER_KHRONOS_validation"/*, "VK_LAYER_RENDERDOC_Capture"*/};
		return layers;
#else
		static const std::vector<const char*> layers{};
		return layers;
#endif // !NDEBUG
	}

	bool RenderApiInstance::InitInstanceLayers()
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

	bool RenderApiInstance::InitInstanceExtensions()
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



	Surface::Surface(platform::Window window_hande, const RenderApiInstance& instance, const Global& global) :
		RenderObjBase(global), window_hande_(window_hande)
	{
		if (window_hande_)
		{
			if (platform::CreateSurface(instance.GetHandle(), window_hande_, handle_))
			{
				valid_ = true;
			}
		}
	}

	platform::Window Surface::GetWindow()
	{
		return window_hande_;
	}

	Surface::~Surface()
	{
		if (handle_ != VK_NULL_HANDLE)
		{
		}
	}

	VkSurfaceFormatKHR Surface::GetSurfaceFormat(const VkPhysicalDevice& physical_device) const
	{
		auto formats = util::GetSizeThenAlocThenGetDataPtrPtr(vkGetPhysicalDeviceSurfaceFormatsKHR, physical_device, handle_);

		if (formats.size() == 0)
			throw std::runtime_error("No surface formats detected. Epic fail:(");


		for (auto format : formats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB)
				return format;
		}

		assert(false);
		return formats[0]; //TODO fuck
	}

	VkPresentModeKHR Surface::GetSurfacePresentMode(const VkPhysicalDevice& physical_device) const
	{
		auto presentat_modes = util::GetSizeThenAlocThenGetDataPtrPtr(vkGetPhysicalDeviceSurfacePresentModesKHR, physical_device, handle_);

		for (auto mode : presentat_modes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
				return mode;
		}

		for (auto mode : presentat_modes)
		{
			if (mode == VK_PRESENT_MODE_FIFO_KHR)
				return mode;
		}

		for (auto mode : presentat_modes)
		{
			return mode;
		}


		return presentat_modes[2]; //TODO fuck
	}

	Extent Surface::GetSwapExtend(const VkSurfaceCapabilitiesKHR& capabilities) const
	{
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		else
		{
			VkExtent2D extent = platform::GetWindowExtent(window_hande_);

			extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return extent;
		}
	}

}