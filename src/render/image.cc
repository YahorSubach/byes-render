#include "image.h"

#include <memory>

#include "global.h"

#pragma warning(push, 0)
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#pragma warning(pop)

render::Image::Image(const Global& global, VkFormat format, Extent extent, uint32_t layer_cnt) : LazyRenderObj(global), format_(format), extent_(extent), holds_external_handle_(false), usage_(0), layer_cnt_(layer_cnt), mipmap_levels_count_(1)
{
	//mipmap_levels_count_ = image_properties_.Check(ImageProperty::kMipMap) ? static_cast<uint32_t>(std::floor(std::log2(std::max(width, height))) + 1) : 1;

	
}

render::Image::Image(const Global& global, VkFormat format, VkImage image_handle) : LazyRenderObj(global), format_(format), mipmap_levels_count_(1), holds_external_handle_(true), usage_(0), extent_(0,0)
{
	handle_ = image_handle;
}

render::Image::Image(const Global& global, BuiltinImageType type) : Image(global, global.color_format, {1, 1})
{
	VkDeviceSize image_size = 4;

	const unsigned char black[4] = { 0, 0, 0, 255 };
	const unsigned char white[4] = { 255, 255, 255, 255 };
	const unsigned char normal[4] = { 128, 128, 255, 255 };


	auto&& error_map = { 
		"       ",
		"  ***  ",
		"  *    ",
		"  ***  ",
		"  *    ",
		"  ***  ",
		"       ",
	};

	unsigned char error[4 * 7 * 7];

	int index = 0;
	for (auto&& line : error_map)
	{
		for (auto&& c : std::string_view(line))
		{
			error[index++] = c == ' ' ? 0 : 255;
			error[index++] = 0;
			error[index++] = 0;
			error[index++] = 255;
		}
	}



	const unsigned char* data = nullptr;

	switch (type)
	{
	case render::Image::BuiltinImageType::kBlack:
		data = black;
		break;
	case render::Image::BuiltinImageType::kWhite:
		data = white;
		break;
	case render::Image::BuiltinImageType::kNormal:
		data = normal;
		break;
	case render::Image::BuiltinImageType::kError:
		image_size = 4 * 7 * 7;
		extent_ = { 7, 7 };
		data = error;
		break;
	default:
		break;
	}

	pixels_data_ = std::make_unique<std::vector<unsigned char>>(image_size);
	pixels_data_->reserve(image_size);
	pixels_data_->assign(data, data + image_size);

	VkFormatProperties format_properties;
	vkGetPhysicalDeviceFormatProperties(global_.physical_device, format_, &format_properties);

	if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT) && (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT))
	{
		mipmap_levels_count_ = static_cast<uint32_t>(std::floor(std::log2(std::max(extent_.height, extent_.width))) + 1);
	}
}

render::Image::Image(const Global& global, VkFormat format, Extent extent, const unsigned char* pixels): Image(global, format, extent)
{
	VkDeviceSize image_size = extent.width * extent.height * (format != VK_FORMAT_R8_SRGB ? 4 : 1);
	pixels_data_ = std::make_unique<std::vector<unsigned char>>(image_size);
	pixels_data_->reserve(image_size);
	pixels_data_->assign(pixels, pixels + image_size);

	VkFormatProperties format_properties;
	vkGetPhysicalDeviceFormatProperties(global_.physical_device, format, &format_properties);

	if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT) && (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT))
	{
		mipmap_levels_count_ = static_cast<uint32_t>(std::floor(std::log2(std::max(extent_.height, extent_.width))) + 1);
	}
}

render::Image render::Image::FromFile(const Global& global, const std::string_view& path)
{
	int width = 0;
	int height = 0;
	int channels;
	
	//properties.Set(ImageProperty::kLoad);

	stbi_uc* pixels = stbi_load(path.data(), &width, &height, &channels, STBI_rgb_alpha);

	assert(width > 0 && height > 0);

	Image res = Image(global, VK_FORMAT_R8G8B8A8_SRGB, {u32(width), u32(height)}, pixels);

	stbi_image_free(pixels);

	return res;
}

VkFormat render::Image::GetFormat() const
{
	return format_;
}

uint32_t render::Image::GetLayerCount() const
{
	return layer_cnt_;
}

