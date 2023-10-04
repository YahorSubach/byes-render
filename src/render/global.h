#ifndef RENDER_ENGINE_RENDER_GLOBAL_H_
#define RENDER_ENGINE_RENDER_GLOBAL_H_

#include <vector>
#include <variant>


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
		std::optional<Sampler> nearest_sampler;
		std::optional<Sampler> shadowmap_sampler;

		std::optional<Image> error_image;
		std::optional<Image> default_normal;

		Format depth_map_format = VK_FORMAT_D32_SFLOAT;
		Format color_format = VK_FORMAT_R8G8B8A8_SRGB;

		uint32_t frame_ind;
		mutable std::vector<std::pair<uint32_t, std::variant<VkBuffer, OffsettedMemory, VkImageView, VkDescriptorSet>>> delete_list;
	};
}
#endif  // RENDER_ENGINE_RENDER_GLOBAL_H_