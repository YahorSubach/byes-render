#include "render/render_engine.h"

#include "platform.h"
#include "stl_util.h"


#include <vector>
#include <map>
#include <tuple>
#include <fstream>

#include "vulkan/vulkan.h"
#include "surface.h"

#include <tchar.h>

#include "common.h"
#include "render/vk_util.h"
#include "render/frame_handler.h"

namespace render
{
	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	class RenderEngine::RenderEngineImpl
	{
	public:

		RenderEngineImpl(const RenderEngineImpl&) = delete;
		RenderEngineImpl& operator=(const RenderEngineImpl&) = delete;

		RenderEngineImpl(InitParam param)
		{
			vk_init_success_ = false;

			if (!InitInstanceExtensions())
			{
				LOG(err, "Could not init InstanceExtensions");
				return;
			}

			if (!stl_util::All(platform::GetRequiredExtensions(), vk_instance_extensions_, [](auto&& ext_name, auto&& ext) { return std::strcmp(ext_name, ext.extensionName) == 0; }))
			{
				LOG(err, "Required platform Extension missing");
				return;
			}

			if (!InitInstanceLayers())
			{
				LOG(err, "InitInstanceLayers error");
				return;
			}

			if (!stl_util::All(GetValidationLayers(), vk_instance_layers_, [](auto&& ext_name, auto&& ext) { return std::strcmp(ext_name, ext.layerName) == 0; }))
			{
				LOG(err, "Instance Layers missing");
				return;
			}

			application_info_.apiVersion = VK_API_VERSION_1_0; // CHECK WHAT DOES THIS MEAN!
			application_info_.applicationVersion = 1;
			application_info_.engineVersion = 1;
			application_info_.pApplicationName = "vulkan_concepts";
			application_info_.pEngineName = "vulkan_concepts_engine";
			application_info_.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			application_info_.pNext = nullptr;

			VkInstanceCreateInfo instance_create_info{};
			instance_create_info.enabledExtensionCount = static_cast<uint32_t>(platform::GetRequiredExtensions().size());
			instance_create_info.ppEnabledExtensionNames = platform::GetRequiredExtensions().data();
			instance_create_info.enabledLayerCount = static_cast<uint32_t>(GetValidationLayers().size());
			instance_create_info.ppEnabledLayerNames = GetValidationLayers().data();
			instance_create_info.pApplicationInfo = &application_info_;
			instance_create_info.pNext = nullptr;
			instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;


			if (vkCreateInstance(&instance_create_info, nullptr, &vk_instance_) != VK_SUCCESS)
			{
				LOG(err, "vkCreateInstance error");
				return;
			}


			if (!InitPhysicalDevices())
			{
				LOG(err, "InitPhysicalDevices error");
				return;
			}

			if (!InitLogicalDevices())
			{
				LOG(err, "InitLogicalDevices error");
				return;
			}

			std::pair<uint32_t, uint32_t> device_and_queue_indices = DetermineDeviceAndQueueForUse();

			if (device_and_queue_indices.first < 0 || device_and_queue_indices.second < 0)
			{
				LOG(err, "Could not determine valid device or queue");
				return;
			}

			selected_device_index = device_and_queue_indices.first;
			selected_queue_index = device_and_queue_indices.second;

			vkGetDeviceQueue(vk_logical_devices_[selected_device_index], selected_queue_index, 0, &graphics_queue);

			surface_ptr_ = std::make_unique<Surface>(param, vk_instance_, vk_physical_devices_[device_and_queue_indices.first], selected_queue_index, vk_logical_devices_[selected_device_index]);

			vk_init_success_ = true;
		}


		bool VKInitSuccess()
		{
			return vk_init_success_;
		}

		VkShaderModule createShaderModule(const std::vector<char>& code) 
		{
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = code.size();
			createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

			VkShaderModule shader_module;
			if (vkCreateShaderModule(vk_logical_devices_[selected_device_index], &createInfo, nullptr, &shader_module) != VK_SUCCESS) {
				throw std::runtime_error("failed to create shader module!");
			}

			return shader_module;
		}

		void createRenderPass() {
			VkAttachmentDescription color_attachment{};
			color_attachment.format = surface_ptr_->swapchain_image_format;
			color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

			color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

			color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference color_attachment_ref{};
			color_attachment_ref.attachment = 0;
			color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &color_attachment_ref;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;

			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;

			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


			VkRenderPassCreateInfo render_pass_info{};
			render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			render_pass_info.attachmentCount = 1;
			render_pass_info.pAttachments = &color_attachment;
			render_pass_info.subpassCount = 1;
			render_pass_info.pSubpasses = &subpass;
			render_pass_info.dependencyCount = 1;
			render_pass_info.pDependencies = &dependency;

			if (vkCreateRenderPass(vk_logical_devices_[selected_device_index], &render_pass_info, nullptr, &render_pass) != VK_SUCCESS) {
				throw std::runtime_error("failed to create render pass!");
			}
		}

		void createGraphicsPipeline()
		{
			auto vert_shader_code = readFile("shaders/vert.spv");
			auto frag_shader_code = readFile("shaders/frag.spv");

			VkShaderModule vert_shader_module = createShaderModule(vert_shader_code);
			VkShaderModule frag_shader_module = createShaderModule(frag_shader_code);

			VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
			vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vert_shader_stage_info.module = vert_shader_module;
			vert_shader_stage_info.pName = "main"; // entry point in shader

			VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
			frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			frag_shader_stage_info.module = frag_shader_module;
			frag_shader_stage_info.pName = "main";

			VkPipelineShaderStageCreateInfo shader_stages[] = { vert_shader_stage_info, frag_shader_stage_info };


			VkPipelineVertexInputStateCreateInfo vertex_input_info{};
			vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertex_input_info.vertexBindingDescriptionCount = 0;
			vertex_input_info.pVertexBindingDescriptions = nullptr; // Optional
			vertex_input_info.vertexAttributeDescriptionCount = 0;
			vertex_input_info.pVertexAttributeDescriptions = nullptr; // Optional
			
			VkPipelineInputAssemblyStateCreateInfo input_assembly{};
			input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			input_assembly.primitiveRestartEnable = VK_FALSE;

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)surface_ptr_->swapchain_extent.width;
			viewport.height = (float)surface_ptr_->swapchain_extent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = surface_ptr_->swapchain_extent;

			VkPipelineViewportStateCreateInfo viewport_state{};
			viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewport_state.viewportCount = 1;
			viewport_state.pViewports = &viewport;
			viewport_state.scissorCount = 1;
			viewport_state.pScissors = &scissor;

			VkPipelineRasterizationStateCreateInfo rasterizer{};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE; //then geometry never passes through the rasterizer stage. This basically disables any output to the framebuffer.
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizer.depthBiasEnable = VK_FALSE;
			rasterizer.depthBiasConstantFactor = 0.0f; // Optional
			rasterizer.depthBiasClamp = 0.0f; // Optional
			rasterizer.depthBiasSlopeFactor = 0.0f; // Optional


			VkPipelineMultisampleStateCreateInfo multisampling{};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampling.minSampleShading = 1.0f; // Optional
			multisampling.pSampleMask = nullptr; // Optional
			multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
			multisampling.alphaToOneEnable = VK_FALSE; // Optional


			VkPipelineColorBlendAttachmentState color_blend_attachment{};
			color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			color_blend_attachment.blendEnable = VK_FALSE;
			color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
			color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
			color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
			color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
			color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
			color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional


			VkPipelineColorBlendStateCreateInfo color_blending{};
			color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			color_blending.logicOpEnable = VK_FALSE;
			color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
			color_blending.attachmentCount = 1;
			color_blending.pAttachments = &color_blend_attachment;
			color_blending.blendConstants[0] = 0.0f; // Optional
			color_blending.blendConstants[1] = 0.0f; // Optional
			color_blending.blendConstants[2] = 0.0f; // Optional
			color_blending.blendConstants[3] = 0.0f; // Optional



			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = 0; // Optional
			pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
			pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
			pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

			if (vkCreatePipelineLayout(vk_logical_devices_[selected_device_index], &pipelineLayoutInfo, nullptr, &pipeline_layout) != VK_SUCCESS) {
				throw std::runtime_error("failed to create pipeline layout!");
			}

			VkGraphicsPipelineCreateInfo pipeline_info{};
			pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipeline_info.stageCount = 2;
			pipeline_info.pStages = shader_stages;

			pipeline_info.pVertexInputState = &vertex_input_info;
			pipeline_info.pInputAssemblyState = &input_assembly;
			pipeline_info.pViewportState = &viewport_state;
			pipeline_info.pRasterizationState = &rasterizer;
			pipeline_info.pMultisampleState = &multisampling;
			pipeline_info.pDepthStencilState = nullptr; // Optional
			pipeline_info.pColorBlendState = &color_blending;
			pipeline_info.pDynamicState = nullptr; // Optional

			pipeline_info.layout = pipeline_layout;

			pipeline_info.renderPass = render_pass;
			pipeline_info.subpass = 0;

			pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
			pipeline_info.basePipelineIndex = -1; // Optional

			if (vkCreateGraphicsPipelines(vk_logical_devices_[selected_device_index], VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline) != VK_SUCCESS) {
				throw std::runtime_error("failed to create graphics pipeline!");
			}

			vkDestroyShaderModule(vk_logical_devices_[selected_device_index], frag_shader_module, nullptr);
			vkDestroyShaderModule(vk_logical_devices_[selected_device_index], vert_shader_module, nullptr);
		}

