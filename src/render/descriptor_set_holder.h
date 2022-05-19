#ifndef RENDER_ENGINE_RENDER_DESCRIPTOR_SET_HOLDER_H_
#define RENDER_ENGINE_RENDER_DESCRIPTOR_SET_HOLDER_H_

#include <map>
#include <variant>

#include "vulkan/vulkan.h"

#include "render/buffer.h"
#include "render/image_view.h"
#include "render/object_base.h"
#include "render/descriptor_set.h"
#include "render/descriptor_sets_manager.h"

namespace render
{
	template<typename DataType, DescriptorBindingType BindingType>
	class DescriptorSetBindingData;

	template<typename DataType>
	class DescriptorSetBindingData<DataType, DescriptorBindingType::kUniform>
	{
		UniformBuffer uniform_buffer_;

		VkDescriptorBufferInfo vk_buffer_info_;
	public:

		DescriptorSetBindingData(const DeviceConfiguration& device_cfg) : uniform_buffer_(device_cfg, sizeof(DataType)) {}

		void Update(const DataType& new_data)
		{
			uint32_t size = sizeof(DataType);

			void* mapped_data;
			vkMapMemory(uniform_buffer_.GetDeviceCfg().logical_device, uniform_buffer_.GetBufferMemory(), 0, size, 0, &mapped_data);
			memcpy(mapped_data, &new_data, size);
			vkUnmapMemory(uniform_buffer_.GetDeviceCfg().logical_device, uniform_buffer_.GetBufferMemory());
		}

		void FillWriteDescriptorSet(VkWriteDescriptorSet& write_desc_set)
		{
			vk_buffer_info_.buffer = uniform_buffer_.GetHandle();
			vk_buffer_info_.offset = 0;
			vk_buffer_info_.range = sizeof(DataType);

			write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_desc_set.dstArrayElement = 0;
			write_desc_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write_desc_set.descriptorCount = 1;
			write_desc_set.pBufferInfo = &vk_buffer_info_;
		}
	};

	template<typename DataType>
	class DescriptorSetBindingData<DataType, DescriptorBindingType::kSampler>
	{
		ImageView image_view_;

		VkDescriptorImageInfo vk_image_info_;

		const Sampler* sampler_;

	public:

		DescriptorSetBindingData(const DeviceConfiguration& device_cfg) : image_view_(device_cfg) {}

			void Update(const DataType& new_data)
		{
			image_view_.Assign(*new_data.image);
			sampler_ = new_data.sampler;
		}

		void FillWriteDescriptorSet(VkWriteDescriptorSet& write_desc_set)
		{
			vk_image_info_.imageLayout = image_view_.GetImageType() == Image::ImageType::kDepthImage ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			vk_image_info_.imageView = image_view_.GetHandle();
			vk_image_info_.sampler = sampler_->GetHandle();

			write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_desc_set.dstArrayElement = 0;
			write_desc_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_desc_set.descriptorCount = 1;
			write_desc_set.pImageInfo = &vk_image_info_;
		}
	};

	template<DescriptorSetType Type, int BindingIndex>
	class DescriptorSetBinding : public DescriptorSetBinding<Type, BindingIndex-1>
	{
		using DescriptorSetDesc = typename DescriptorSet<Type>;
		using BindingDesc = typename DescriptorSetDesc::template Binding<BindingIndex>;
		using BindingDataType = typename BindingDesc::Data;

		DescriptorSetBindingData<BindingDataType, BindingDesc::type> data_;

	public:
		
		DescriptorSetBinding(const DeviceConfiguration& device_cfg) : DescriptorSetBinding<Type, BindingIndex - 1>(device_cfg), data_(device_cfg) {}

		virtual void FillData(BindingDataType& data) = 0;

		void UpdateData()
		{
			BindingDesc::Data new_data;
			FillData(new_data);

			data_.Update(new_data);

			DescriptorSetBinding<Type, BindingIndex - 1>::UpdateData();
		}

