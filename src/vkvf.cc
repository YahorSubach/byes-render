#include "vkvf.h"

#include "vulkan/vulkan.h"

namespace vkvf
{
	class VKVisualFacade::VKVisualFacadeImpl
	{
	public:

		VKVisualFacadeImpl(const VKVisualFacadeImpl&) = delete;
		VKVisualFacadeImpl& operator=(const VKVisualFacadeImpl&) = delete;

		VKVisualFacadeImpl()
		{
			vk_init_success_ = false;

			auto application_info = std::make_unique<VkApplicationInfo>();
			application_info->apiVersion = VK_HEADER_VERSION;
			application_info->applicationVersion = 1;
			application_info->engineVersion = 1;
			application_info->pApplicationName = "vulkan_concepts";
			application_info->pEngineName = "vulkan_concepts_engine";
			application_info->sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

			auto create_info = std::make_unique<VkInstanceCreateInfo>();
			create_info->enabledExtensionCount = 0;
			create_info->enabledLayerCount = 0;
			create_info->pApplicationInfo = application_info.get();
			create_info->pNext = nullptr;
			create_info->sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

			vk_instance_ = std::make_unique<VkInstance>();

			VkResult result = vkCreateInstance(create_info.get(), nullptr, vk_instance_.get());
			if (result == VK_SUCCESS)
				vk_init_success_ = true;

		}

		bool VKInitSuccess()
		{
			return vk_init_success_;
		}

	private:
		bool vk_init_success_;
		std::unique_ptr<VkApplicationInfo> application_info_;
		std::unique_ptr<VkInstance> vk_instance_;

	};


	VKVisualFacade::VKVisualFacade()
	{
		impl_ = std::make_unique<VKVisualFacadeImpl>();
	}

	VKVisualFacade::~VKVisualFacade() = default;

	bool VKVisualFacade::VKInitSuccess()
	{
		return impl_->VKInitSuccess();
	}
}