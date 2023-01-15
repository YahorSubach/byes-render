#ifndef RENDER_ENGINE_RENDER_DESCRIPTOR_SET_HOLDER_H_
#define RENDER_ENGINE_RENDER_DESCRIPTOR_SET_HOLDER_H_

#include <map>
#include <variant>
#include <span>

#include "vulkan/vulkan.h"

#include "common.h"

#include "render/buffer.h"
#include "render/image_view.h"
#include "render/object_base.h"
#include "render/descriptor_set.h"
#include "render/descriptor_sets_manager.h"

namespace render
{
	namespace descriptor_sets_holder
	{


		template<typename DataType, DescriptorBindingType BindingType>
		class BindingData;

		template<typename DataType>
		class BindingData<DataType, DescriptorBindingType::kUniform>
		{
			UniformBuffer uniform_buffer_;

			VkDescriptorBufferInfo vk_buffer_info_;
		public:

			BindingData(const Global& global) : uniform_buffer_(global, sizeof(DataType)) {}

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
		class BindingData<DataType, DescriptorBindingType::kSampler>
		{
			ImageView image_view_;

			VkDescriptorImageInfo vk_image_info_;

			util::NullableRef<const Sampler> sampler_;

		public:

			BindingData(const Global& global) : image_view_(global) {}

			void Update(const DataType& new_data)
			{
				image_view_.Assign(*(new_data.GetImage()));
				sampler_ = new_data.GetSampler();
				//image_view_.AddUsageFlag(VK_IMAGE_USAGE_SAMPLED_BIT);
			}

			void FillWriteDescriptorSet(VkWriteDescriptorSet& write_desc_set)
			{
				vk_image_info_.imageLayout = image_view_.GetFormat() == image_view_.GetDeviceCfg().depth_map_format ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				vk_image_info_.imageView = image_view_.GetHandle();
				vk_image_info_.sampler = sampler_->GetHandle();

				write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write_desc_set.dstArrayElement = 0;
				write_desc_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write_desc_set.descriptorCount = 1;
				write_desc_set.pImageInfo = &vk_image_info_;
			}
		};

		template<typename DescribedObjectType, DescriptorSetType Type, int BindingIndex>
		class Binding : public Binding<DescribedObjectType, Type, BindingIndex - 1>
		{
			using DescriptorSetDesc = typename DescriptorSet<Type>;
			using BindingDesc = typename DescriptorSetDesc::template Binding<BindingIndex>;
			using BindingDataType = typename BindingDesc::Data;

			BindingData<BindingDataType, BindingDesc::type> data_;

		public:

			Binding(const Global& global) : Binding<DescribedObjectType, Type, BindingIndex - 1>(global), data_(global) {}

			virtual void FillData(const DescribedObjectType& object, BindingDataType& data) = 0;

			void UpdateDataInternal(const DescribedObjectType& object)
			{
				typename BindingDesc::Data new_data;
				FillData(object, new_data);

				data_.Update(new_data);

				Binding<DescribedObjectType, Type, BindingIndex - 1>::UpdateDataInternal(object);
			}

			void FillDescriptorSetWrites(VkDescriptorSet descriptor_set, std::span<VkWriteDescriptorSet>& descriptor_set_writes)
			{
				descriptor_set_writes[BindingIndex].dstSet = descriptor_set;
				descriptor_set_writes[BindingIndex].dstBinding = BindingIndex;

				data_.FillWriteDescriptorSet(descriptor_set_writes[BindingIndex]);

				Binding<DescribedObjectType, Type, BindingIndex - 1>::FillDescriptorSetWrites(descriptor_set, descriptor_set_writes);
			}
		};

		template<typename DescribedObjectType, DescriptorSetType Type>
		class Binding<DescribedObjectType, Type, -1>
		{
		public:

			Binding(const Global& global) {}

			void UpdateDataInternal(const DescribedObjectType& object) {}

			void FillDescriptorSetWrites(VkDescriptorSet descriptor_set, std::span<VkWriteDescriptorSet>& write_descriptor_sets) {}
		};

		template<typename DescribedObjectType, DescriptorSetType Type>
		struct Bindings : public Binding<DescribedObjectType, Type, DescriptorSet<Type>::binding_count - 1>
		{
		public:

			Bindings(const Global& global) : Binding<DescribedObjectType, Type, DescriptorSet<Type>::binding_count - 1>(global) {}

