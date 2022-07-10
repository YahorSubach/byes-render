#include "shader_module.h"

#include <fstream>

render::ShaderModule::ShaderModule(const DeviceConfiguration& device_cfg, const std::string& shader_path, const std::array<DescriptorSetLayout, static_cast<uint32_t>(DescriptorSetType::Count)>& descriptor_sets_layouts) : RenderObjBase(device_cfg)
{
	std::ifstream file("../shaders/" + shader_path, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = buffer.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

	if (vkCreateShaderModule(device_cfg_.logical_device, &createInfo, nullptr, &handle_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	if (shader_path.find("vert") != std::string::npos)
	{
		shader_type_ = ShaderType::Vertex;
	}
	else if (shader_path.find("frag") != std::string::npos)
	{
		shader_type_ = ShaderType::Fragment;
	}
	else
	{
		shader_type_ = ShaderType::Invalid;
	}

	if (shader_path.find("color") != std::string::npos || shader_path.find("shadow") != std::string::npos || shader_path.find("ui") != std::string::npos || shader_path.find("build_g_buffers") != std::string::npos)
	{
		if (shader_path.find("vert") != std::string::npos)
		{
			if (shader_path.find("ui") != std::string::npos)
			{
				VertexBindingDesc position_binding =
				{
				(32 / 8) * 3,

					{
						{ VK_FORMAT_R32G32B32_SFLOAT, 0}
					}
				};

				VertexBindingDesc tex_binding =
				{
				(32 / 8) * 2,

					{
						{ VK_FORMAT_R32G32_SFLOAT, 0}
					}
				};

				vertex_bindings_descs_ = { position_binding , tex_binding };

			}
			else
			{
				VertexBindingDesc position_binding =
				{
				(32 / 8) * 3,

					{
						{ VK_FORMAT_R32G32B32_SFLOAT, 0}
					}
				};

				VertexBindingDesc normal_binding =
				{
				(32 / 8) * 3,

					{
						{ VK_FORMAT_R32G32B32_SFLOAT, 0}
					}
				};

				VertexBindingDesc tex_binding =
				{
				(32 / 8) * 2,

					{
						{ VK_FORMAT_R32G32_SFLOAT, 0}
					}
				};

				if (shader_path.find("skin") != std::string::npos)
				{

					VertexBindingDesc joints_binding =
					{
					(8 / 8) * 4,

						{
							{ VK_FORMAT_R8G8B8A8_UINT, 0}
						}
					};

					VertexBindingDesc weights_binding =
					{
					(32 / 8) * 4,

						{
							{ VK_FORMAT_R32G32B32A32_SFLOAT, 0}
						}
					};

					vertex_bindings_descs_ = { position_binding , normal_binding , tex_binding, joints_binding, weights_binding };
				}
				else
				{
					vertex_bindings_descs_ = { position_binding , normal_binding , tex_binding};
				}
			}


		}
	}


	if ((shader_path.find("color") != std::string::npos || shader_path.find("build_g_buffers") != std::string::npos) && shader_path.find("vert") != std::string::npos)
	{
		descriptor_sets_.emplace(0, descriptor_sets_layouts[static_cast<int>(DescriptorSetType::kCameraPositionAndViewProjMat)]);
		descriptor_sets_.emplace(1, descriptor_sets_layouts[static_cast<int>(DescriptorSetType::kModelMatrix)]);

		if (shader_path.find("skin") != std::string::npos)
		{
			descriptor_sets_.emplace(5, descriptor_sets_layouts[static_cast<int>(DescriptorSetType::kSkeleton)]);
		}
	}
	else
	if ((shader_path.find("color") != std::string::npos || shader_path.find("build_g_buffers") != std::string::npos) && shader_path.find("frag") != std::string::npos)
	{
		descriptor_sets_.emplace(2, descriptor_sets_layouts[static_cast<int>(DescriptorSetType::kMaterial)]);
		descriptor_sets_.emplace(3, descriptor_sets_layouts[static_cast<int>(DescriptorSetType::kLightPositionAndViewProjMat)]);
		descriptor_sets_.emplace(4, descriptor_sets_layouts[static_cast<int>(DescriptorSetType::kEnvironement)]);
	}
	else
	if (shader_path.find("shadow") != std::string::npos && shader_path.find("vert") != std::string::npos)
	{
		descriptor_sets_.emplace(0, descriptor_sets_layouts[static_cast<int>(DescriptorSetType::kLightPositionAndViewProjMat)]);
		descriptor_sets_.emplace(1, descriptor_sets_layouts[static_cast<int>(DescriptorSetType::kModelMatrix)]);

		if (shader_path.find("skin") != std::string::npos)
		{
			descriptor_sets_.emplace(2, descriptor_sets_layouts[static_cast<int>(DescriptorSetType::kSkeleton)]);
		}
	}
	else
	if (shader_path.find("shadow") != std::string::npos && shader_path.find("frag") != std::string::npos)
	{
	}
	else
	if (shader_path.find("ui") != std::string::npos && shader_path.find("frag") != std::string::npos)
	{
		descriptor_sets_.emplace(0, descriptor_sets_layouts[static_cast<int>(DescriptorSetType::kModelMatrix)]);
		descriptor_sets_.emplace(1, descriptor_sets_layouts[static_cast<int>(DescriptorSetType::kTexture)]);
	}
	else
	if (shader_path.find("collect_g_buffers") != std::string::npos && shader_path.find("frag") != std::string::npos)
	{
		descriptor_sets_.emplace(0, descriptor_sets_layouts[static_cast<int>(DescriptorSetType::kGBuffers)]);
	}
}

const std::vector<render::ShaderModule::VertexBindingDesc>& render::ShaderModule::GetVertexBindingsDescs() const
{
	return vertex_bindings_descs_;
}

const std::map<uint32_t, const render::DescriptorSetLayout&>& render::ShaderModule::GetDescriptorSets() const
{
	return descriptor_sets_;
}


render::ShaderModule::~ShaderModule()
{
	if (handle_ != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(device_cfg_.logical_device, handle_, nullptr);
	}
}

render::ShaderType render::ShaderModule::GetShaderType() const
{
	return shader_type_;
}