		void createFramebuffers() {

			swapchain_frame_buffers.resize(surface_ptr_->images_views_.size());

			for (size_t i = 0; i < surface_ptr_->images_views_.size(); i++)
			{
				VkImageView attachments[] = {
					surface_ptr_->images_views_[i]
				};

				VkFramebufferCreateInfo framebuffer_info{};
				framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebuffer_info.renderPass = render_pass;
				framebuffer_info.attachmentCount = 1;
				framebuffer_info.pAttachments = attachments;
				framebuffer_info.width = surface_ptr_->swapchain_extent.width;
				framebuffer_info.height = surface_ptr_->swapchain_extent.height;
				framebuffer_info.layers = 1;

				if (vkCreateFramebuffer(vk_logical_devices_[selected_device_index], &framebuffer_info, nullptr, &swapchain_frame_buffers[i]) != VK_SUCCESS) {
					throw std::runtime_error("failed to create framebuffer!");
				}
			}

		}

		void createCommandPool() {
			VkCommandPoolCreateInfo pool_info{};
			pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			pool_info.queueFamilyIndex = selected_queue_index;
			pool_info.flags = 0; // Optional

			if (vkCreateCommandPool(vk_logical_devices_[selected_device_index], &pool_info, nullptr, &command_pool) != VK_SUCCESS) {
				throw std::runtime_error("failed to create command pool!");
			}
		}

