#ifndef RENDER_ENGINE_RENDER_VK_UTIL_H_
#define RENDER_ENGINE_RENDER_VK_UTIL_H_

#include "common.h"

#include "vulkan/vulkan.h"

namespace render
{
	struct vk_util
	{
		static VkSemaphore CreateSemaphore(VkDevice device)
		{
			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkSemaphore semaphore;

			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore) == VK_SUCCESS)
				return semaphore;

			LOG(err, "semaphore creation error");

			return VK_NULL_HANDLE;
		}

		static VkFence CreateFence(VkDevice device)
		{
			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			VkFence fence;

			if (vkCreateFence(device, &fenceInfo, nullptr, &fence) == VK_SUCCESS)
				return fence;

			LOG(err, "fence creation error");

			return VK_NULL_HANDLE;
		}
	};
}


#endif  // RENDER_ENGINE_RENDER_VK_UTIL_H_