			void FillDescriptorSetWrites(VkDescriptorSet descriptor_set, std::span<VkWriteDescriptorSet> write_descriptor_sets)
			{
				Binding<DescribedObjectType, Type, DescriptorSet<Type>::binding_count - 1>::FillDescriptorSetWrites(descriptor_set, write_descriptor_sets);
			}

			void UpdateDataInternal(const DescribedObjectType& object)
			{
				Binding<DescribedObjectType, Type, DescriptorSet<Type>::binding_count - 1>::UpdateDataInternal(object);
			}
		};




		//--------------------------------------------------------------------------------------------------------------------------



		template<typename DescribedObjectType, DescriptorSetType T1, DescriptorSetType... Ts>
		class HolderInternal : public HolderInternal<DescribedObjectType, Ts...>, public Bindings<DescribedObjectType, T1>
		{
		public:

			HolderInternal(const Global& global) : HolderInternal<DescribedObjectType, Ts...>(global), Bindings<DescribedObjectType, T1>(global)
			{}

		protected:

			int FillDescriptorSetWrites(DescriptorSetsManager& manager, std::span<VkWriteDescriptorSet> write_descriptor_sets)
			{
				int writes_filled_cnt = 0;

				for (int frame_ind = 0; frame_ind < kFramesCount; frame_ind++)
				{
					VkDescriptorSet vk_descriptor_set = manager.GetFreeDescriptor(T1);
					HolderInternal<DescribedObjectType, Ts...>::descriptor_sets_per_frame_[frame_ind].emplace(T1, vk_descriptor_set);

					std::span<VkWriteDescriptorSet> writes_to_fill(write_descriptor_sets.data() + writes_filled_cnt, DescriptorSet<T1>::binding_count);

					Bindings<DescribedObjectType, T1>::FillDescriptorSetWrites(vk_descriptor_set, writes_to_fill);
					writes_filled_cnt += DescriptorSet<T1>::binding_count;
				}

				std::span<VkWriteDescriptorSet> writes_to_fill(write_descriptor_sets.data() + writes_filled_cnt, write_descriptor_sets.size() - writes_filled_cnt);
				writes_filled_cnt += HolderInternal<DescribedObjectType, Ts...>::FillDescriptorSetWrites(manager, writes_to_fill);

				return writes_filled_cnt;
			}

			void UpdateDataInternal(const DescribedObjectType& object)
			{
				Bindings<DescribedObjectType, T1>::UpdateDataInternal(object);
				HolderInternal<DescribedObjectType, Ts...>::UpdateDataInternal(object);
			}
		};

		template<typename DescribedObjectType>
		class HolderInternal<DescribedObjectType, DescriptorSetType::ListEnd> : public RenderObjBase<void*>
		{
		public:

			HolderInternal(const Global& global) : RenderObjBase(global) {}

			const std::map<DescriptorSetType, VkDescriptorSet>& GetDescriptorSets(uint32_t frame_index) const
			{
				return descriptor_sets_per_frame_[frame_index];
			}

		protected:

			int FillDescriptorSetWrites(DescriptorSetsManager& manager, std::span<VkWriteDescriptorSet> write_descriptor_sets) { return 0; }
			void UpdateDataInternal(const DescribedObjectType& object) {}

			std::array<std::map<DescriptorSetType, VkDescriptorSet>, kFramesCount> descriptor_sets_per_frame_;
		};

		template<typename DescribedObjectType, DescriptorSetType... Ts>
		class Holder : public HolderInternal<DescribedObjectType, Ts..., DescriptorSetType::ListEnd>
		{
		public:

			Holder(const Global& global, DescriptorSetsManager& manager) : HolderInternal<DescribedObjectType, Ts..., DescriptorSetType::ListEnd>(global)
			{
			}

			void UpdateData(const DescribedObjectType& object)
			{
				HolderInternal<DescribedObjectType, Ts..., DescriptorSetType::ListEnd>::UpdateDataInternal(object);
			}

		protected:

			void AttachDescriptorSets(DescriptorSetsManager& manager)
			{
				//TODO: calc preciesly
				std::array<VkWriteDescriptorSet, 64> writes = {};
				int writes_filled_cnt = HolderInternal<DescribedObjectType, Ts..., DescriptorSetType::ListEnd>::FillDescriptorSetWrites(manager, writes);
				vkUpdateDescriptorSets(RenderObjBase<void*>::global_.logical_device, writes_filled_cnt, writes.data(), 0, nullptr);
			}
		};
	}
}
#endif  // RENDER_ENGINE_RENDER_DESCRIPTOR_SET_HOLDER_H_