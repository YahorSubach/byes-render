#ifndef RENDER_ENGINE_RENDER_SCENE_H_
#define RENDER_ENGINE_RENDER_SCENE_H_

#include "vulkan/vulkan.h"

#include "common.h"
#include "render/object_base.h"
#include "render/batches_manager.h"
#include "render/graphics_pipeline.h"
#include "render/framebuffer_collection.h"
#include "render/image_view.h"
#include "render/ui/ui.h"
#include "render/ui/panel.h"

#include "render/descriptor_set_holder.h"


namespace render
{

	struct Model
	{
		Node& node;
		util::NullableRef<Mesh> mesh;
		util::NullableRef<Skin> skin;

		struct DescritorSetsHolderImpl : public descriptor_sets_holder::Holder<Model, DescriptorSetType::kModelMatrix, DescriptorSetType::kSkeleton, DescriptorSetType::kMaterial>
		{
			void FillData(const Model& scene, render::DescriptorSet<render::DescriptorSetType::kCameraPositionAndViewProjMat>::Binding<0>::Data& data) override;
			void FillData(const Model& scene, render::DescriptorSet<render::DescriptorSetType::kLightPositionAndViewProjMat>::Binding<0>::Data& data) override;
			void FillData(const Model& scene, render::DescriptorSet<render::DescriptorSetType::kEnvironement>::Binding<0>::Data& data) override;
		};
	};


	//class RenderNodeBase
	//{
	//public:
	//	RenderNodeBase(DescriptorSetsHolderInternal<DescriptorSetType::ListEnd>& desc_set_holder) : desc_set_holder_(desc_set_holder) {}
	//	const std::map<DescriptorSetType, VkDescriptorSet>& GetDescriptorSets() const { return desc_set_holder_.GetDescriptorSets(); }

	//protected:
	//	DescriptorSetsHolderInternal<DescriptorSetType::ListEnd>& desc_set_holder_;
	//};

	//class PrimitivesHolderRenderNode : public RenderNodeBase
	//{

	//public:

	//	PrimitivesHolderRenderNode(DescriptorSetsHolderInternal<DescriptorSetType::ListEnd>& desc_set_holder, const std::vector<Primitive>& primitives) : RenderNodeBase(desc_set_holder), primitives_(primitives) {}
	//	
	//	const std::vector<Primitive>& GetPrimitives() const { return primitives_; }
	//
	//protected:
	//	const std::vector<Primitive>& primitives_;
	//
	//};

	//class SceneRenderNode : public RenderNodeBase
	//{
	//public:

	//	SceneRenderNode(DescriptorSetsHolderInternal<DescriptorSetType::ListEnd>& desc_set_holder, const std::vector<PrimitivesHolderRenderNode>& children) : RenderNodeBase(desc_set_holder), children_(children) {}

	//	const std::vector<PrimitivesHolderRenderNode>& GetChildren() const { return children_; }

	//protected:
	//	const std::vector<PrimitivesHolderRenderNode>& children_;
	//};

	//class ModelDescSetHolder : public DescriptorSetsHolder<DescriptorSetType::kModelMatrix, DescriptorSetType::kSkeleton, DescriptorSetType::kMaterial>
	//{
	//public:

	//	ModelDescSetHolder(const DeviceConfiguration& device_cfg, const Model& model);
	//	const Model& GetModel() const;

	//	void FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<0>::Data& data) override;
	//	void FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<1>::Data& data) override;
	//	void FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<2>::Data& data) override;
	//	void FillData(render::DescriptorSet<render::DescriptorSetType::kMaterial>::Binding<3>::Data& data) override;

	//	void FillData(render::DescriptorSet<render::DescriptorSetType::kSkeleton>::Binding<0>::Data& data) override;

	//	void FillData(render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data) override;


	//	PrimitivesHolderRenderNode GetRenderNode();

	//private:
	//	const Model& model_;
	//	std::optional<Sampler> diffuse_sampler_;
	//};


