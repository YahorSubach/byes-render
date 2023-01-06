#ifndef RENDER_ENGINE_RENDER_SHADER_MODULE_H_
#define RENDER_ENGINE_RENDER_SHADER_MODULE_H_

#include <array>
#include <map>

#include "vulkan/vulkan.h"

#include "render/object_base.h"
#include "render/descriptor_set.h"
#include "render/descriptor_set_layout.h"
#include "render/vertex_buffer.h"

namespace render
{
	class ShaderModule : public RenderObjBase<VkShaderModule>
	{
	public:

		struct VertexBindingAttributeDesc
		{
			uint32_t format;
			uint32_t offset;
			VertexBufferType type;
		};

		struct VertexBindingDesc
		{
			uint32_t stride;
			std::map<uint32_t, VertexBindingAttributeDesc> attributes;
		};

		struct DescriptorSetLayoutBindingDesc
		{
			uint32_t binding_index;
			uint32_t type;
		};

		//struct DescriptorSetLayoutDesc
		//{
		//	DescriptorSetId set_id;
		//	std::vector<uint32_t> bindings_descs_types;
		//};

		ShaderModule(const DeviceConfiguration& device_cfg, const std::string& shader_path, const std::array<DescriptorSetLayout, kDescriptorSetTypesCount>& descriptor_sets_layouts);

		ShaderModule(const ShaderModule&) = delete;
		ShaderModule(ShaderModule&&) = default;

		ShaderModule& operator=(const ShaderModule&) = delete;
		ShaderModule& operator=(ShaderModule&&) = default;

		const std::map<uint32_t, VertexBindingDesc>& GetInputBindingsDescs() const;
		const std::map<uint32_t, const render::DescriptorSetLayout&>& GetDescriptorSets() const;


		//const std::vector<DescriptorSetLayoutDesc>& GetDescriptorSetLayoutsDescs() const;

		virtual ~ShaderModule() override;

		ShaderType GetShaderType() const;

	private:

		void FillInputDescsAndDescSets(const std::string& shader_path, const std::array<DescriptorSetLayout, kDescriptorSetTypesCount>& descriptor_sets_layouts);

		ShaderType shader_type_;

		std::map<uint32_t, VertexBindingDesc> input_bindings_descs_;
		std::map<uint32_t, const DescriptorSetLayout&> descriptor_sets_;
		//std::vector<DescriptorSetLayoutDesc> descriptor_sets_descs_;
	};
}
#endif  // RENDER_ENGINE_RENDER_RENDER_PASS_H_