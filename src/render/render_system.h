#ifndef RENDER_ENGINE_RENDER_RENDER_SYSTEM_H_
#define RENDER_ENGINE_RENDER_RENDER_SYSTEM_H_

#include <vector>
#include <map>
#include <memory>


#include "vulkan/vulkan.h"

#include "stl_util.h"
#include "render\global.h"
#include "surface.h"
#include "command_pool.h"
#include "descriptor_sets_manager.h"
#include "frame_handler.h"
#include "render_api.h"


namespace render
{
	

	class RenderSystem
	{
	public:
		RenderSystem(platform::Window window, const std::string& app_name);
		
		bool ShouldRender() const;
		void Render(uint32_t frame_index, const Scene& scene);

		const Global& GetGlobal() const;
		DescriptorSetsManager& GetDescriptorSetsManager();

		void AddOnSwapchainUpdateCallback(std::function<void(const Swapchain&)> callback);

	private:
		
		std::vector<std::function<void(const Swapchain&)>> on_swapchain_update_callbacks;

		Global global_;
		RenderApi render_api_;
		Surface surface_;

		std::optional<DescriptorSetsManager> descriptor_set_manager_;

		std::optional<Swapchain> swapchain_;

		std::array<std::optional<FrameHandler>, kFramesCount> frames_;
		std::array<std::optional<Framebuffer>, kFramesCount> swapchain_framebuffers_;

		Extents extents_;
		Formats formats_;

		RenderSetup render_setup_;
	};

}
#endif  // RENDER_ENGINE_RENDER_RENDER_SYSTEM_H_