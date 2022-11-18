//#ifndef RENDER_ENGINE_RENDER_COMMAND_BUFFER_FILLER_H_
//#define RENDER_ENGINE_RENDER_COMMAND_BUFFER_FILLER_H_
//
//#include <vector>
//#include <map>
//#include <functional>
//
//#include "vulkan/vulkan.h"
//
//#include "render/batches_manager.h"
//#include "render/framebuffer_collection.h"
//#include "render/descriptor_set.h"
//#include "render/render_setup.h"
//#include "render/render_pass.h"
//#include "render/scene.h"
//
//namespace render
//{
//
//	struct PipelineInfo
//	{
//		PipelineId id;
//		RenderModelType primitive_type;
//		SceneRenderNode& scene;
//	};
//
//	struct RenderPassInfo
//	{
//		RenderPassId render_pass_id;
//		FramebufferId framebuffer_id;
//
//		std::vector<PipelineInfo> pipelines;
//	};
//
//
//	class CommandBufferFiller
//	{
//	public:
//
//		CommandBufferFiller(const RenderSetup& render_setup, const FramebufferCollection& framebuffer_collection);
//
//		void Fill(VkCommandBuffer command_buffer, std::vector<RenderPassInfo> render_info, const Framebuffer& screen_buffer);
//
//	private:
//
//		void ProcessDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout, const std::map<uint32_t, const DescriptorSetLayout&>& pipeline_desc_sets, const std::map<DescriptorSetType, VkDescriptorSet>& holder_desc_sets) const;
//
//		const RenderSetup& render_setup_;
//		const FramebufferCollection& framebuffer_collection_;
//		//std::map<RenderPassId, RenderPass> render_passes_;
//		//std::map<PipelineId, GraphicsPipeline> pipelines_;
//	};
//
//}
//#endif  // RENDER_ENGINE_RENDER_COMMAND_BUFFER_FILLER_H_