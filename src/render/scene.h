#ifndef RENDER_ENGINE_RENDER_SCENE_H_
#define RENDER_ENGINE_RENDER_SCENE_H_

#include "vulkan/vulkan.h"

#include "common.h"
#include "render/object_base.h"
#include "render/batches_manager.h"
#include "render/graphics_pipeline.h"
#include "render/image_view.h"
#include "render/ui/ui.h"

#include "render/descriptor_set_holder.h"


namespace render
{

	struct Primitive
	{
		const std::vector<BufferAccessor>& vertex_buffers;
		const BufferAccessor& index_buffer;
		RenderModelType type;
	};




	class Model : public DescriptorSetHolder<NoChild, DescriptorSetType::kModelMatrix, DescriptorSetType::kSkeleton, DescriptorSetType::kMaterial>
	{
	public:

		Model(const DeviceConfiguration& device_cfg, const Batch& batch, const Sampler& diffuse_sampler);
		const Batch& GetBatch() const;

		void FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<0>::Data& data) override;
		void FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<1>::Data& data) override;

		void FillData(render::DescriptorSet<render::DescriptorSetType::kSkeleton>::Binding<0>::Data& data) override;

		void FillData(render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data) override;

		const std::vector<Primitive>& GetPrimitives() const;

	private:
		const Batch& batch_;
		const Sampler& diffuse_sampler_;

		std::vector<Primitive> primitives_;
	};

	class ModelScene : public DescriptorSetHolder<Model, DescriptorSetType::kCameraPositionAndViewProjMat, DescriptorSetType::kLightPositionAndViewProjMat, DescriptorSetType::kEnvironement>
	{
	public:

		ModelScene(const DeviceConfiguration& device_cfg, const BatchesManager& batch_manager, const Image& shadow_map);

		const std::vector<Model>& GetModels() const;

		void UpdateCameraData(glm::vec3 pos, glm::vec3 look, float aspect);

		void FillData(render::DescriptorSet<render::DescriptorSetType::kCameraPositionAndViewProjMat>::Binding<0>::Data& data) override;
		void FillData(render::DescriptorSet<render::DescriptorSetType::kLightPositionAndViewProjMat>::Binding<0>::Data& data) override;
		void FillData(render::DescriptorSet<render::DescriptorSetType::kEnvironement>::Binding<0>::Data& data) override;
		void FillData(render::DescriptorSet<render::DescriptorSetType::kEnvironement>::Binding<1>::Data& data) override;

		virtual std::vector<std::reference_wrapper<Model>>& GetChildren() override;

	private:
		
		std::vector<Model> models_;
		std::vector<std::reference_wrapper<Model>> children_;

		DescriptorSet<DescriptorSetType::kCameraPositionAndViewProjMat>::Binding<0>::Data camera_data_;
		DescriptorSet<DescriptorSetType::kLightPositionAndViewProjMat>::Binding<0>::Data light_data_;

		Image env_image_;
		const Image& shadow_map_;

		Sampler diffuse_sampler_;
		Sampler shadow_sampler_;
	};



	class UIPoly : public DescriptorSetHolder<NoChild, DescriptorSetType::kModelMatrix>
	{
		const ui::UI& ui_;
		std::vector<Primitive> primitives_;

		GPULocalBuffer polygon_vert_pos_;

		std::vector<BufferAccessor> vertex_buffers_;

	public:
		UIPoly(const DeviceConfiguration& device_cfg, const ui::UI& ui, glm::vec2 pos, uint32_t height);

		void FillData(render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data) override;

		const std::vector<Primitive>& GetPrimitives() const;
	};

	class UIScene : public DescriptorSetHolder<UIPoly, DescriptorSetType::kTexture>
	{
		const ui::UI& ui_;

		std::vector<UIPoly> ui_polygones_;
		std::vector<std::reference_wrapper<UIPoly>> ui_polygones_geom_;

	public:
		UIScene(const DeviceConfiguration& device_cfg, const ui::UI& ui);

		void FillData(render::DescriptorSet<render::DescriptorSetType::kTexture>::Binding<0>::Data& data) override;

		virtual std::vector<std::reference_wrapper<UIPoly>>& GetChildren() override;
	};
}

#endif  // RENDER_ENGINE_RENDER_SCENE_H_