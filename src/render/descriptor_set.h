#ifndef RENDER_ENGINE_RENDER_DESCRIPTOR_SET_H_
#define RENDER_ENGINE_RENDER_DESCRIPTOR_SET_H_

#include <map>

#include "vulkan/vulkan.h"

#include "render/data_types.h"
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
		None = Count,
		
		ListEnd
	};

	enum class DescriptorBindingType
	{
		kUniform,
		kSampler,

		Count
	};

	const uint32_t kDescriptorSetTypesCount = static_cast<uint32_t>(DescriptorSetType::Count);



	template<class T, int n = 16, class BindingType = void>
	struct BindingCounter
	{
		static const int count = n;
	};

	template<class T>
	struct BindingCounter<T, 0, void>
	{
		static const int count = 0;
	};

	template<class T, int n>
	struct BindingCounter<T, n, typename std::void_t< typename T::template Binding<n - 1>::NotBinded > > : BindingCounter<T, n - 1, void>
	{};


	template<DescriptorBindingType binding_type, ShaderTypeFlags shader_flags>
	struct BindingBase
	{
		static const DescriptorBindingType type = binding_type;
		static const ShaderTypeFlags shaders_flags = shader_flags;
	};

	template<ShaderTypeFlags shader_flags>
	struct BindingBase<DescriptorBindingType::kSampler, shader_flags>
	{
		static const DescriptorBindingType type = DescriptorBindingType::kSampler;
		static const ShaderTypeFlags shaders_flags = shader_flags;

		struct Data
		{};
	};


	template<DescriptorSetType Type>
	struct DescriptorSetBindings;

	template<>
	struct DescriptorSetBindings<DescriptorSetType::kCameraPositionAndViewProjMat>
	{
		template<int i>
		struct Binding {using NotBinded = void;};

		template<>
		struct Binding<0> : BindingBase<DescriptorBindingType::kUniform, ShaderTypeFlags::Vertex | ShaderTypeFlags::Geometry | ShaderTypeFlags::Fragment>
		{
			struct Data
			{
				glm::vec4 position;
				glm::mat4 proj_view_mat;
			};
		};
	};

	struct SamplerData
	{
		std::reference_wrapper<const Image> image;
		//std::reference_wrapper<ImageView> image_view;
		std::reference_wrapper<const Sampler> sampler;
	};

	template<>
	struct DescriptorSetBindings<DescriptorSetType::kLightPositionAndViewProjMat>
	{
		template<int i>
		struct Binding {using NotBinded = void;};

		template<>
		struct Binding<0> : BindingBase<DescriptorBindingType::kSampler, ShaderTypeFlags::Fragment>
		{
			struct Data
			{
				std::optional<SamplerData> cubemaps;
			};
		};

		template<>
		struct Binding<1> : BindingBase<DescriptorBindingType::kUniform, ShaderTypeFlags::Fragment>
		{
			struct Data
			{
				glm::vec4 position;
				glm::mat4 view_mat;
				glm::mat4 proj_mat;
				float near_plane;
				float far_plane;
			};
		};
	};

	template<>
	struct DescriptorSetBindings<DescriptorSetType::kModelMatrix>
	{
		template<int i>
		struct Binding {using NotBinded = void;};

		template<>
		struct Binding<0> : BindingBase<DescriptorBindingType::kUniform, ShaderTypeFlags::Vertex>
		{
			struct Data
			{
				glm::mat4 model_mat;
			};
		};
	};

	template<>
	struct DescriptorSetBindings<DescriptorSetType::kSkeleton>
	{
		template<int i>
		struct Binding {using NotBinded = void;};

		template<>
		struct Binding<0> : BindingBase<DescriptorBindingType::kUniform, ShaderTypeFlags::Vertex>
		{
			struct Data
			{
				glm::mat4 matrices[32];
			};
		};
	};

	template<>
	struct DescriptorSetBindings<DescriptorSetType::kMaterial>
	{
		template<int i>
		struct Binding {using NotBinded = void;};

		template<>
		struct Binding<0> : BindingBase<DescriptorBindingType::kUniform, ShaderTypeFlags::Fragment>
		{
			struct Data
			{
				uint32_t flags;
			};
		};

		template<>
		struct Binding<1> : BindingBase<DescriptorBindingType::kSampler, ShaderTypeFlags::Fragment>
		{
			struct Data
			{
				std::optional<SamplerData> albedo;
			};
		};

		template<>
		struct Binding<2> : BindingBase<DescriptorBindingType::kSampler, ShaderTypeFlags::Fragment>
		{
			struct Data
			{
				std::optional<SamplerData> metallic_roughness;
			};
		};

		template<>
		struct Binding<3> : BindingBase<DescriptorBindingType::kSampler, ShaderTypeFlags::Fragment>
		{
			struct Data
			{
				std::optional<SamplerData> normal_map;
			};
		};
	};


	template<>
	struct DescriptorSetBindings<DescriptorSetType::kEnvironement>
	{
		template<int i>
		struct Binding {using NotBinded = void;};

		template<>
		struct Binding<0> : BindingBase<DescriptorBindingType::kSampler, ShaderTypeFlags::Fragment>
		{
			struct Data
			{
				std::optional<SamplerData> environement;
			};
		};
	};

	template<>
	struct DescriptorSetBindings<DescriptorSetType::kTexture>
	{
		template<int i>
		struct Binding {using NotBinded = void;};

		template<>
		struct Binding<0> : BindingBase<DescriptorBindingType::kSampler, ShaderTypeFlags::Fragment>
		{
			struct Data
			{
				std::optional<SamplerData> texture;
			};
		};
	};

	template<>
	struct DescriptorSetBindings<DescriptorSetType::kBitmapAtlas>
	{
		template<int i>
		struct Binding { using NotBinded = void; };

		template<>
		struct Binding<0> : BindingBase<DescriptorBindingType::kUniform, ShaderTypeFlags::Vertex>
		{
			struct Data
			{
				glm::vec2 atlas_position;
				glm::vec2 width_heigth;
				glm::vec4 color;
			};
		};
	};

	template<>
	struct DescriptorSetBindings<DescriptorSetType::kGBuffers>
	{
		template<int i>
		struct Binding {using NotBinded = void;};

		template<>
		struct Binding<0> : BindingBase<DescriptorBindingType::kSampler, ShaderTypeFlags::Fragment>
		{
			struct Data
			{
				std::optional<SamplerData> albedo;
			};
		};

		template<>
		struct Binding<1> : BindingBase<DescriptorBindingType::kSampler, ShaderTypeFlags::Fragment>
		{
			struct Data
			{
				std::optional<SamplerData> position;
			};
		};

		template<>
		struct Binding<2> : BindingBase<DescriptorBindingType::kSampler, ShaderTypeFlags::Fragment>
		{
			struct Data
			{
				std::optional<SamplerData> normal;
			};
		};

		template<>
		struct Binding<3> : BindingBase<DescriptorBindingType::kSampler, ShaderTypeFlags::Fragment>
		{
			struct Data
			{
				std::optional<SamplerData> metallic_roughness;
			};
		};
	};


	template<>
	struct DescriptorSetBindings<DescriptorSetType::kColor>
	{
		template<int i>
		struct Binding { using NotBinded = void; };

		template<>
		struct Binding<0> : BindingBase<DescriptorBindingType::kUniform, ShaderTypeFlags::Fragment>
		{
			struct Data
			{
				glm::vec4 color;
			};
		};
	};

	template<>
	struct DescriptorSetBindings<DescriptorSetType::kShadowCubeMapParams>
	{
		template<int i>
		struct Binding { using NotBinded = void; };

		template<>
		struct Binding<0> : BindingBase<DescriptorBindingType::kUniform, ShaderTypeFlags::Geometry>
		{
			struct Data
			{
				glm::mat4 cube_views[6];
				glm::mat4 cube_proj;
			};
		};
	};


	template<>
	struct DescriptorSetBindings<DescriptorSetType::kShadowCubeMaps>
	{
		template<int i>
		struct Binding { using NotBinded = void; };

		template<>
		struct Binding<0> : BindingBase<DescriptorBindingType::kSampler, ShaderTypeFlags::Fragment>
		{
			struct Data
			{
				std::optional<SamplerData> cubemaps;
			};
		};
	};

	template<DescriptorSetType Type>
	struct DescriptorSet : DescriptorSetBindings<Type>
	{
		static const uint32_t binding_count = BindingCounter<DescriptorSetBindings<Type>>::count;
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
			const int binding_count = DescriptorSet<Type>::binding_count;
			
			DescriptorSetInfo info;
			info.bindings.reserve(binding_count);

			BindingStaticIter<binding_count - 1>::UpdateInfo(info);

			return info;
		}
	};

	
	struct DescriptorSetUtil
	{
		

		static const std::map<DescriptorSetType, DescriptorSetInfo>& GetTypeToInfoMap()
		{
			if (info_map_.size() == 0)
			{
#define ENUM_OP(x) info_map_[DescriptorSetType::k##x] = DescriptorSetInfoBuilder<DescriptorSetType::k##x>::BuildInfo();
#include "descriptor_types.inl"
#undef ENUM_OP
			}
			return info_map_;
		}

		static const std::map<std::string, DescriptorSetType>& GetNameToTypeMap()
		{
			if (name_map_.size() == 0)
			{
#define ENUM_OP(x) name_map_[#x] = DescriptorSetType::k##x;
#include "descriptor_types.inl"
#undef ENUM_OP
			}
			return name_map_;
		}

	private:

		static std::map<DescriptorSetType, DescriptorSetInfo> info_map_;
		static std::map<std::string, DescriptorSetType> name_map_;

	};


}
#endif  // RENDER_ENGINE_RENDER_DESCRIPTOR_SET_H_