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
#include "render/global.h"

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
		protected:
			std::reference_wrapper<const Global> global_ref_;
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
			},
				global_ref_(global)
			{}

			virtual bool FillData(DataType& data) = 0;

			bool UpdateAndTryFillWrite(int frame_index, VkWriteDescriptorSet& write_desc_set)
			{
				uint32_t size = sizeof(DataType);

				DataType new_data;
				if (FillData(new_data))
				{
					void* mapped_data;
					vkMapMemory(uniform_buffers_[frame_index].GetGlobal().logical_device, uniform_buffers_[frame_index].GetBufferMemory().vk_memory, uniform_buffers_[frame_index].GetBufferMemory().offset, size, 0, &mapped_data);
					memcpy(mapped_data, &new_data, size);
					vkUnmapMemory(uniform_buffers_[frame_index].GetGlobal().logical_device, uniform_buffers_[frame_index].GetBufferMemory().vk_memory);
				}
				else
				{
					DebugBreak();
				}


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
			std::array<VkImage, kFramesCount> images_per_frame_;

			std::array<std::optional<ImageView>, kFramesCount> image_views_;

			VkDescriptorImageInfo vk_image_info_;

		protected:
			std::reference_wrapper<const Global> global_ref_;

		public:

			BindingData(const Global& global) : global_ref_(global),
				images_per_frame_
			{
				VK_NULL_HANDLE,VK_NULL_HANDLE,VK_NULL_HANDLE,VK_NULL_HANDLE
			}
			{}

			void FillWriteDescriptorSet(int frame_index, VkWriteDescriptorSet& write_desc_set, SamplerData& sampler_data)
			{
				vk_image_info_.imageLayout = image_views_[frame_index]->GetFormat() == image_views_[frame_index]->GetGlobal().depth_map_format ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				vk_image_info_.imageView = image_views_[frame_index]->GetHandle();
				vk_image_info_.sampler = sampler_data.sampler.get().GetHandle();

				write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write_desc_set.dstArrayElement = 0;
				write_desc_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write_desc_set.descriptorCount = 1;
				write_desc_set.pImageInfo = &vk_image_info_;
			}

			virtual bool FillData(DataType& data) = 0;

			bool UpdateAndTryFillWrite(int frame_index, VkWriteDescriptorSet& write_desc_set)
			{
				DataType new_data;
				if (FillData(new_data))
				{

					auto&& [sampler_data] = new_data;

					if (images_per_frame_[frame_index] != sampler_data->image.get().GetHandle())
					{
						image_views_[frame_index].emplace(global_ref_.get(), sampler_data->image.get());
						images_per_frame_[frame_index] = sampler_data->image.get().GetHandle();
						FillWriteDescriptorSet(frame_index, write_desc_set, *sampler_data);
						return true;
					}

				}
				else
				{
					DebugBreak();
				}

				return false;
			}

		};

		template<DescriptorSetType Type, int BindingIndex>
		class BindingIter :
			public BindingIter<Type, BindingIndex - 1>,
			public BindingData<typename DescriptorSet<Type>::template Binding<BindingIndex>::Data, DescriptorSet<Type>::template Binding<BindingIndex>::type>
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

			Set(const Set& set) = delete;
			Set(Set&& set) : BindingIter<Type, DescriptorSet<Type>::binding_count - 1>(std::move(set))
			{
				vk_descriptor_sets_ = set.vk_descriptor_sets_;
				set.vk_descriptor_sets_.fill(VK_NULL_HANDLE);
			}

			Set& operator=(const Set& set) = delete;
			Set& operator=(Set&& set)
			{
				BindingIter<Type, DescriptorSet<Type>::binding_count - 1>::operator=(std::move(set));

				FreeDescriptorSets(BindingIter<Type, DescriptorSet<Type>::binding_count - 1>::global_ref_);
				vk_descriptor_sets_ = set.vk_descriptor_sets_;
				set.vk_descriptor_sets_.fill(VK_NULL_HANDLE);

				return *this;
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

			void FreeDescriptorSets(const Global& global)
			{
				for (auto&& set : vk_descriptor_sets_)
				{
					if (set != VK_NULL_HANDLE)
					{
						global.delete_list.push_back({ global.frame_ind, set });
						set = VK_NULL_HANDLE;
					}
				}
			}

		protected:

			std::array<VkDescriptorSet, kFramesCount> vk_descriptor_sets_;
		};




		//--------------------------------------------------------------------------------------------------------------------------



		template<DescriptorSetType T1, DescriptorSetType... Ts>
		class SetIter : public SetIter<Ts...>, public Set<T1>
		{
		public:

			SetIter(const Global& global, DescriptorSetsManager& manager) : SetIter<Ts...>(global, manager), Set<T1>(global)
			{}

			SetIter(const SetIter& iter) = delete;
			SetIter(SetIter&& iter) = default;

			SetIter& operator=(const SetIter& iter) = delete;
			SetIter& operator=(SetIter&& iter) = default;
			~SetIter()
			{
				Set<T1>::FreeDescriptorSets(SetIter<Ts...>::global_);
			}

		protected:

			int UpdateAndTryFillWrites(int frame_index, std::span<VkWriteDescriptorSet> write_descriptor_sets)
			{
				int writes_filled_by_this_set = Set<T1>::UpdateAndTryFillWrites(frame_index, write_descriptor_sets);

				return writes_filled_by_this_set + SetIter<Ts...>::UpdateAndTryFillWrites(frame_index, std::span(write_descriptor_sets.begin() + writes_filled_by_this_set, write_descriptor_sets.end()));
			}

			void AttachVkDescriptorSet(int frame_index)
			{
				VkDescriptorSet vk_descriptor_set = Set<T1>::AttachVkDescriptorSet(frame_index, SetIter<Ts...>::desc_set_manager_);
				SetIter<Ts...>::descriptor_sets_per_frame_[frame_index].emplace(T1, vk_descriptor_set);

				SetIter<Ts...>::AttachVkDescriptorSet(frame_index);
			}

		};

		template<>
		class SetIter<DescriptorSetType::ListEnd> : public RenderObjBase<void*>
		{
		public:

			SetIter(const Global& global, DescriptorSetsManager& manager) : RenderObjBase(global), desc_set_manager_(manager) {}

		protected:

			int UpdateAndTryFillWrites(int frame_index, std::span<VkWriteDescriptorSet> write_descriptor_sets) { return 0; }

			void AttachVkDescriptorSet(int frame_index) {}

			std::array<std::map<DescriptorSetType, VkDescriptorSet>, kFramesCount> descriptor_sets_per_frame_;
			std::reference_wrapper<DescriptorSetsManager> desc_set_manager_;
		};

		template<DescriptorSetType... Ts>
		class Holder : public SetIter<Ts..., DescriptorSetType::ListEnd>
		{
		public:

			Holder(const Global& global, DescriptorSetsManager& manager) : SetIter<Ts..., DescriptorSetType::ListEnd>(global, manager)
			{
				for (int frame_index = 0; frame_index < kFramesCount; frame_index++)
				{
					SetIter<Ts..., DescriptorSetType::ListEnd>::AttachVkDescriptorSet(frame_index);
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
		};
	}
}
#endif  // RENDER_ENGINE_RENDER_DESCRIPTOR_SET_HOLDER_H_