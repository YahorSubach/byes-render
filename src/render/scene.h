#ifndef RENDER_ENGINE_RENDER_SCENE_H_
#define RENDER_ENGINE_RENDER_SCENE_H_

#include "vulkan/vulkan.h"

#include "common.h"
#include "render/object_base.h"
#include "render/batches_manager.h"
#include "render/graphics_pipeline.h"
#include "render/image_view.h"
#include "render/ui/ui.h"
#include "render/ui/panel.h"

#include "render/descriptor_set_holder.h"


namespace render
{
	class RenderNodeBase
	{
	public:
		RenderNodeBase(DescriptorSetHolderInternal<DescriptorSetType::ListEnd>& desc_set_holder) : desc_set_holder_(desc_set_holder) {}
		const std::map<DescriptorSetType, VkDescriptorSet>& GetDescriptorSets() const { return desc_set_holder_.GetDescriptorSets(); }

	protected:
		DescriptorSetHolderInternal<DescriptorSetType::ListEnd>& desc_set_holder_;
	};

	class PrimitivesHolderRenderNode : public RenderNodeBase
	{

	public:

		PrimitivesHolderRenderNode(DescriptorSetHolderInternal<DescriptorSetType::ListEnd>& desc_set_holder, const std::vector<Primitive>& primitives) : RenderNodeBase(desc_set_holder), primitives_(primitives) {}
		
		const std::vector<Primitive>& GetPrimitives() const { return primitives_; }
	
	protected:
		const std::vector<Primitive>& primitives_;
	
	};

	class SceneRenderNode : public RenderNodeBase
	{
	public:

		SceneRenderNode(DescriptorSetHolderInternal<DescriptorSetType::ListEnd>& desc_set_holder, const std::vector<PrimitivesHolderRenderNode>& children) : RenderNodeBase(desc_set_holder), children_(children) {}

		const std::vector<PrimitivesHolderRenderNode>& GetChildren() const { return children_; }

	protected:
		const std::vector<PrimitivesHolderRenderNode>& children_;
	};

	class ModelDescSetHolder : public DescriptorSetHolder<DescriptorSetType::kModelMatrix, DescriptorSetType::kSkeleton, DescriptorSetType::kMaterial>
	{
	public:

		ModelDescSetHolder(const DeviceConfiguration& device_cfg, const Mesh& mesh);
		const Mesh& GetMesh() const;

		void FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<0>::Data& data) override;
		void FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<1>::Data& data) override;

		void FillData(render::DescriptorSet<render::DescriptorSetType::kSkeleton>::Binding<0>::Data& data) override;

		void FillData(render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data) override;

		const std::vector<Primitive>& GetPrimitives() const;

		PrimitivesHolderRenderNode GetRenderNode();

	private:
		const Mesh& mesh_;
		Sampler diffuse_sampler_;
	};


	class ModelSceneDescSetHolder : public DescriptorSetHolder<DescriptorSetType::kCameraPositionAndViewProjMat, DescriptorSetType::kLightPositionAndViewProjMat, DescriptorSetType::kEnvironement>
	{
	public:

		ModelSceneDescSetHolder(const DeviceConfiguration& device_cfg, const BatchesManager& batch_manager, const Image& shadow_map);

		const std::vector<ModelDescSetHolder>& GetModels() const;

		void UpdateCameraData(glm::vec3 pos, glm::vec3 look, float aspect);

		void FillData(render::DescriptorSet<render::DescriptorSetType::kCameraPositionAndViewProjMat>::Binding<0>::Data& data) override;
		void FillData(render::DescriptorSet<render::DescriptorSetType::kLightPositionAndViewProjMat>::Binding<0>::Data& data) override;
		void FillData(render::DescriptorSet<render::DescriptorSetType::kEnvironement>::Binding<0>::Data& data) override;
		void FillData(render::DescriptorSet<render::DescriptorSetType::kEnvironement>::Binding<1>::Data& data) override;

		SceneRenderNode GetRenderNode();

		void UpdateData();

		void AttachDescriptorSets(DescriptorSetsManager& manager);

	private:
		
		std::vector<ModelDescSetHolder> models_;
		std::vector<PrimitivesHolderRenderNode> children_nodes_;

		DescriptorSet<DescriptorSetType::kCameraPositionAndViewProjMat>::Binding<0>::Data camera_data_;
		DescriptorSet<DescriptorSetType::kLightPositionAndViewProjMat>::Binding<0>::Data light_data_;

		Image env_image_;
		const Image& shadow_map_;

		Sampler diffuse_sampler_;
		Sampler shadow_sampler_;
	};


	class UIPoly : public DescriptorSetHolder<DescriptorSetType::kModelMatrix, DescriptorSetType::kTexture>
	{
		const ui::UI& ui_;
		std::vector<Primitive> primitives_;

		glm::mat4 transform_;
		const Image& image_;

	public:
		UIPoly(const DeviceConfiguration& device_cfg, const ui::UI& ui, glm::mat4 transform, const Image& image);

		void FillData(render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data) override;
		void FillData(render::DescriptorSet<render::DescriptorSetType::kTexture>::Binding<0>::Data& data) override;

		PrimitivesHolderRenderNode GetRenderNode();
	};

	class UIScene : public DescriptorSetHolder<>
	{
		const ui::UI& ui_;

		ui::Panel screen_panel_;
		ui::TextBlock text_block_;

		std::vector<UIPoly> ui_polygones_;
		std::vector<std::reference_wrapper<UIPoly>> ui_polygones_geom_;

	public:
		UIScene(const DeviceConfiguration& device_cfg, const ui::UI& ui);

		SceneRenderNode GetRenderNode();

		void UpdateData();

		void AttachDescriptorSets(DescriptorSetsManager& manager);

		std::vector<PrimitivesHolderRenderNode> children_nodes_;
	};
}

#endif  // RENDER_ENGINE_RENDER_SCENE_H_