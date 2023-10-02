#ifndef RENDER_ENGINE_RENDER_RENDER_SYSTEM_H_
#define RENDER_ENGINE_RENDER_RENDER_SYSTEM_H_

#include <vector>
#include <map>
#include <memory>


#include "vulkan/vulkan.h"

#include "stl_util.h"
#include "render\global.h"
#include "surface.h"
#include "render\command_pool.h"


namespace render
{


	class RenderSystem
	{
	public:
		RenderSystem(platform::Window window, const std::string& app_name);
		
		void Render();
	private:
		
		bool InitPhysicalDevices();

		void InitPhysicalDeviceProperties(VkPhysicalDevice physical_device);
		void InitPhysicalDeviceMemoryProperties(VkPhysicalDevice physical_device);
		void InitPhysicalDeviceQueueFamiliesProperties(VkPhysicalDevice physical_device);
		void InitPhysicalDeviceFeatures(const VkPhysicalDevice& physical_device);
		bool InitPhysicalDeviceLayers(const VkPhysicalDevice& physical_device);
		bool InitPhysicalDeviceExtensions(const VkPhysicalDevice& physical_device);


		uint32_t DetermineDeviceForUse();

		bool InitLogicalDevice(const VkPhysicalDevice& physical_device, const std::vector<uint32_t> queue_famaly_indices);

		void FillGlobal();

		uint32_t FindDeviceQueueFamalyWithFlag(const VkPhysicalDevice& physical_deivce, VkQueueFlags enabled_flags, VkQueueFlags disabled_flags = 0);

		const std::vector<const char*>& GetRequiredDeviceExtensions();

		Global global_;
		RenderApiInstance api_instance_;
		Surface surface_;

		uint32_t selected_device_index_;
		uint32_t selected_graphics_queue_index_;
		uint32_t selected_transfer_queue_index_;


		std::vector<VkPhysicalDevice> vk_physical_devices_;
		std::vector<util::DeleterWrapper<VkDevice>> vk_logical_devices_;

		std::map<VkPhysicalDevice, VkPhysicalDeviceMemoryProperties> vk_physical_devices_memory_propeties_;
		std::map<VkPhysicalDevice, VkPhysicalDeviceProperties> vk_physical_devices_propeties_;
		std::map<VkPhysicalDevice, std::vector<VkQueueFamilyProperties>> vk_physical_devices_to_queues_;
		std::map<VkPhysicalDevice, VkPhysicalDeviceFeatures> vk_physical_devices_features_;

		std::map<VkPhysicalDevice, std::vector<VkLayerProperties>> vk_physical_devices_layers_;

		std::map<VkPhysicalDevice, std::vector<VkExtensionProperties>> vk_physical_devices_extensions_;

		std::unique_ptr<CommandPool> graphics_command_pool_ptr_;
		std::unique_ptr<CommandPool> transfer_command_pool_ptr_;

		DescriptorSetsManager descriptor_set_manager_;

	};

}
#endif  // RENDER_ENGINE_RENDER_RENDER_SYSTEM_H_