		void createCommandBuffers() 
		{
			command_buffers.resize(swapchain_frame_buffers.size());

			VkCommandBufferAllocateInfo alloc_info{};
			alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			alloc_info.commandPool = command_pool;
			alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			alloc_info.commandBufferCount = (uint32_t)command_buffers.size();

			if (vkAllocateCommandBuffers(vk_logical_devices_[selected_device_index], &alloc_info, command_buffers.data()) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate command buffers!");
			}

			for (size_t i = 0; i < command_buffers.size(); i++) {
				VkCommandBufferBeginInfo begin_info{};
				begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Optional
				begin_info.pInheritanceInfo = nullptr; // Optional

				if (vkBeginCommandBuffer(command_buffers[i], &begin_info) != VK_SUCCESS) {
					throw std::runtime_error("failed to begin recording command buffer!");
				}

				VkRenderPassBeginInfo render_pass_info{};
				render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				render_pass_info.renderPass = render_pass;
				render_pass_info.framebuffer = swapchain_frame_buffers[i];

				render_pass_info.renderArea.offset = { 0, 0 };
				render_pass_info.renderArea.extent = surface_ptr_->swapchain_extent;

				VkClearValue clear_color = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
				render_pass_info.clearValueCount = 1;
				render_pass_info.pClearValues = &clear_color;

				vkCmdBeginRenderPass(command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

				vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
				vkCmdDraw(command_buffers[i], 3, 1, 0, 0);

				vkCmdEndRenderPass(command_buffers[i]);

				if (vkEndCommandBuffer(command_buffers[i]) != VK_SUCCESS) {
					throw std::runtime_error("failed to record command buffer!");
				}

			}
		}

		void createSemaphores() {
			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			if (vkCreateSemaphore(vk_logical_devices_[selected_device_index], &semaphoreInfo, nullptr, &image_available_semaphore) != VK_SUCCESS ||
				vkCreateSemaphore(vk_logical_devices_[selected_device_index], &semaphoreInfo, nullptr, &render_finished_semaphore) != VK_SUCCESS) {

				throw std::runtime_error("failed to create semaphores!");
			}
		}

		void cleanup() {
			//vkDestroyPipelineLayout(vk_logical_devices_[selected_device_index], pipelineLayout, nullptr);
		}

		void ShowWindow()
		{
			platform::ShowWindow(surface_ptr_->GetWindow());

			std::vector<FrameHandler> frames;

			frames.reserve(command_buffers.size());

			for (size_t frame_ind = 0; frame_ind < command_buffers.size(); frame_ind++)
			{
				frames.emplace_back(vk_logical_devices_[selected_device_index], graphics_queue, surface_ptr_->swapchain_, frame_ind, command_buffers[frame_ind], render_finished_semaphore);
			}

			while (!platform::IsWindowClosed(surface_ptr_->GetWindow()))
			{
				uint32_t image_index;
				vkAcquireNextImageKHR(vk_logical_devices_[selected_device_index], surface_ptr_->swapchain_, UINT64_MAX, 
					image_available_semaphore, VK_NULL_HANDLE, &image_index);

				frames[image_index].Process(image_available_semaphore);

			//	VkSubmitInfo submit_info{};
			//	/*submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			//	VkSemaphore waitSemaphores[] = { image_available_semaphore };
			//	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			//	submit_info.waitSemaphoreCount = 1;
			//	submit_info.pWaitSemaphores = waitSemaphores;
			//	submit_info.pWaitDstStageMask = waitStages;

			//	submit_info.commandBufferCount = 1;
			//	submit_info.pCommandBuffers = &command_buffers[image_index];

			//	VkSemaphore signal_semaphores[] = { render_finished_semaphore };
			//	submit_info.signalSemaphoreCount = 1;
			//	submit_info.pSignalSemaphores = signal_semaphores;*/

			//	if (vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS) {
			//		throw std::runtime_error("failed to submit draw command buffer!");
			//	}

			//	//uint32_t image_index2;
			//	//vkAcquireNextImageKHR(vk_logical_devices_[selected_device_index], surface_ptr_->swapchain_, UINT64_MAX,
			//	//	image_available_semaphore, VK_NULL_HANDLE, &image_index2);

			//	//VkPresentInfoKHR present_info{};
			//	//present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			//	//present_info.waitSemaphoreCount = 1;
			//	//present_info.pWaitSemaphores = signal_semaphores;

			//	//VkSwapchainKHR swapChains[] = { surface_ptr_->swapchain_ };
			//	//present_info.swapchainCount = 1;
			//	//present_info.pSwapchains = swapChains;
			//	//present_info.pImageIndices = &image_index;

			//	//present_info.pResults = nullptr; // Optional

			//	////uint32_t image_index3;
			//	////vkAcquireNextImageKHR(vk_logical_devices_[selected_device_index], surface_ptr_->swapchain_, UINT64_MAX,
			//	////	image_available_semaphore, VK_NULL_HANDLE, &image_index3);

			//	vkQueuePresentKHR(graphics_queue, &present_info);

			//	//uint32_t image_index4;
			//	//vkAcquireNextImageKHR(vk_logical_devices_[selected_device_index], surface_ptr_->swapchain_, UINT64_MAX,
			//	//	image_available_semaphore, VK_NULL_HANDLE, &image_index4);
			}
		}

		~RenderEngineImpl()
		{
			for (auto it = vk_logical_devices_.begin(); it != vk_logical_devices_.end(); it++)
			{
				vkDeviceWaitIdle(*it);
				vkDestroyDevice(*it, nullptr);
			}
			vkDestroyInstance(vk_instance_, nullptr);
		}

	private:

		const std::vector<const char*>& GetRequiredExtensions()
		{
			static const std::vector<const char*> extensions{ "VK_KHR_swapchain" };
			return extensions;
		}

		const std::vector<const char*>& GetValidationLayers()
		{
#ifndef NDEBUG
			static const std::vector<const char*> layers{ "VK_LAYER_KHRONOS_validation" };
			return layers;
#else
			static const std::vector<const char*> layers{};
			return layers;
#endif // !NDEBUG
		}

		bool InitPhysicalDevices()
		{
			uint32_t physical_devices_count;

			if (vkEnumeratePhysicalDevices(vk_instance_, &physical_devices_count, nullptr) != VK_SUCCESS)
				return false;

			vk_physical_devices_.resize(physical_devices_count);

			if (vkEnumeratePhysicalDevices(vk_instance_, &physical_devices_count, reinterpret_cast<VkPhysicalDevice*>(vk_physical_devices_.data())) != VK_SUCCESS)
				return false;
			//TODO: add device selection
			for (size_t i = 0; i < physical_devices_count; i++)
			{
				InitPhysicalDeviceProperties(vk_physical_devices_[i]);
				InitPhysicalDeviceQueueFamaliesProperties(vk_physical_devices_[i]);
				if (!(InitPhysicalDeviceLayers(vk_physical_devices_[i]) &&
					InitPhysicalDeviceExtensions(vk_physical_devices_[i])))
					return false;
			}
			return true;
		}

		bool InitLogicalDevices()
		{
			//TODO: add device selection
			for (VkPhysicalDevice& physical_device : vk_physical_devices_)
			{
				if (!InitLogicalDevice(physical_device))
					return false;
			}
			return true;
	}

		std::pair<uint32_t, uint32_t> DetermineDeviceAndQueueForUse()
		{
			VkPhysicalDevice physical_device_out = nullptr;

			uint32_t device_queue_famaly_ind = -1;


			for (auto&& [physical_device, queues_properties] : vk_physical_devices_queues_)
			{
				for (uint32_t i = 0; i < queues_properties.size(); i++)
				{
					if (platform::GetPhysicalDevicePresentationSupport(physical_device, i) && (queues_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
					{
						physical_device_out = physical_device;
						device_queue_famaly_ind = i;
						break;
					}
				}

				if (device_queue_famaly_ind != -1)
					break;
			}

			auto physical_device_out_it = std::find(vk_physical_devices_.begin(), vk_physical_devices_.end(), physical_device_out);

			if (physical_device_out_it != vk_physical_devices_.end())
				return { static_cast<uint32_t>(physical_device_out_it - vk_physical_devices_.begin()), device_queue_famaly_ind };

			return { -1, -1 };
		}


		void InitPhysicalDeviceProperties(VkPhysicalDevice physical_device)
		{
			using PhDevPropMapType = std::map<VkPhysicalDevice, VkPhysicalDeviceMemoryProperties>;

			auto it = vk_physical_devices_propeties_.lower_bound(physical_device);

			if (it == vk_physical_devices_propeties_.end() || it->first != physical_device)
			{
				it = vk_physical_devices_propeties_.emplace_hint(it,
					std::piecewise_construct, std::forward_as_tuple(physical_device), std::forward_as_tuple());
				vkGetPhysicalDeviceMemoryProperties(physical_device, &it->second);

			}
		}

		void InitPhysicalDeviceQueueFamaliesProperties(VkPhysicalDevice physical_device)
		{
			using PhDevFamQMapType = std::map<VkPhysicalDevice, std::vector<VkQueueFamilyProperties>>;

			auto it = vk_physical_devices_queues_.lower_bound(physical_device);

			if (it == vk_physical_devices_queues_.end() || it->first != physical_device)
			{
				uint32_t device_queue_famaly_prop_cnt;
				vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &device_queue_famaly_prop_cnt, nullptr);

				it = vk_physical_devices_queues_.emplace_hint(it,
					std::piecewise_construct, std::forward_as_tuple(physical_device), std::forward_as_tuple());

				it->second.resize(device_queue_famaly_prop_cnt);

				vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &device_queue_famaly_prop_cnt, it->second.data());
			}
		}


		void InitPhysicalDeviceFeatures(const VkPhysicalDevice& physical_device)
		{

			using PhDevPropMapType = std::map<VkPhysicalDevice, VkPhysicalDeviceFeatures>;

			auto it = vk_physical_devices_features_.lower_bound(physical_device);

			if (it == vk_physical_devices_features_.end() || it->first != physical_device)
			{
				it = vk_physical_devices_features_.emplace_hint(it,
					std::piecewise_construct, std::forward_as_tuple(physical_device), std::forward_as_tuple());
				vkGetPhysicalDeviceFeatures(physical_device, &it->second);

			}
		}

		bool InitInstanceLayers()
		{
			uint32_t count;
			if (vkEnumerateInstanceLayerProperties(&count, nullptr) == VK_SUCCESS)
			{
				vk_instance_layers_.resize(count);
				if (vkEnumerateInstanceLayerProperties(&count, vk_instance_layers_.data()) == VK_SUCCESS)
					return true;
			}
			return false;
		}

		bool InitPhysicalDeviceLayers(const VkPhysicalDevice& physical_device)
		{
			using PhDevFamQMapType = std::map<VkPhysicalDevice, std::vector<VkLayerProperties>>;

			auto it = vk_physical_devices_layers_.lower_bound(physical_device);

			if (it == vk_physical_devices_layers_.end() || it->first != physical_device)
			{
				uint32_t device_layers_cnt;
				if (vkEnumerateDeviceLayerProperties(physical_device, &device_layers_cnt, nullptr) != VK_SUCCESS)
					return false;

				it = vk_physical_devices_layers_.emplace_hint(it,
					std::piecewise_construct, std::forward_as_tuple(physical_device), std::forward_as_tuple());

				it->second.resize(device_layers_cnt);

				if (vkEnumerateDeviceLayerProperties(physical_device, &device_layers_cnt, it->second.data()) != VK_SUCCESS)
					return false;
			}
			return true;
		}

		bool InitInstanceExtensions()
		{
			uint32_t count;
			if (vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr) == VK_SUCCESS)
			{
				vk_instance_extensions_.resize(count);
				if (vkEnumerateInstanceExtensionProperties(nullptr, &count, vk_instance_extensions_.data()) == VK_SUCCESS)
					return true;
			}
			return false;
		}

