#ifndef RENDER_ENGINE_RENDER_FRAME_HANDLER_H_
#define RENDER_ENGINE_RENDER_FRAME_HANDLER_H_

#include "vulkan/vulkan.h"

#include "object_base.h"

#include "render/buffer.h"
#include "render/swapchain.h"
#include "render/render_setup.h"
#include "render/batches_manager.h"
#include "render/render_graph.h"
#include "render/sampler.h"
#include "render/scene.h"

namespace render
{

	class DebugGeometry
	{
	public:

		struct Point
		{
			glm::vec3 position;
			glm::vec3 color;

			std::pair<Point, Point> operator>>(const Point&);
		};

		using Line = std::pair<Point, Point>;
		
		DebugGeometry(const Global& global);

		void Update();

		void SetDebugLines(const std::vector<Line>& lines);

		GPULocalVertexBuffer coords_lines_position_buffer_;
		GPULocalVertexBuffer coords_lines_color_buffer_;
		unsigned int coords_lines_vertex_cnt;


		GPULocalVertexBuffer debug_lines_position_buffer_;
		GPULocalVertexBuffer debug_lines_color_buffer_;
		unsigned int debug_lines_vertex_cnt;

		std::atomic_bool ready_to_write;
		std::atomic_bool ready_to_read;

		std::vector<glm::vec3> debug_lines_position_data_;
		std::vector<glm::vec3> debug_lines_color_data_;
	};


	class FrameHandler: public RenderObjBase<void*>
	{
	public:

		FrameHandler(const Global& global, const Swapchain& swapchain, const RenderSetup& render_setup, 
			const std::array<Extent, kExtentTypeCnt>& extents, DescriptorSetsManager& descriptor_set_manager, 
			const BatchesManager& batches_manager, const ui::UI& ui, const Scene& scene, DebugGeometry& debug_geometry);
		
		FrameHandler(const FrameHandler&) = delete;
		FrameHandler(FrameHandler&&) = default;

		FrameHandler& operator=(const FrameHandler&) = delete;
		FrameHandler& operator=(FrameHandler&&) = default;
		
		//void AddModel(const render::Mesh& model);



		bool Draw(const FrameInfo& frame_info, Scene::SceneImpl& scene);

		VkSemaphore GetImageAvailableSemaphore() const;

		virtual ~FrameHandler() override;


	private:
		VkSwapchainKHR swapchain_;
		VkCommandBuffer command_buffer_;
		
		VkPipelineStageFlags wait_stages_;
		
		VkSemaphore render_finished_semaphore_;
		VkSemaphore image_available_semaphore_;

		VkFence cmd_buffer_fence_;

		VkSubmitInfo submit_info_;
		VkPresentInfoKHR present_info_;

		VkQueue graphics_queue_;

		const ui::UI& ui_;

		const RenderSetup& render_setup_;

		//ModelSceneDescSetHolder model_scene_;
		RenderGraphHandler render_graph_handler_;
		//UIScene ui_scene_;

		DebugGeometry& debug_geometry_;
	};
}

#endif  // RENDER_ENGINE_RENDER_FRAME_HANDLER_H_