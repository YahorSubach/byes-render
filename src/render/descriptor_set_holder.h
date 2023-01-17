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
			std::array<UniformBuffer, kFramesCount> uniform_buffers_;
			std::array<bool, kFramesCount> attached_per_frame_;

			VkDescriptorBufferInfo vk_buffer_info_;
		public:

			BindingData(const Global& global) :
				uniform_buffers_
			{
				UniformBuffer(global, sizeof(DataType)),
				UniformBuffer(global, sizeof(DataType)),
				UniformBuffer(global, sizeof(DataType)),
				UniformBuffer(global, sizeof(DataType)),
			},
			attached_per_frame_
			{
				false, false, false, false
			}
			{}

			virtual void FillData(DataType& data) = 0;

			bool UpdateAndTryFillWrite(int frame_index, VkWriteDescriptorSet& write_desc_set)
			{
				uint32_t size = sizeof(DataType);

				DataType new_data;
				FillData(new_data);

				void* mapped_data;
				vkMapMemory(uniform_buffers_[frame_index].GetDeviceCfg().logical_device, uniform_buffers_[frame_index].GetBufferMemory(), 0, size, 0, &mapped_data);
				memcpy(mapped_data, &new_data, size);
				vkUnmapMemory(uniform_buffers_[frame_index].GetDeviceCfg().logical_device, uniform_buffers_[frame_index].GetBufferMemory());

				if (!attached_per_frame_[frame_index])
				{
					attached_per_frame_[frame_index] = true;
					FillWriteDescriptorSet(frame_index, write_desc_set);
					return true;
				}
				return false;
			}

			void FillWriteDescriptorSet(int frame_index, VkWriteDescriptorSet& write_desc_set)
			{

				vk_buffer_info_.buffer = uniform_buffers_[frame_index].GetHandle();
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
			const Global& gloabl_;

			std::array<VkImage, kFramesCount> images_per_frame_;

			std::optional<ImageView> image_view_;

			VkDescriptorImageInfo vk_image_info_;

			util::NullableRef<const Sampler> sampler_;

		public:

			BindingData(const Global& global) : gloabl_(global),
				images_per_frame_
			{
				VK_NULL_HANDLE,VK_NULL_HANDLE,VK_NULL_HANDLE,VK_NULL_HANDLE
			}
			{}

			void FillWriteDescriptorSet(int frame_index, VkWriteDescriptorSet& write_desc_set)
			{
				vk_image_info_.imageLayout = image_view_->GetFormat() == image_view_->GetDeviceCfg().depth_map_format ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				vk_image_info_.imageView = image_view_->GetHandle();
				vk_image_info_.sampler = sampler_->GetHandle();

				write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write_desc_set.dstArrayElement = 0;
				write_desc_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write_desc_set.descriptorCount = 1;
				write_desc_set.pImageInfo = &vk_image_info_;
			}

			virtual void FillData(DataType& data, util::NullableRef<const Sampler> sampler) = 0;

			bool UpdateAndTryFillWrite(int frame_index, VkWriteDescriptorSet& write_desc_set)
			{
				DataType new_data;
				FillData(new_data, sampler_);

				auto&& [image] = new_data;
				image_view_.emplace(gloabl_, *image);

				if (images_per_frame_[frame_index] != image->GetHandle())
				{
					images_per_frame_[frame_index] = image->GetHandle();
					FillWriteDescriptorSet(frame_index, write_desc_set);
					return true;
				}
			}

		};

		template<DescriptorSetType Type, int BindingIndex>
		class BindingIter :
			public BindingIter<Type, BindingIndex - 1>,
			BindingData<typename DescriptorSet<Type>::template Binding<BindingIndex>::Data, DescriptorSet<Type>::template Binding<BindingIndex>::type>
		{
			//using DescriptorSetDesc = typename DescriptorSet<Type>;
			//using BindingDesc = typename DescriptorSetDesc::template BindingIter<BindingIndex>;
			//using BindingDataType = typename BindingDesc::Data;
			//using Data = typename BindingData<BindingDataType, typename DescriptorSetDesc::template BindingIter<BindingIndex>::type>;

		public:

			BindingIter(const Global& global) : 
				BindingData<typename DescriptorSet<Type>::template Binding<BindingIndex>::Data, DescriptorSet<Type>::template Binding<BindingIndex>::type>(global),
				BindingIter<Type, BindingIndex - 1>(global) {}


			int UpdateAndTryFillWrites(int frame_index, VkDescriptorSet descriptor_set, std::span<VkWriteDescriptorSet>& descriptor_set_writes)
			{
				int filled_writes = BindingIter<Type, BindingIndex - 1>::UpdateAndTryFillWrites(frame_index, descriptor_set, descriptor_set_writes);

				if (BindingData<typename DescriptorSet<Type>::template Binding<BindingIndex>::Data, DescriptorSet<Type>::template Binding<BindingIndex>::type>::UpdateAndTryFillWrite(frame_index, descriptor_set_writes[filled_writes]))
				{
					descriptor_set_writes[filled_writes].dstSet = descriptor_set;
					descriptor_set_writes[filled_writes].dstBinding = BindingIndex;
					return 1 + filled_writes;
				}

				return filled_writes;
			}

			//void FillDescriptorSetWrites(int frame_index, VkDescriptorSet descriptor_set, std::span<VkWriteDescriptorSet>& descriptor_set_writes)
			//{
			//	descriptor_set_writes[BindingIndex].dstSet = descriptor_set;
			//	descriptor_set_writes[BindingIndex].dstBinding = BindingIndex;

			//	BindingIter<Type, BindingIndex - 1>::FillDescriptorSetWrites(frame_index, descriptor_set, descriptor_set_writes);
			//}
		};

		template<DescriptorSetType Type>
		class BindingIter<Type, -1>
		{
		public:

			BindingIter(const Global& global) {}

			int UpdateAndTryFillWrites(int frame_index, VkDescriptorSet descriptor_set, std::span<VkWriteDescriptorSet>& descriptor_set_writes) { return 0; }

			//void FillDescriptorSetWrites(int frame_index, VkDescriptorSet descriptor_set, std::span<VkWriteDescriptorSet>& write_descriptor_sets) {}
		};

		template<DescriptorSetType Type>
		struct Set : public BindingIter<Type, DescriptorSet<Type>::binding_count - 1>
		{
		public:

			Set(const Global& global) : BindingIter<Type, DescriptorSet<Type>::binding_count - 1>(global),
				vk_descriptor_sets_
			{
				VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE
			}
			{}

			void FillDescriptorSetWrites(int frame_index, VkDescriptorSet descriptor_set, std::span<VkWriteDescriptorSet> write_descriptor_sets)
			{
				BindingIter<Type, DescriptorSet<Type>::binding_count - 1>::FillDescriptorSetWrites(frame_index, descriptor_set, write_descriptor_sets);
			}

			int UpdateAndTryFillWrites(int frame_index, std::span<VkWriteDescriptorSet>& descriptor_set_writes)
			{
				return BindingIter<Type, DescriptorSet<Type>::binding_count - 1>::UpdateAndTryFillWrites(frame_index, vk_descriptor_sets_[frame_index], descriptor_set_writes);
			}

			VkDescriptorSet AttachVkDescriptorSet(int frame_index, DescriptorSetsManager& manager)
			{
				assert(vk_descriptor_sets_[frame_index] == VK_NULL_HANDLE);

				vk_descriptor_sets_[frame_index] = manager.GetFreeDescriptor(Type);

				return vk_descriptor_sets_[frame_index];
			}

		protected:

			std::array<VkDescriptorSet, kFramesCount> vk_descriptor_sets_;
		};




		//--------------------------------------------------------------------------------------------------------------------------



		template<DescriptorSetType T1, DescriptorSetType... Ts>
		class SetIter : public SetIter<Ts...>, public Set<T1>
		{
		public:

			SetIter(const Global& global) : SetIter<Ts...>(global), Set<T1>(global)
			{}

		protected:

			//int FillDescriptorSetWrites(DescriptorSetsManager& manager, std::span<VkWriteDescriptorSet> write_descriptor_sets)
			//{
			//	int writes_filled_cnt = 0;

			//	for (int frame_ind = 0; frame_ind < kFramesCount; frame_ind++)
			//	{
			//		VkDescriptorSet vk_descriptor_set = manager.GetFreeDescriptor(T1);
			//		SetIter<Ts...>::descriptor_sets_per_frame_[frame_ind].emplace(T1, vk_descriptor_set);

			//		std::span<VkWriteDescriptorSet> writes_to_fill(write_descriptor_sets.data() + writes_filled_cnt, DescriptorSet<T1>::binding_count);

			//		Set<T1>::FillDescriptorSetWrites(vk_descriptor_set, writes_to_fill);
			//		writes_filled_cnt += DescriptorSet<T1>::binding_count;
			//	}

			//	std::span<VkWriteDescriptorSet> writes_to_fill(write_descriptor_sets.data() + writes_filled_cnt, write_descriptor_sets.size() - writes_filled_cnt);
			//	writes_filled_cnt += SetIter<Ts...>::FillDescriptorSetWrites(manager, writes_to_fill);

			//	return writes_filled_cnt;
			//}

			//void UpdateDataInternal(int frame_index)
			//{
			//	Set<T1>::UpdateDataInternal(frame_index);
			//	SetIter<Ts...>::UpdateDataInternal(frame_index);
			//}

			int UpdateAndTryFillWrites(int frame_index, std::span<VkWriteDescriptorSet> write_descriptor_sets)
			{
				int writes_filled_by_this_set = Set<T1>::UpdateAndTryFillWrites(frame_index, write_descriptor_sets);

				return writes_filled_by_this_set + SetIter<Ts...>::UpdateAndTryFillWrites(frame_index, std::span(write_descriptor_sets.begin() + writes_filled_by_this_set, write_descriptor_sets.end()));
			}

			void AttachVkDescriptorSet(int frame_index, DescriptorSetsManager& manager)
			{
				VkDescriptorSet vk_descriptor_set = Set<T1>::AttachVkDescriptorSet(frame_index, manager);

				SetIter<Ts...>::descriptor_sets_per_frame_[frame_index].emplace(T1, vk_descriptor_set);
			}

		};

		template<>
		class SetIter<DescriptorSetType::ListEnd> : public RenderObjBase<void*>
		{
		public:

			SetIter(const Global& global) : RenderObjBase(global) {}

		protected:

			int UpdateAndTryFillWrites(int frame_index, std::span<VkWriteDescriptorSet> write_descriptor_sets) { return 0; }

			void AttachVkDescriptorSet(int frame_index, DescriptorSetsManager& manager) {}

			std::array<std::map<DescriptorSetType, VkDescriptorSet>, kFramesCount> descriptor_sets_per_frame_;
		};

		template<DescriptorSetType... Ts>
		class Holder : public SetIter<Ts..., DescriptorSetType::ListEnd>
		{
		public:

			Holder(const Global& global, DescriptorSetsManager& manager) : SetIter<Ts..., DescriptorSetType::ListEnd>(global), global_(global)
			{
				for (int frame_index = 0; frame_index < kFramesCount; frame_index++)
				{
					SetIter<Ts..., DescriptorSetType::ListEnd>::AttachVkDescriptorSet(frame_index, manager);
					UpdateAndTryFillWrites(frame_index);
				}
			}

			void UpdateAndTryFillWrites(int frame_index)
			{
				std::array<VkWriteDescriptorSet, 64> writes = {};
				int filled_writes_cnt = SetIter<Ts..., DescriptorSetType::ListEnd>::UpdateAndTryFillWrites(frame_index, writes);
				if (filled_writes_cnt > 0)
				{
					vkUpdateDescriptorSets(RenderObjBase<void*>::global_.logical_device, filled_writes_cnt, writes.data(), 0, nullptr);
				}
			}

			const std::map<DescriptorSetType, VkDescriptorSet>& GetDescriptorSets(uint32_t frame_index) const
			{
				return SetIter<Ts..., DescriptorSetType::ListEnd>::descriptor_sets_per_frame_[frame_index];
			}

		protected:


			//void AttachDescriptorSets(DescriptorSetsManager& manager)
			//{
			//	//TODO: calc preciesly
			//	std::array<VkWriteDescriptorSet, 64> writes = {};
			//	int writes_filled_cnt = SetIter<Ts..., DescriptorSetType::ListEnd>::FillDescriptorSetWrites(manager, writes);
			//	vkUpdateDescriptorSets(RenderObjBase<void*>::global_.logical_device, writes_filled_cnt, writes.data(), 0, nullptr);
			//}

			const Global& global_;
		};
	}
}
#endif  // RENDER_ENGINE_RENDER_DESCRIPTOR_SET_HOLDER_H_