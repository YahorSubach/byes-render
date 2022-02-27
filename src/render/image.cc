#include "image.h"

#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

render::Image::Image(const DeviceConfiguration& device_cfg, VkFormat format, const uint32_t& width, const uint32_t& height, ImageType image_type) : RenderObjBase(device_cfg), image_type_(image_type)
{
	format_ = format;
	VkImageCreateInfo image_info{};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.extent.width = static_cast<uint32_t>(width);
	image_info.extent.height = static_cast<uint32_t>(height);
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.format = format;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	if(image_type == ImageType::kColorImage)		image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	else if (image_type == ImageType::kDepthImage)	image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	image_info.sharingMode = VK_SHARING_MODE_CONCURRENT;

	const std::vector<uint32_t> sharing_queues_indices = { device_cfg.graphics_queue_index, device_cfg.transfer_queue_index };

	image_info.queueFamilyIndexCount = static_cast<uint32_t>(sharing_queues_indices.size());
	image_info.pQueueFamilyIndices = sharing_queues_indices.data();

	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.flags = 0; // Optional

	if (vkCreateImage(device_cfg_.logical_device, &image_info, nullptr, &handle_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements mem_requirements;
	vkGetImageMemoryRequirements(device_cfg_.logical_device, handle_, &mem_requirements);

	memory_ = std::make_unique<Memory>(device_cfg_, mem_requirements.size, mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vkBindImageMemory(device_cfg_.logical_device, handle_, memory_->GetMemoryHandle(), 0);
}

render::Image::Image(const DeviceConfiguration& device_cfg, VkFormat format, VkImage image_handle) : RenderObjBase(device_cfg), image_type_(ImageType::kSwapchainImage)
{
	handle_ = image_handle;
	format_ = format;
}

render::Image::Image(const DeviceConfiguration& device_cfg, VkFormat format, const uint32_t& width, const uint32_t& height, const void* pixels): Image(device_cfg, format, width, height, ImageType::kColorImage)
{
	VkDeviceSize image_size = width * height * 4;
	Buffer staging_buffer(device_cfg, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {});

	staging_buffer.LoadData(pixels, image_size);
	TransitionImageLayout(*device_cfg.graphics_cmd_pool, TransitionType::kPrepareToTransfer);
	CopyBuffer(*device_cfg.transfer_cmd_pool, staging_buffer, width, height);
	TransitionImageLayout(*device_cfg.graphics_cmd_pool, TransitionType::kTransferToFragment);
}

render::Image render::Image::FromFile(const DeviceConfiguration& device_cfg, const std::string& path)
{
	int width = 0;
	int height = 0;
	int channels;
	
	stbi_uc* pixels = stbi_load(path.data(), &width, &height, &channels, STBI_rgb_alpha);

	Image res = Image(device_cfg, VK_FORMAT_R8G8B8A8_SRGB, width, height, pixels);

	stbi_image_free(pixels);


	return res;
}

VkFormat render::Image::GetFromat() const
{
	return format_;
}

void render::Image::TransitionImageLayout(const CommandPool& command_pool, TransitionType transfer_type)
{

	VkPipelineStageFlags source_stage;
	VkPipelineStageFlags destination_stage;
	VkImageMemoryBarrier barrier{};

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = handle_;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0; // TODO
	barrier.dstAccessMask = 0; // TODO


	if (transfer_type == TransitionType::kPrepareToTransfer)
	{
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (transfer_type == TransitionType::kTransferToFragment)
	{
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	command_pool.ExecuteOneTimeCommand([barrier, source_stage, destination_stage, image = handle_](VkCommandBuffer buffer)
		{
			vkCmdPipelineBarrier(
				buffer,
				source_stage, destination_stage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		});
}

void render::Image::CopyBuffer(const CommandPool& command_pool, const Buffer& buffer, uint32_t width, uint32_t height)
{
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	command_pool.ExecuteOneTimeCommand([region, &buffer, image = handle_](VkCommandBuffer command_buffer)
		{
			vkCmdCopyBufferToImage(
				command_buffer,
				buffer.GetHandle(),
				image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&region
			);
		});
}

render::Image::~Image()
{
	if (handle_ != VK_NULL_HANDLE && image_type_ != ImageType::kSwapchainImage)
	{
		vkDestroyImage(device_cfg_.logical_device, handle_, nullptr);
	}
}

render::Image::ImageType render::Image::GetImageType() const
{
	return image_type_;
}
