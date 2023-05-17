#ifndef RENDER_ENGINE_RENDER_BATCHES_MANAGER_H_
#define RENDER_ENGINE_RENDER_BATCHES_MANAGER_H_

#include "vulkan/vulkan.h"

#include <vector>

#include "common.h"
#include "render/animator.h"
#include "render/batch.h"
#include "render/descriptor_pool.h"
#include "render/gltf_wrapper.h"
#include "render/image.h"
#include "render/image_view.h"
#include "render/render_pass.h"
#include "render/object_base.h"
#include "render/sampler.h"
#include "render/swapchain.h"
#include "stl_util.h"
#include "gltf_wrapper.h"


namespace render
{
	class BatchesManager: RenderObjBase<void*>
	{
	public:

		BatchesManager(const Global& global);

		BatchesManager(const BatchesManager&) = delete;
		BatchesManager(BatchesManager&&) = default;

		BatchesManager& operator=(const BatchesManager&) = delete;
		BatchesManager& operator=(BatchesManager&&) = default;

		std::vector<ModelPack>& GetModelPacks();

		const ImageView& GetEnvImageView() const;

		void Update();
		void Add(const tinygltf::Model& model, DescriptorSetsManager& manager, const RenderSetup& render_setup);

	private:

		std::unique_ptr<DescriptorPool> descriptor_pool_ptr_;

		std::vector<std::reference_wrapper<Mesh>> meshes_;

		std::vector<GPULocalBuffer> buffers_;
		std::vector<Image> images_;
		std::vector<ImageView> image_views_;
		
		std::vector<GLTFWrapper> gltf_wrappers_;

		std::vector<Animator> animators_;

	};
}

#endif  // RENDER_ENGINE_RENDER_BATCHES_MANAGER_H_