		bool InitPhysicalDeviceExtensions(const VkPhysicalDevice& physical_device)
		{
			using PhDevFamQMapType = std::map<VkPhysicalDevice, std::vector<VkExtensionProperties>>;

			auto it = vk_physical_devices_extensions_.lower_bound(physical_device);

			if (it == vk_physical_devices_extensions_.end() || it->first != physical_device)
			{
				uint32_t device_extensions_cnt;
				if (vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extensions_cnt, nullptr) != VK_SUCCESS)
					return false;

				it = vk_physical_devices_extensions_.emplace_hint(it,
					std::piecewise_construct, std::forward_as_tuple(physical_device), std::forward_as_tuple(device_extensions_cnt));

				if (vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extensions_cnt, it->second.data()) != VK_SUCCESS)
					return false;
			}
			return true;
		}

		bool InitLogicalDevice(const VkPhysicalDevice& physical_device)
		{
			VkDeviceCreateInfo logical_device_create_info{};

			if (stl_util::All(GetRequiredExtensions(), vk_physical_devices_extensions_[physical_device], [](auto&& ext_name, auto&& ext) { return std::strcmp(ext_name, ext.extensionName) == 0; }))
			{
				logical_device_create_info.enabledExtensionCount = static_cast<uint32_t>(GetRequiredExtensions().size());
				logical_device_create_info.ppEnabledExtensionNames = GetRequiredExtensions().data();
			}
			else return false;



			logical_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			logical_device_create_info.pNext = nullptr;
			logical_device_create_info.flags = 0;
			logical_device_create_info.enabledLayerCount = 0;
			logical_device_create_info.ppEnabledLayerNames = nullptr;
			logical_device_create_info.pEnabledFeatures = &vk_physical_devices_features_[physical_device];


			//TODO: select and create only graphic queue
			VkDeviceQueueCreateInfo queueu_create_info{};
			queueu_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueu_create_info.flags = 0;
			queueu_create_info.queueFamilyIndex = 0;

			uint32_t device_queue_count = vk_physical_devices_queues_[physical_device][0].queueCount;

			queueu_create_info.queueCount = device_queue_count;

			std::vector<float> queue_priorities(device_queue_count);
			for (auto&& priority : queue_priorities) priority = 1.0f;

			queueu_create_info.pQueuePriorities = queue_priorities.data();
			queueu_create_info.pNext = nullptr;

			logical_device_create_info.queueCreateInfoCount = 1;
			logical_device_create_info.pQueueCreateInfos = &queueu_create_info;

			vk_logical_devices_.emplace_back();

			if (vkCreateDevice(physical_device, &logical_device_create_info, nullptr, &vk_logical_devices_[vk_logical_devices_.size() - 1]) != VK_SUCCESS)
				return false;
			return true;
		}

		bool vk_init_success_;
		VkApplicationInfo application_info_;
		VkInstance vk_instance_;



		std::vector<VkPhysicalDevice> vk_physical_devices_;
		std::map<VkPhysicalDevice, VkPhysicalDeviceMemoryProperties> vk_physical_devices_propeties_;
		std::map<VkPhysicalDevice, std::vector<VkQueueFamilyProperties>> vk_physical_devices_queues_;
		std::map<VkPhysicalDevice, VkPhysicalDeviceFeatures> vk_physical_devices_features_;
		std::vector<VkLayerProperties > vk_instance_layers_;
		std::map<VkPhysicalDevice, std::vector<VkLayerProperties>> vk_physical_devices_layers_;
		std::vector<VkExtensionProperties> vk_instance_extensions_;
		std::map<VkPhysicalDevice, std::vector<VkExtensionProperties>> vk_physical_devices_extensions_;
		
		std::vector<VkFramebuffer> swapchain_frame_buffers;


		size_t selected_queue_index;
		size_t selected_device_index;

		std::vector<VkDevice> vk_logical_devices_;
		std::unique_ptr<Surface> surface_ptr_;
		VkRenderPass render_pass;
		VkPipelineLayout pipeline_layout;
		VkPipeline graphics_pipeline;
		VkCommandPool command_pool;
		std::vector<VkCommandBuffer> command_buffers;

		VkSemaphore image_available_semaphore;
		VkSemaphore render_finished_semaphore;

		VkQueue graphics_queue;

};

	RenderEngine::RenderEngine(InitParam param)
	{
		impl_ = std::make_unique<RenderEngineImpl>(param);
	}

	RenderEngine::~RenderEngine() = default;

	bool RenderEngine::VKInitSuccess()
	{
		return impl_->VKInitSuccess();
	}


	void RenderEngine::CreateGraphicsPipeline()
	{
		impl_->createRenderPass();
		impl_->createGraphicsPipeline();
		impl_->createFramebuffers();
		impl_->createCommandPool();
		impl_->createCommandBuffers();
		impl_->createCommandBuffers();
		impl_->createSemaphores();
	}

	void RenderEngine::ShowWindow()
	{
		impl_->ShowWindow();
	}
}