void render::Image::TransitionImageLayout(const CommandPool& command_pool, TransitionType transfer_type) const
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
	barrier.subresourceRange.levelCount = mipmap_levels_count_;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layer_cnt_;
	barrier.srcAccessMask = 0; // TODO
	barrier.dstAccessMask = 0; // TODO


	if (transfer_type == TransitionType::kTransferDst)
	{
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (transfer_type == TransitionType::kTransferSrc)
	{
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (transfer_type == TransitionType::kFragmentRead)
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

void render::Image::CopyBuffer(const CommandPool& command_pool, const Buffer& buffer) const
{
	assert(handle_ != VK_NULL_HANDLE);


	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = layer_cnt_;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		extent_.width,
		extent_.height,
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
	if (handle_ != VK_NULL_HANDLE && !holds_external_handle_)
	{
		vkDestroyImage(global_.logical_device, handle_, nullptr);
	}
}

//const render::ImagePropertiesStorage& render::Image::GetImageProperties() const
//{
//	return image_properties_;
//}


uint32_t render::Image::GetMipMapLevelsCount() const
{
	return mipmap_levels_count_;
}

uint32_t render::Image::AddUsageFlag(uint32_t flag) const
{
	assert(handle_ == VK_NULL_HANDLE);
	usage_ = usage_ | flag;
	return usage_;
}

uint32_t render::Image::CheckUsageFlag(uint32_t flag) const
{
	return usage_ & flag;
}

bool render::Image::InitHandle() const
{
	VkImageCreateInfo image_info{};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.extent.width = static_cast<uint32_t>(extent_.width);
	image_info.extent.height = static_cast<uint32_t>(extent_.height);
	image_info.extent.depth = 1;
	image_info.mipLevels = mipmap_levels_count_;
	image_info.arrayLayers = layer_cnt_;
	image_info.format = format_;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (pixels_data_)
	{
		AddUsageFlag(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	}

	if (mipmap_levels_count_ > 1)
	{
		AddUsageFlag(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
	}

	image_info.usage = usage_;
	
	//if (image_properties_.Check(ImageProperty::kShaderInput)) image_info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	//if (image_properties_.Check(ImageProperty::kColorAttachment)) image_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	//if (image_properties_.Check(ImageProperty::kDepthAttachment)) image_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	//if (image_properties_.Check(ImageProperty::kRead) || image_properties_.Check(ImageProperty::kMipMap)) image_info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	//if (image_properties_.Check(ImageProperty::kWrite) || image_properties_.Check(ImageProperty::kMipMap)) image_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	std::vector<uint32_t> sharing_queues_indices;
	sharing_queues_indices.push_back(global_.graphics_queue_index);
	if (global_.graphics_queue_index != global_.transfer_queue_index)
	{
		sharing_queues_indices.push_back(global_.transfer_queue_index);
		image_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
	}

	image_info.queueFamilyIndexCount = static_cast<uint32_t>(sharing_queues_indices.size());
	image_info.pQueueFamilyIndices = sharing_queues_indices.data();

	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.flags = layer_cnt_ == 6 ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0; // Optional

	if (vkCreateImage(global_.logical_device, &image_info, nullptr, &handle_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements mem_requirements;
	vkGetImageMemoryRequirements(global_.logical_device, handle_, &mem_requirements);

	memory_ = std::make_unique<Memory>(global_, (uint32_t)mem_requirements.size, (uint32_t)mem_requirements.alignment, mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkBindImageMemory(global_.logical_device, handle_, memory_->GetMemoryHandle(), memory_->GetMemoryOffset());

	if (pixels_data_)
	{
		StagingBuffer staging_buffer(global_, pixels_data_->size());
		staging_buffer.LoadData(pixels_data_->data(), pixels_data_->size());
		pixels_data_.reset();

		TransitionImageLayout(*global_.graphics_cmd_pool, TransitionType::kTransferDst);
		CopyBuffer(*global_.transfer_cmd_pool, staging_buffer);

		GenerateMipMaps();
	}
	


	return true;
}

void render::Image::GenerateMipMaps() const
{

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = handle_;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layer_cnt_;
	barrier.subresourceRange.levelCount = 1;


		global_.graphics_cmd_pool->ExecuteOneTimeCommand([this, &barrier/*barrier, source_stage, destination_stage, image = handle_*/](VkCommandBuffer buffer)
			{
			int32_t mip_width = extent_.width;
			int32_t mip_height = extent_.height;

			for (uint32_t i = 1; i < mipmap_levels_count_; i++)
			{
				barrier.subresourceRange.baseMipLevel = i - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				vkCmdPipelineBarrier(buffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
					0, nullptr,
					0, nullptr,
					1, &barrier);

				VkImageBlit blit{};
				blit.srcOffsets[0] = { 0, 0, 0 };
				blit.srcOffsets[1] = { mip_width, mip_height, 1 };
				blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.srcSubresource.mipLevel = i - 1;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = 1;
				blit.dstOffsets[0] = { 0, 0, 0 };
				blit.dstOffsets[1] = { mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 };
				blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = layer_cnt_;

				vkCmdBlitImage(buffer,
					handle_, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					handle_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &blit,
					VK_FILTER_LINEAR);

				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(buffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
					0, nullptr,
					0, nullptr,
					1, &barrier);

				mip_width /= 2;
				mip_height /= 2;
			}

			barrier.subresourceRange.baseMipLevel = mipmap_levels_count_ - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(buffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			});

}
