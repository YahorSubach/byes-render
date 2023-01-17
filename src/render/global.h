#ifndef RENDER_ENGINE_RENDER_GLOBAL_H_
#define RENDER_ENGINE_RENDER_GLOBAL_H_

#include "render/command_pool.h"
#include "render/sampler.h"


namespace render
{

	struct Global
	{
		VkPhysicalDevice physical_device;
		VkPhysicalDeviceProperties physical_device_properties;

		VkDevice  logical_device;

		VkQueue transfer_queue;
		uint32_t transfer_queue_index;

		VkQueue graphics_queue;
		uint32_t graphics_queue_index;

		CommandPool* graphics_cmd_pool;
		CommandPool* transfer_cmd_pool;

		std::vector<Sampler> mipmap_cnt_to_global_samplers;
		std::optional<Sampler> shadowmap_sampler;

		util::NullableRef<Image> error_image;

		Format presentation_format;
		Format depth_map_format = VK_FORMAT_D32_SFLOAT;
		Format high_range_color_format = VK_FORMAT_R32G32B32A32_SFLOAT;
		Format color_format = VK_FORMAT_R8G8B8A8_SRGB;
	};
}
#endif  // RENDER_ENGINE_RENDER_GLOBAL_H_