	//class ModelSceneDescSetHolder : public DescriptorSetsHolder<Scene, DescriptorSetType::kCameraPositionAndViewProjMat, DescriptorSetType::kLightPositionAndViewProjMat, DescriptorSetType::kEnvironement>
	//{
	//public:

	//	ModelSceneDescSetHolder(const DeviceConfiguration& device_cfg, const Scene& scene);

	//	const std::vector<ModelDescSetHolder>& GetModels() const;

	//	void FillData(const Scene& scene, render::DescriptorSet<render::DescriptorSetType::kCameraPositionAndViewProjMat>::Binding<0>::Data& data) override;
	//	void FillData(render::DescriptorSet<render::DescriptorSetType::kLightPositionAndViewProjMat>::Binding<0>::Data& data) override;
	//	void FillData(render::DescriptorSet<render::DescriptorSetType::kEnvironement>::Binding<0>::Data& data) override;
	//	//void FillData(render::DescriptorSet<render::DescriptorSetType::kEnvironement>::Binding<1>::Data& data) override;
	//	//void FillData(render::DescriptorSet<render::DescriptorSetType::kGBuffers>::Binding<0>::Data& data) override;
	//	//void FillData(render::DescriptorSet<render::DescriptorSetType::kGBuffers>::Binding<1>::Data& data) override;
	//	//void FillData(render::DescriptorSet<render::DescriptorSetType::kGBuffers>::Binding<2>::Data& data) override;
	//	//void FillData(render::DescriptorSet<render::DescriptorSetType::kGBuffers>::Binding<3>::Data& data) override;

	//	SceneRenderNode GetRenderNode();

	//	void UpdateData();

	//	void AttachDescriptorSets(DescriptorSetsManager& manager);

	//	//void AddModel(const render::Mesh& model);

	//	util::NullableRef<const Image> g_albedo_image;
	//	util::NullableRef<const Image> g_position_image;
	//	util::NullableRef<const Image> g_normal_image;
	//	util::NullableRef<const Image> g_metal_rough_image;

	//	util::NullableRef<const Image> shadowmap_image;

	//private:
	//	
	//	std::vector<ModelDescSetHolder> model_descriptor_sets_holders_;
	//	std::vector<PrimitivesHolderRenderNode> children_nodes_;

	//	Image env_image_;

	//	Sampler diffuse_sampler_;
	//	Sampler nearest_sampler_;
	//	Sampler shadow_sampler_;

	//	const Scene& scene_;
	//};


	/*class UIPoly : public DescriptorSetsHolder<DescriptorSetType::kModelMatrix, DescriptorSetType::kBitmapAtlas>
	{
		const ui::UI& ui_;
		std::vector<Primitive> primitives_;

		glm::mat4 transform_;

		glm::vec2 atlas_position_;
		glm::vec2 atlas_width_height_;

	public:
		UIPoly(const DeviceConfiguration& device_cfg, const ui::UI& ui, glm::mat4 transform, glm::vec2 atlas_position, glm::vec2 atlas_width_height);

		void FillData(render::DescriptorSet<render::DescriptorSetType::kModelMatrix>::Binding<0>::Data& data) override;
		void FillData(render::DescriptorSet<render::DescriptorSetType::kBitmapAtlas>::Binding<0>::Data& data) override;

		PrimitivesHolderRenderNode GetRenderNode();
	};

	class UIScene : public DescriptorSetsHolder<DescriptorSetType::kTexture>
	{
		const ui::UI& ui_;

		ui::Panel screen_panel_;
		ui::TextBlock text_block_;

		std::vector<UIPoly> ui_polygones_;
		std::vector<std::reference_wrapper<UIPoly>> ui_polygones_geom_;

	public:
		UIScene(const DeviceConfiguration& device_cfg, const ui::UI& ui);

		void FillData(render::DescriptorSet<render::DescriptorSetType::kTexture>::Binding<0>::Data& data) override;

		SceneRenderNode GetRenderNode();

		void UpdateData();

		void AttachDescriptorSets(DescriptorSetsManager& manager);

		std::vector<PrimitivesHolderRenderNode> children_nodes_;
	};*/
}

#endif  // RENDER_ENGINE_RENDER_SCENE_H_