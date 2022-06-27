#ifndef RENDER_ENGINE_RENDER_DESCRIPTOR_SET_H_
#define RENDER_ENGINE_RENDER_DESCRIPTOR_SET_H_

#include <map>

#include "vulkan/vulkan.h"

#include "render/image_view.h"
#include "render/object_base.h"
#include "render/sampler.h"

namespace render
{
	enum class DescriptorSetType
	{

#define ENUM_OP(val) k##val, 
#include "render/descriptor_types.inl"
#undef ENUM_OP

		Count,
		
		ListEnd
		
	};

	enum class DescriptorBindingType
	{
		kUniform,
		kSampler,

		Count
	};




	template<DescriptorSetType Type>
	struct DescriptorSet;

	template<>
	struct DescriptorSet<DescriptorSetType::kCameraPositionAndViewProjMat>
	{
		static const uint32_t bindings_count = 1;

		template<int i>
		struct Binding;

		template<>
		struct Binding<0>
		{
			static const DescriptorBindingType type = DescriptorBindingType::kUniform;
			static const ShaderTypeFlags shaders_flags = ShaderTypeFlags::Vertex | ShaderTypeFlags::Fragment;

			struct Data
			{
				glm::vec4 position;
				glm::mat4 proj_view_mat;
			};

			Data data;
		};
	};

	template<>
	struct DescriptorSet<DescriptorSetType::kLightPositionAndViewProjMat>
	{
		static const uint32_t bindings_count = 1;

		template<int i>
		struct Binding;

		template<>
		struct Binding<0>
		{
			static const DescriptorBindingType type = DescriptorBindingType::kUniform;
			static const ShaderTypeFlags shaders_flags = ShaderTypeFlags::Vertex | ShaderTypeFlags::Fragment;

			struct Data
			{
				glm::vec4 position;
				glm::mat4 view_mat;
				glm::mat4 proj_mat;
				float near_plane;
				float far_plane;
			};

			Data data;
		};
	};

	template<>
	struct DescriptorSet<DescriptorSetType::kModelMatrix>
	{
		static const uint32_t bindings_count = 1;

		template<int i>
		struct Binding;

		template<>
		struct Binding<0>
		{
			static const DescriptorBindingType type = DescriptorBindingType::kUniform;
			static const ShaderTypeFlags shaders_flags = ShaderTypeFlags::Vertex;

			struct Data
			{
				glm::mat4 model_mat;
			};

			Data data;
		};
	};

	template<>
	struct DescriptorSet<DescriptorSetType::kSkeleton>
	{
		static const uint32_t bindings_count = 1;

		template<int i>
		struct Binding;

		template<>
		struct Binding<0>
		{
			static const DescriptorBindingType type = DescriptorBindingType::kUniform;
			static const ShaderTypeFlags shaders_flags = ShaderTypeFlags::Vertex;

			struct Data
			{
				glm::mat4 matrices[32];
				uint32_t use;
			};

			Data data;
		};
	};

	template<>
	struct DescriptorSet<DescriptorSetType::kMaterial>
	{
		static const uint32_t bindings_count = 2;

		template<int i>
		struct Binding;

		template<>
		struct Binding<0>
		{
			static const DescriptorBindingType type = DescriptorBindingType::kUniform;
			static const ShaderTypeFlags shaders_flags = ShaderTypeFlags::Fragment;

			struct Data
			{
				uint32_t emit;
			};

			Data data;
		};

		template<>
		struct Binding<1>
		{
			static const DescriptorBindingType type = DescriptorBindingType::kSampler;
			static const ShaderTypeFlags shaders_flags = ShaderTypeFlags::Fragment;

			struct Data
			{
				const Image* image;
				const Sampler* sampler;
			};

			Data data;
		};
	};


	template<>
	struct DescriptorSet<DescriptorSetType::kEnvironement>
	{
		static const uint32_t bindings_count = 2;

		template<int i>
		struct Binding;

		template<>
		struct Binding<0>
		{
			static const DescriptorBindingType type = DescriptorBindingType::kSampler;
			static const ShaderTypeFlags shaders_flags = ShaderTypeFlags::Fragment;

			struct Data
			{
				const Image* image;
				const Sampler* sampler;
			};

			Data data;
		};

		template<>
		struct Binding<1>
		{
			static const DescriptorBindingType type = DescriptorBindingType::kSampler;
			static const ShaderTypeFlags shaders_flags = ShaderTypeFlags::Fragment;

			struct Data
			{
				const Image* image;
				const Sampler* sampler;
			};

			Data data;
		};
	};

	template<>
	struct DescriptorSet<DescriptorSetType::kTexture>
	{
		static const uint32_t bindings_count = 1;

		template<int i>
		struct Binding;

		template<>
		struct Binding<0>
		{
			static const DescriptorBindingType type = DescriptorBindingType::kSampler;
			static const ShaderTypeFlags shaders_flags = ShaderTypeFlags::Fragment;

			struct Data
			{
				const Image* image;
				const Sampler* sampler;
			};

			Data data;
		};
	};


	struct DescriptorSetInfo
	{

		struct BindingInfo
		{
			DescriptorBindingType type;
			ShaderTypeFlags shaders_flags;
			uint32_t data_size;
		};

		std::vector<BindingInfo> bindings;
	};

	template<DescriptorSetType Type>
	struct DescriptorSetInfoBuilder
	{
		template<int BindingIndex>
		struct BindingStaticIter
		{
			static void UpdateInfo(DescriptorSetInfo& info)
			{
				BindingStaticIter<BindingIndex - 1>::UpdateInfo(info);
				info.bindings.push_back({ DescriptorSet<Type>::Binding<BindingIndex>::type, DescriptorSet<Type>::Binding<BindingIndex>::shaders_flags, sizeof(DescriptorSet<Type>::Binding<BindingIndex>::Data)});
			}
		};

		template<>
		struct BindingStaticIter<0>
		{
			static void UpdateInfo(DescriptorSetInfo& info)
			{
				info.bindings.push_back({ DescriptorSet<Type>::Binding<0>::type, DescriptorSet<Type>::Binding<0>::shaders_flags, sizeof(DescriptorSet<Type>::Binding<0>::Data) });
			}
		};

		static DescriptorSetInfo BuildInfo()
		{
			const int bindings_count = DescriptorSet<Type>::bindings_count;
			
			DescriptorSetInfo info;
			info.bindings.reserve(bindings_count);

			BindingStaticIter<bindings_count - 1>::UpdateInfo(info);

			return info;
		}
	};

	
	struct DescriptorSetsInfos
	{
		static std::map<DescriptorSetType, DescriptorSetInfo> Get()
		{
			std::map<DescriptorSetType, DescriptorSetInfo> infos;

#define ENUM_OP(x) infos[DescriptorSetType::k##x] = DescriptorSetInfoBuilder<DescriptorSetType::k##x>::BuildInfo();
#include "descriptor_types.inl"
#undef ENUM_OP

			return infos;
		}
	};


}
#endif  // RENDER_ENGINE_RENDER_DESCRIPTOR_SET_H_