		void FillWriteDescriptorSets(VkDescriptorSet descriptor_set, std::vector<VkWriteDescriptorSet>& write_descriptor_sets)
		{
			write_descriptor_sets[BindingIndex].dstSet = descriptor_set;
			write_descriptor_sets[BindingIndex].dstBinding = BindingIndex;

			data_.FillWriteDescriptorSet(write_descriptor_sets[BindingIndex]);

			DescriptorSetBinding<Type, BindingIndex - 1>::FillWriteDescriptorSets(descriptor_set, write_descriptor_sets);
		}
	};

	template<DescriptorSetType Type>
	class DescriptorSetBinding<Type, -1>: public RenderObjBase<void*>
	{
	public:

		DescriptorSetBinding(const DeviceConfiguration& device_cfg) : RenderObjBase(device_cfg) {}

		void UpdateData() {}

		void FillWriteDescriptorSets(VkDescriptorSet descriptor_set, std::vector<VkWriteDescriptorSet>& write_descriptor_sets) {}
	};

	template<DescriptorSetType Type>
	struct DescriptorSetBindingsCollection : public DescriptorSetBinding<Type, DescriptorSet<Type>::bindings_count - 1>
	{
	public:

		DescriptorSetBindingsCollection(const DeviceConfiguration& device_cfg) : DescriptorSetBinding<Type, DescriptorSet<Type>::bindings_count - 1>(device_cfg) {}

		VkDescriptorSet AttachDescriptorSets(DescriptorSetsManager& manager)
		{
			VkDescriptorSet desc_set = manager.GetFreeDescriptor(Type);
			std::vector<VkWriteDescriptorSet> writes(DescriptorSet<Type>::bindings_count);

			DescriptorSetBinding<Type, DescriptorSet<Type>::bindings_count - 1>::FillWriteDescriptorSets(desc_set, writes);

			vkUpdateDescriptorSets(device_cfg_.logical_device, writes.size(), writes.data(), 0, nullptr);

			return desc_set;
		}

		void UpdateData()
		{
			DescriptorSetBinding<Type, DescriptorSet<Type>::bindings_count - 1>::UpdateData();
		}
	};




	//--------------------------------------------------------------------------------------------------------------------------




	template<DescriptorSetType T1, DescriptorSetType... Ts>
	class DescriptorSetHolderInternal : public DescriptorSetHolderInternal<Ts...>, public DescriptorSetBindingsCollection<T1>
	{
	public:

		DescriptorSetHolderInternal(const DeviceConfiguration& device_cfg) : DescriptorSetHolderInternal<Ts...>(device_cfg), DescriptorSetBindingsCollection<T1>(device_cfg)
		{}

	protected:

		void AttachDescriptorSets(DescriptorSetsManager& manager)
		{
			VkDescriptorSet desc_set = DescriptorSetBindingsCollection<T1>::AttachDescriptorSets(manager);

			descriptor_sets_[T1] = desc_set;
			
			DescriptorSetHolderInternal<Ts...>::AttachDescriptorSets(manager);
		}

		void UpdateData()
		{
			DescriptorSetBindingsCollection<T1>::UpdateData();
			DescriptorSetHolderInternal<Ts...>::UpdateData();
		}
	};

	template<>
	class DescriptorSetHolderInternal<DescriptorSetType::ListEnd>
	{
	public:

		const std::map<DescriptorSetType, VkDescriptorSet>& GetDescriptorSets() const
		{
			return descriptor_sets_;
		}

		DescriptorSetHolderInternal(const DeviceConfiguration& device_cfg) {}

	protected:

		void AttachDescriptorSets(DescriptorSetsManager& manager) {}

		void UpdateData() {}

		std::map<DescriptorSetType, VkDescriptorSet> descriptor_sets_;
	};

	template<DescriptorSetType... Ts>
	class DescriptorSetHolder : public DescriptorSetHolderInternal<Ts..., DescriptorSetType::ListEnd>
	{
	public:

		DescriptorSetHolder(const DeviceConfiguration& device_cfg) : DescriptorSetHolderInternal(device_cfg) {}
	};



}
#endif  // RENDER_ENGINE_RENDER_DESCRIPTOR_SET_HOLDER_H_