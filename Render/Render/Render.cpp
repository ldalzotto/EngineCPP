#pragma once

#include <interface/Render/render.hpp>
#include <driver/Render/rdwindow.hpp>
#include "Math/math.hpp"
#include "Common/Container/pool.hpp"
#include "Common/Container/vector.hpp"
#include <fstream>
#include <stdlib.h>

using namespace Math;

#define DEFAULT_FENCE_TIMEOUT 2000

struct Window
{
	WindowHandle Handle;
	short int Width;
	short int Height;

	Window() = default;
	inline Window(const short int p_width, const short int p_height, const std::string& p_title)
	{
		this->Handle = rdwindow::create_window(p_width, p_height, p_title);
		this->Width = p_width;
		this->Height = p_height;
	};
	inline void dispose()
	{
		rdwindow::free_window(this->Handle);
	};
};

struct RenderPass
{
	vk::RenderPass l_render_pass;

	void create(const vk::Device& p_device, vk::SurfaceFormatKHR p_surface_format)
	{
		com::Vector<vk::AttachmentDescription> l_attachments(1);
		l_attachments.Size = 1;

		vk::AttachmentDescription& l_color_attachment = l_attachments[0];
		l_color_attachment = vk::AttachmentDescription();
		l_color_attachment.setFormat(p_surface_format.format);
		l_color_attachment.setSamples(vk::SampleCountFlagBits::e1);
		l_color_attachment.setLoadOp(vk::AttachmentLoadOp::eClear);
		l_color_attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
		l_color_attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
		l_color_attachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
		l_color_attachment.setInitialLayout(vk::ImageLayout::eUndefined);
		l_color_attachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

		com::Vector<vk::SubpassDescription> l_subpasses(1);
		l_subpasses.Size = 1;

		vk::AttachmentReference l_color_attachment_ref;
		l_color_attachment_ref.setAttachment(0);
		l_color_attachment_ref.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		vk::SubpassDescription& l_color_subpass = l_subpasses[0];
		l_color_subpass = vk::SubpassDescription();
		l_color_subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
		l_color_subpass.setColorAttachmentCount(1);
		l_color_subpass.setPColorAttachments(&l_color_attachment_ref);

		vk::RenderPassCreateInfo l_renderpass_create_info;
		l_renderpass_create_info.setAttachmentCount((uint32_t)l_attachments.Size);
		l_renderpass_create_info.setPAttachments(l_attachments.Memory);
		l_renderpass_create_info.setSubpassCount((uint32_t)l_subpasses.Size);
		l_renderpass_create_info.setPSubpasses(l_subpasses.Memory);

		this->l_render_pass = p_device.createRenderPass(l_renderpass_create_info);
	}

	void dispose(const vk::Device& p_device)
	{
		p_device.destroyRenderPass(this->l_render_pass);
	}
};

struct CommandBuffer
{
	vk::CommandBuffer command_buffer = nullptr;
	const vk::CommandPool* pool = nullptr;
	const vk::Queue* queue = nullptr;
	bool hasBegun = false;

	inline CommandBuffer(const vk::Device& p_device, const vk::CommandPool& p_command_pool, const vk::Queue& p_queue)
	{
		vk::CommandBufferAllocateInfo l_command_buffer_allocate_info;
		this->pool = &p_command_pool;
		this->queue = &p_queue;
		l_command_buffer_allocate_info.setCommandPool(*this->pool);
		l_command_buffer_allocate_info.setLevel(vk::CommandBufferLevel::ePrimary);
		l_command_buffer_allocate_info.setCommandBufferCount(1);
		auto l_command_buffers = p_device.allocateCommandBuffers(l_command_buffer_allocate_info);
		this->command_buffer = l_command_buffers[0];
	}

	inline void begin()
	{
		if (!this->hasBegun)
		{
			vk::CommandBufferBeginInfo l_command_buffer_begin_info;
			this->command_buffer.begin(l_command_buffer_begin_info);
			this->hasBegun = true;
		}
	}

	inline void beginRenderPass(const RenderPass& p_renderpass, const com::Vector<vk::ClearValue>& p_clear_values, const vk::Rect2D& p_rendered_screen)
	{
		vk::RenderPassBeginInfo l_renderpass_begin;
		l_renderpass_begin.setPNext(nullptr);
		l_renderpass_begin.setRenderPass(p_renderpass.l_render_pass);
		l_renderpass_begin.setRenderArea(p_rendered_screen);
		l_renderpass_begin.setClearValueCount((uint32_t)p_clear_values.Size);
		l_renderpass_begin.setPClearValues(p_clear_values.Memory);

		this->command_buffer.beginRenderPass(l_renderpass_begin, vk::SubpassContents::eInline);
	}

	inline void end()
	{
		if (this->hasBegun)
		{
			this->command_buffer.end();
			this->hasBegun = false;
		}
	}

	inline void flush(const vk::Device& p_device)
	{
		this->end();

		vk::Fence l_command_buffer_end_fence = p_device.createFence(vk::FenceCreateInfo());

		vk::SubmitInfo l_wait_for_end_submit;
		l_wait_for_end_submit.setCommandBufferCount(1);
		l_wait_for_end_submit.setPCommandBuffers(&this->command_buffer);
		this->queue->submit(1, &l_wait_for_end_submit, l_command_buffer_end_fence);

		p_device.waitForFences(1, &l_command_buffer_end_fence, true, DEFAULT_FENCE_TIMEOUT);

		p_device.destroyFence(l_command_buffer_end_fence);
		p_device.freeCommandBuffers(*this->pool, 1, &this->command_buffer);

	}
};


struct SwapChain
{
public:

	typedef struct _SwapChainBuffers {
		VkImage image;
		VkImageView view;
	} SwapChainBuffer;

	vk::SwapchainKHR handle;
	vk::SurfaceFormatKHR surface_format;
	vk::PresentModeKHR present_mode;
	vk::Extent2D extend;
	uint32_t image_count;
	std::vector<vk::Image> images;
	com::Vector<SwapChainBuffer> buffers;

	RenderPass renderpass;

	com::Vector<vk::Framebuffer> framebuffers;

private:
	const vk::Instance* instance;
	const vk::PhysicalDevice* physicalDevice;
	const vk::Device* device;
	const vk::SurfaceKHR* surface;
	const Window* window;

	vk::SurfaceCapabilitiesKHR surface_capabilities;

public:
	inline SwapChain() = default;

	inline void init(const vk::Instance& p_instance, const vk::Device& p_device,
		const vk::PhysicalDevice& p_physical_device,
		const vk::SurfaceKHR& p_surface, const Window& p_window)
	{
		this->instance = &p_instance;
		this->device = &p_device;
		this->window = &p_window;
		this->physicalDevice = &p_physical_device;
		this->surface = &p_surface;

		this->pick_surface_format();
		this->pick_presentation_mode();
		this->pick_extent();
		this->pick_image_count();

		vk::SwapchainCreateInfoKHR l_swapchain_create_info;
		l_swapchain_create_info.setSurface(*this->surface);
		l_swapchain_create_info.setMinImageCount(this->image_count);
		l_swapchain_create_info.setImageFormat(this->surface_format.format);
		l_swapchain_create_info.setImageColorSpace(this->surface_format.colorSpace);
		l_swapchain_create_info.setImageExtent(this->extend);
		l_swapchain_create_info.setImageArrayLayers(1);
		l_swapchain_create_info.setImageUsage(vk::ImageUsageFlags(vk::ImageUsageFlagBits::eColorAttachment));
		l_swapchain_create_info.setImageSharingMode(vk::SharingMode::eExclusive);
		l_swapchain_create_info.setQueueFamilyIndexCount(0);
		l_swapchain_create_info.setPQueueFamilyIndices(nullptr);
		l_swapchain_create_info.setPreTransform(this->surface_capabilities.currentTransform);
		l_swapchain_create_info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
		l_swapchain_create_info.setPresentMode(this->present_mode);
		l_swapchain_create_info.setClipped(true);
		l_swapchain_create_info.setOldSwapchain(nullptr);
		this->handle = this->device->createSwapchainKHR(l_swapchain_create_info);

		this->create_images();
		this->createRenderPass(p_device);
		this->create_framebuffers(p_device);
	}

	inline void dispose()
	{
		this->destroy_framebuffers();
		this->destroyRenderPass();
		this->destroy_images();
		this->device->destroySwapchainKHR(this->handle);
	}

	inline uint32_t getNextImage(vk::Semaphore p_presentcomplete_semaphore)
	{
		return this->device->acquireNextImageKHR(this->handle, UINT64_MAX, p_presentcomplete_semaphore, nullptr).value;
	}

	inline void presentImage(vk::Queue p_queue, const uint32_t p_imageindex, vk::Semaphore p_wait_semaphore)
	{
		vk::PresentInfoKHR l_present_info;
		l_present_info.setPNext(nullptr);
		l_present_info.setSwapchainCount(1);
		l_present_info.setPSwapchains(&this->handle);
		l_present_info.setPImageIndices(&p_imageindex);
		l_present_info.setWaitSemaphoreCount(1);
		l_present_info.setPWaitSemaphores(&p_wait_semaphore);
		p_queue.presentKHR(l_present_info);
	}
private:

	inline void pick_surface_format()
	{
		auto l_surface_formats = this->physicalDevice->getSurfaceFormatsKHR(*this->surface);
		for (int i = 0; i < l_surface_formats.size(); i++)
		{
			VkSurfaceFormatKHR& l_surface_format = l_surface_formats[i];
			if (l_surface_format.format == VkFormat::VK_FORMAT_B8G8R8A8_SRGB && l_surface_format.colorSpace == VkColorSpaceKHR::VK_COLORSPACE_SRGB_NONLINEAR_KHR)
			{
				this->surface_format = l_surface_format;
				return;
			}
		}
	}

	inline void pick_presentation_mode()
	{
		auto l_present_modes = this->physicalDevice->getSurfacePresentModesKHR(*this->surface);
		for (int i = 0; i < l_present_modes.size(); i++)
		{
			vk::PresentModeKHR& l_present_mode = l_present_modes[i];
			if (l_present_mode == vk::PresentModeKHR::eMailbox)
			{
				this->present_mode = l_present_mode;
				return;
			}
		}

		this->present_mode = vk::PresentModeKHR::eFifo;
	}

	inline void pick_extent()
	{
		this->surface_capabilities = this->physicalDevice->getSurfaceCapabilitiesKHR(*this->surface);
		if (this->surface_capabilities.currentExtent.width != UINT32_MAX)
		{
			this->extend = this->surface_capabilities.currentExtent;
			return;
		}
		else
		{
			this->extend.setWidth(this->window->Width);
			this->extend.setHeight(this->window->Height);

			if (this->extend.width < this->surface_capabilities.minImageExtent.width)
			{
				this->extend.width = this->surface_capabilities.minImageExtent.width;
			}
			if (this->extend.width > this->surface_capabilities.maxImageExtent.width)
			{
				this->extend.width = this->surface_capabilities.maxImageExtent.width;
			}
			if (this->extend.height < this->surface_capabilities.minImageExtent.height)
			{
				this->extend.height = this->surface_capabilities.minImageExtent.height;
			}
			if (this->extend.height > this->surface_capabilities.maxImageExtent.height)
			{
				this->extend.height = this->surface_capabilities.maxImageExtent.height;
			}
		}
	}

	inline void pick_image_count()
	{
		this->image_count = this->surface_capabilities.minImageCount + 1;
		if ((this->surface_capabilities.maxImageCount > 0) && (this->image_count > this->surface_capabilities.maxImageCount))
		{
			this->image_count = this->surface_capabilities.maxImageCount;
		}
	}

	inline void create_images()
	{
		this->images = this->device->getSwapchainImagesKHR(this->handle);
		this->buffers.resize(this->images.size());
		for (int i = 0; i < this->images.size(); i++)
		{
			vk::Image& l_image = this->images[i];

			vk::ImageViewCreateInfo l_image_view_create_info;
			l_image_view_create_info.setImage(l_image);
			l_image_view_create_info.setViewType(vk::ImageViewType::e2D);
			l_image_view_create_info.setFormat(this->surface_format.format);
			l_image_view_create_info.setComponents(vk::ComponentMapping());

			vk::ImageSubresourceRange l_image_subresource;
			l_image_subresource.setBaseMipLevel(0);
			l_image_subresource.setLevelCount(1);
			l_image_subresource.setBaseArrayLayer(0);
			l_image_subresource.setLayerCount(1);
			l_image_subresource.setAspectMask(vk::ImageAspectFlags(vk::ImageAspectFlagBits::eColor));
			l_image_view_create_info.setSubresourceRange(l_image_subresource);

			SwapChainBuffer l_swapchainBuffer;
			l_swapchainBuffer.image = l_image;
			l_swapchainBuffer.view = this->device->createImageView(l_image_view_create_info);

			this->buffers.push_back(l_swapchainBuffer);
		}
	}

	inline void destroy_images()
	{
		for (int i = 0; i < this->buffers.Size; i++)
		{
			this->device->destroyImageView(this->buffers[i].view);
		}
	}

	inline void createRenderPass(const vk::Device& p_device)
	{
		this->renderpass.create(p_device, this->surface_format);
	}

	inline void destroyRenderPass()
	{
		this->renderpass.dispose(*this->device);
	}

	inline void create_framebuffers(const vk::Device& p_device)
	{
		this->framebuffers = com::Vector<vk::Framebuffer>(this->image_count);
		this->framebuffers.Size = this->framebuffers.Capacity;
		for (size_t i = 0; i < this->framebuffers.Size; i++)
		{
			com::Vector<vk::ImageView> l_attachments(1);
			l_attachments.Size = l_attachments.Capacity;
			l_attachments[0] = this->buffers[i].view;

			vk::FramebufferCreateInfo l_framebuffer_create;
			l_framebuffer_create.setRenderPass(this->renderpass.l_render_pass);
			l_framebuffer_create.setAttachmentCount((uint32_t)l_attachments.Size);
			l_framebuffer_create.setPAttachments(l_attachments.Memory);
			l_framebuffer_create.setWidth(this->window->Width);
			l_framebuffer_create.setHeight(this->window->Height);
			l_framebuffer_create.setLayers(1);

			this->framebuffers[i] = p_device.createFramebuffer(l_framebuffer_create);
		}
	}

	inline void destroy_framebuffers()
	{
		for (size_t i = 0; i < this->framebuffers.Size; i++)
		{
			this->device->destroyFramebuffer(this->framebuffers[i]);
		}
	}

};

struct RenderAPI
{
	static const uint32_t QueueFamilyDefault = -1;

	vk::Instance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	vk::PhysicalDevice graphics_device;
	VkPhysicalDeviceMemoryProperties device_memory_properties;

	vk::Device device;

	vk::SurfaceKHR surface;

	vk::Queue graphics_queue;
	uint32_t graphics_queue_family = QueueFamilyDefault;

	vk::Queue present_queue;
	uint32_t present_queue_family = QueueFamilyDefault;

	SwapChain swap_chain;

	vk::CommandPool command_pool;
	vk::DescriptorPool descriptor_pool;

	com::Vector<CommandBuffer> draw_commandbuffers;

	struct Sync
	{
		vk::Semaphore present_complete_semaphore;
		vk::Semaphore render_complete_semaphore;
		com::Vector<vk::Fence> draw_command_fences;
	};

	Sync synchronization;

	bool validationLayers_enabled;
	com::Vector<const char*> validation_layers;

	RenderAPI() = default;

	inline void init(const Window& p_window)
	{
		this->createInstance();
		this->createDebugCallback();
		this->createSurface(p_window);
		this->getPhysicalDevice();
		this->createPhysicalDevice();
		this->createCommandBufferPool();
		this->createSwapChain(p_window);
		this->create_draw_commandbuffers();
		this->create_synchronization();
		this->create_descriptor_pool();
	};

	inline void dispose()
	{
		this->destroy_descriptor_pool();
		this->destroy_synchronization();
		this->destroy_draw_commandbuffers();
		this->destroySwapChain();
		this->destroyCommandBufferPool();
		this->destroySurface();
		this->device.destroy();
		this->destroyDebugCallback();
		this->destroyInstance();
	}

	uint32_t getMemoryTypeIndex(uint32_t p_typeBits, vk::MemoryPropertyFlags p_properties) const
	{
		for (uint32_t i = 0; i < this->device_memory_properties.memoryTypeCount; i++)
		{
			if ((p_typeBits & 1) == 1)
			{

				if ((vk::MemoryPropertyFlags(this->device_memory_properties.memoryTypes[i].propertyFlags) & p_properties) == p_properties)
				{
					return i;
				}
			}
			p_typeBits >>= 1;
		}

		return -1;
	}

private:
	inline void createInstance()
	{
		vk::ApplicationInfo l_app_info;
		l_app_info.setPApplicationName("test");

		vk::InstanceCreateInfo l_instance_create_info{};
		l_instance_create_info.setPApplicationInfo(&l_app_info);

		com::Vector<const char*> l_extensions;
		this->validationLayers(l_instance_create_info, this->validation_layers);
		this->extensions(l_instance_create_info, l_extensions);
		this->instance = vk::createInstance(l_instance_create_info);
	};

	inline void destroyInstance()
	{
		this->instance.destroy();
	};

	inline void validationLayers(vk::InstanceCreateInfo& p_instance_create_info, com::Vector<const char*>& p_validationLayers)
	{
#if !NDEBUG
		this->validationLayers_enabled = true;
#else
		this->validationLayers_enabled = false;
#endif
		if (this->validationLayers_enabled)
		{
			p_validationLayers = com::Vector<const char*>(1);
			p_validationLayers.Size = 1;
			p_validationLayers.Memory[0] = "VK_LAYER_KHRONOS_validation";

			auto l_available_layers = vk::enumerateInstanceLayerProperties();

			for (int i = 0; i < p_validationLayers.Size; i++)
			{
				bool l_layer_match = false;
				for (int j = 0; j < l_available_layers.size(); j++)
				{
					if (strcmp(p_validationLayers[i], l_available_layers[j].layerName) == 0)
					{
						l_layer_match = true;
						break;
					}
				}
				if (!l_layer_match)
				{
					printf("validation layers requested, but not available!");
					abort();
					return;
				}
			}

			p_instance_create_info.setEnabledLayerCount((uint32_t)p_validationLayers.Size);
			p_instance_create_info.setPpEnabledLayerNames(p_validationLayers.Memory);
		}
		else
		{
			p_instance_create_info.setEnabledLayerCount(0);
		}
	}

	inline void extensions(vk::InstanceCreateInfo& p_instance_create_info, com::Vector<const char*>& p_extensions)
	{
		uint32_t l_glfw_extension_count = 0;
		const char** l_glfw_extensions;
		l_glfw_extensions = glfwGetRequiredInstanceExtensions(&l_glfw_extension_count);

		p_extensions = com::Vector<const char*>(l_glfw_extension_count);
		p_extensions.insert_at(com::MemorySlice<const char*>(*l_glfw_extensions, (size_t)l_glfw_extension_count), 0);

		if (this->validationLayers_enabled)
		{
			p_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		p_instance_create_info.setEnabledExtensionCount((uint32_t)p_extensions.Size);
		p_instance_create_info.setPpEnabledExtensionNames(p_extensions.Memory);
	}

	inline void createDebugCallback()
	{
		if (this->validationLayers_enabled)
		{
			VkDebugUtilsMessengerCreateInfoEXT l_createinfo = {};
			l_createinfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			l_createinfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			l_createinfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			l_createinfo.pfnUserCallback = debugCallback;

			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr((VkInstance)instance, "vkCreateDebugUtilsMessengerEXT");
			if (func != nullptr)
			{
				func((VkInstance)this->instance, &l_createinfo, nullptr, &this->debugMessenger);
			}
		}
	};

	inline void destroyDebugCallback()
	{
		if (this->validationLayers_enabled)
		{
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr((VkInstance)instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr)
			{
				func((VkInstance)this->instance, this->debugMessenger, nullptr);
			}
		}
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{

		std::string l_severity = "";

		switch (messageSeverity)
		{
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			l_severity = "[Verbose] - ";
			break;
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			l_severity = "[Warn] - ";
			break;
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			l_severity = "[Error] - ";
			break;
		}
		printf(l_severity.append("validation layer: ").append(pCallbackData->pMessage).append("\n").c_str());

		return VK_FALSE;
	}

	inline void getPhysicalDevice()
	{
		auto l_physical_devices = this->instance.enumeratePhysicalDevices();
		for (int i = 0; i < l_physical_devices.size(); i++)
		{
			bool l_device_match = false;
			vk::PhysicalDevice& l_physical_device = l_physical_devices[i];
			uint32_t l_graphics_queue_family = QueueFamilyDefault;
			uint32_t l_present_queue_family = QueueFamilyDefault;

			if (l_physical_device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu && l_physical_device.getFeatures().geometryShader)
			{
				auto l_device_extensions = l_physical_device.enumerateDeviceExtensionProperties();
				for (int k = 0; k < l_device_extensions.size(); k++)
				{
					if (strcmp(l_device_extensions[k].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
					{

						// vk::SurfaceCapabilitiesKHR l_surface_capabilities = l_physical_device.getSurfaceCapabilitiesKHR(this->surface);
						auto l_supported_surface_formats = l_physical_device.getSurfaceFormatsKHR(this->surface);
						auto l_supported_present_modes = l_physical_device.getSurfacePresentModesKHR(this->surface);

						if (l_supported_surface_formats.size() > 0 && l_supported_present_modes.size() > 0)
						{
							auto l_queueFamilies = l_physical_device.getQueueFamilyProperties();
							l_graphics_queue_family = QueueFamilyDefault;
							l_present_queue_family = QueueFamilyDefault;

							for (int j = 0; j < l_queueFamilies.size(); j++)
							{
								if (l_queueFamilies[j].queueFlags & vk::QueueFlags(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT))
								{
									l_graphics_queue_family = j;
								}

								if (l_physical_device.getSurfaceSupportKHR(j, this->surface))
								{
									l_present_queue_family = j;
								}

								if (l_graphics_queue_family != QueueFamilyDefault && l_present_queue_family != QueueFamilyDefault)
								{
									l_device_match = true;
									break;
								}
							}
						}


						break;
					}
				}



			}

			if (l_device_match)
			{
				this->graphics_device = l_physical_device;
				this->device_memory_properties = this->graphics_device.getMemoryProperties();
				this->graphics_queue_family = l_graphics_queue_family;
				this->present_queue_family = l_present_queue_family;
			}
		}
	}

	inline void createPhysicalDevice()
	{
		vk::DeviceQueueCreateInfo l_devicequeue_create_info;
		l_devicequeue_create_info.setQueueFamilyIndex(this->graphics_queue_family);
		l_devicequeue_create_info.setQueueCount(1);
		const float l_priority = 1.0f;
		l_devicequeue_create_info.setPQueuePriorities(&l_priority);

		vk::PhysicalDeviceFeatures l_devicefeatures;

		vk::DeviceCreateInfo l_device_create_info;
		l_device_create_info.setPQueueCreateInfos(&l_devicequeue_create_info);
		l_device_create_info.setQueueCreateInfoCount(1);
		l_device_create_info.setPEnabledFeatures(&l_devicefeatures);

		if (this->validationLayers_enabled)
		{
			l_device_create_info.setEnabledLayerCount((uint32_t)this->validation_layers.Size);
			l_device_create_info.setPpEnabledLayerNames(this->validation_layers.Memory);
		}

		com::Vector<const char*> l_devices_extensions(1);
		l_devices_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		l_device_create_info.setEnabledExtensionCount((uint32_t)l_devices_extensions.Size);
		l_device_create_info.setPpEnabledExtensionNames(l_devices_extensions.Memory);

		this->device = this->graphics_device.createDevice(l_device_create_info);
		this->graphics_queue = this->device.getQueue(this->graphics_queue_family, 0);
		this->present_queue = this->device.getQueue(this->present_queue_family, 0);
	}

	inline void createSwapChain(const Window& p_window)
	{
		this->swap_chain.init(this->instance, this->device, this->graphics_device, this->surface, p_window);
	}

	inline void destroySwapChain()
	{
		this->swap_chain.dispose();
	}

	inline void createSurface(const Window& p_window)
	{

		vk::Win32SurfaceCreateInfoKHR l_surfeca_create_info;
		l_surfeca_create_info.setHwnd(rdwindow::get_window_native(p_window.Handle));
		l_surfeca_create_info.setHinstance(GetModuleHandle(nullptr));
		this->surface = this->instance.createWin32SurfaceKHR(l_surfeca_create_info);
	};

	inline void destroySurface()
	{
		this->instance.destroySurfaceKHR(this->surface);
	}

	inline void createCommandBufferPool()
	{
		vk::CommandPoolCreateInfo l_command_pool_create_info;
		l_command_pool_create_info.setQueueFamilyIndex(this->graphics_queue_family);
		l_command_pool_create_info.setFlags(vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer));
		this->command_pool = this->device.createCommandPool(l_command_pool_create_info);
	}

	inline void destroyCommandBufferPool()
	{
		this->device.destroyCommandPool(this->command_pool);
	}

	inline void create_draw_commandbuffers()
	{
		this->draw_commandbuffers = com::Vector<CommandBuffer>(this->swap_chain.framebuffers.Size);
		this->draw_commandbuffers.Size = this->draw_commandbuffers.Capacity;
		for (int i = 0; i < this->draw_commandbuffers.Size; i++)
		{
			this->draw_commandbuffers[i] = CommandBuffer(this->device, this->command_pool, this->graphics_queue);
		}
	}

	inline void destroy_draw_commandbuffers()
	{
		for (int i = 0; i < this->draw_commandbuffers.Size; i++)
		{
			this->draw_commandbuffers[i].flush(this->device);
		}
		this->draw_commandbuffers.Size = 0;
	}

	inline void create_synchronization()
	{
		vk::SemaphoreCreateInfo l_semaphore_create_info;
		l_semaphore_create_info.setPNext(nullptr);

		this->synchronization.present_complete_semaphore = this->device.createSemaphore(l_semaphore_create_info);
		this->synchronization.render_complete_semaphore = this->device.createSemaphore(l_semaphore_create_info);

		vk::FenceCreateInfo l_fence_create_info;
		l_fence_create_info.setFlags(vk::FenceCreateFlags(vk::FenceCreateFlagBits::eSignaled));

		this->synchronization.draw_command_fences = com::Vector<vk::Fence>(this->draw_commandbuffers.Size);
		this->synchronization.draw_command_fences.Size = this->synchronization.draw_command_fences.Capacity;
		for (int i = 0; i < this->synchronization.draw_command_fences.Size; i++)
		{
			this->synchronization.draw_command_fences[i] = this->device.createFence(l_fence_create_info);
		}
	}

	inline void destroy_synchronization()
	{
		for (int i = 0; i < this->synchronization.draw_command_fences.Size; i++)
		{
			this->device.destroyFence(this->synchronization.draw_command_fences[i]);
		}

		this->device.destroySemaphore(this->synchronization.present_complete_semaphore);
		this->device.destroySemaphore(this->synchronization.render_complete_semaphore);
	}

	inline void create_descriptor_pool()
	{
		vk::DescriptorPoolSize l_types[1];
		l_types[0] = vk::DescriptorPoolSize();
		l_types[0].setDescriptorCount(1);

		vk::DescriptorPoolCreateInfo l_descriptor_pool_create_info;
		l_descriptor_pool_create_info.setPNext(nullptr);
		l_descriptor_pool_create_info.setPoolSizeCount(1);
		l_descriptor_pool_create_info.setPPoolSizes(l_types);
		l_descriptor_pool_create_info.setMaxSets(1);

		this->descriptor_pool = this->device.createDescriptorPool(l_descriptor_pool_create_info);
	}

	inline void destroy_descriptor_pool()
	{
		this->device.destroyDescriptorPool(this->descriptor_pool);
	}

	inline void define_draw_commands()
	{

	}

};


struct Vertex
{
	vec3f position;
	vec3f color;
};

typedef size_t ShaderHandle;
struct Shader
{
	struct Step
	{
		vk::ShaderModule shader_module;
		std::string entry_name;
		vk::ShaderStageFlagBits stage;
	};

	vk::DescriptorSetLayout descriptorset_layout;
	vk::PipelineLayout pipeline_layout;
	vk::Pipeline pipeline;

	Shader() : descriptorset_layout(nullptr), pipeline_layout(nullptr), pipeline(nullptr)
	{
		
	}

	Shader(const vk::Device& p_device, const RenderPass& p_render_pass, const std::string& p_vertex_shader, const std::string& p_fragment_shader)
	{
		this->createDescriptorSetLayout(p_device);
		this->createPipelineLayout(p_device);
		this->createPipeline(p_device, p_render_pass, p_vertex_shader, p_fragment_shader);
	}

	inline void dispose(const vk::Device& p_device)
	{
		this->destroyPipeline(p_device);
		this->destroyPipelineLayout(p_device);
		this->destroyDescriptorSetLayout(p_device);

		this->descriptorset_layout = nullptr;
		this->pipeline = nullptr;
		this->pipeline_layout = nullptr;
	}

private:

	inline void createDescriptorSetLayout(const vk::Device& p_device)
	{
		// Vertex uniform buffer
		//TODO -> layout bindings are arrays
		vk::DescriptorSetLayoutBinding l_layout_binding;
		l_layout_binding.setDescriptorCount(1);
		l_layout_binding.setStageFlags(vk::ShaderStageFlagBits::eVertex);

		vk::DescriptorSetLayoutCreateInfo l_descriptorLayout_create_info;
		l_descriptorLayout_create_info.setBindingCount(1);
		l_descriptorLayout_create_info.setPBindings(&l_layout_binding);
		
		this->descriptorset_layout = p_device.createDescriptorSetLayout(l_descriptorLayout_create_info);
	}

	inline void destroyDescriptorSetLayout(const vk::Device& p_device)
	{
		p_device.destroyDescriptorSetLayout(this->descriptorset_layout);
	}

	inline void createPipelineLayout(const vk::Device& p_device)
	{
		vk::PipelineLayoutCreateInfo l_pipelinelayout_create_info;
		l_pipelinelayout_create_info.setSetLayoutCount(1);
		l_pipelinelayout_create_info.setPSetLayouts(&this->descriptorset_layout);
		this->pipeline_layout = p_device.createPipelineLayout(l_pipelinelayout_create_info);
	}

	inline void createPipeline(const vk::Device& p_device, const RenderPass& p_renderPass,
				const std::string& p_vertex_shader, const std::string& p_fragment_shader)
	{
		vk::GraphicsPipelineCreateInfo l_pipeline_graphcis_create_info;
		l_pipeline_graphcis_create_info.setLayout(this->pipeline_layout);
		l_pipeline_graphcis_create_info.setRenderPass(p_renderPass.l_render_pass);

		vk::PipelineInputAssemblyStateCreateInfo l_inputassembly_state;
		l_inputassembly_state.setTopology(vk::PrimitiveTopology::eTriangleList);
		l_inputassembly_state.setPrimitiveRestartEnable(false);

		vk::PipelineRasterizationStateCreateInfo l_rasterization_state;
		l_rasterization_state.setPolygonMode(vk::PolygonMode::eFill);
		l_rasterization_state.setCullMode(vk::CullModeFlagBits::eBack);
		l_rasterization_state.setFrontFace(vk::FrontFace::eClockwise);
		l_rasterization_state.setLineWidth(1.0f);
		l_rasterization_state.setDepthClampEnable(false);
		l_rasterization_state.setRasterizerDiscardEnable(false);
		l_rasterization_state.setDepthBiasEnable(false);

		vk::PipelineColorBlendAttachmentState l_blendattachment_state;
		l_blendattachment_state.setColorWriteMask(vk::ColorComponentFlags(0xf));
		l_blendattachment_state.setBlendEnable(false);
		vk::PipelineColorBlendStateCreateInfo l_blendattachment_state_create;
		l_blendattachment_state_create.setAttachmentCount(1);
		l_blendattachment_state_create.setPAttachments(&l_blendattachment_state);

		vk::PipelineViewportStateCreateInfo l_viewport_state;
		l_viewport_state.setViewportCount(1);
		l_viewport_state.setScissorCount(1);

		com::Vector<vk::DynamicState> l_dynamicstates_enabled(2);
		l_dynamicstates_enabled.Size = 2;
		l_dynamicstates_enabled[0] = vk::DynamicState::eViewport;
		l_dynamicstates_enabled[1] = vk::DynamicState::eScissor;
		vk::PipelineDynamicStateCreateInfo l_dynamicstates;
		l_dynamicstates.setDynamicStateCount((uint32_t)l_dynamicstates_enabled.Size);
		l_dynamicstates.setPDynamicStates(l_dynamicstates_enabled.Memory);

		/*
		vk::PipelineDepthStencilStateCreateInfo l_depthstencil_state;
		l_depthstencil_state.setDepthTestEnable(false);
		l_depthstencil_state.setStencilTestEnable(false);
		*/

		vk::PipelineMultisampleStateCreateInfo l_multisample_state;
		l_multisample_state.setRasterizationSamples(vk::SampleCountFlagBits::e1);
		l_multisample_state.setPSampleMask(nullptr);



		vk::VertexInputBindingDescription l_vertex_input_binding;
		l_vertex_input_binding.setBinding(0);
		l_vertex_input_binding.setStride(sizeof(Vertex));
		l_vertex_input_binding.setInputRate(vk::VertexInputRate::eVertex);

		//vertex input
		com::Vector<vk::VertexInputAttributeDescription> l_vertex_input_attributes(2);
		l_vertex_input_attributes.Size = l_vertex_input_attributes.Capacity;

		l_vertex_input_attributes[0].setBinding(0);
		l_vertex_input_attributes[0].setLocation(0);
		l_vertex_input_attributes[0].setFormat(vk::Format::eR32G32B32Sfloat);
		l_vertex_input_attributes[0].setOffset(offsetof(Vertex, position));

		l_vertex_input_attributes[1].setBinding(0);
		l_vertex_input_attributes[1].setLocation(1);
		l_vertex_input_attributes[1].setFormat(vk::Format::eR32G32B32Sfloat);
		l_vertex_input_attributes[1].setOffset(offsetof(Vertex, color));

		vk::PipelineVertexInputStateCreateInfo l_vertex_input_create;
		l_vertex_input_create.setVertexBindingDescriptionCount(1);
		l_vertex_input_create.setPVertexBindingDescriptions(&l_vertex_input_binding);
		l_vertex_input_create.setVertexAttributeDescriptionCount((uint32_t)l_vertex_input_attributes.Size);
		l_vertex_input_create.setPVertexAttributeDescriptions(l_vertex_input_attributes.Memory);

		Step vertex_shader;
		Step fragment_shader;

		vertex_shader.shader_module = load_shadermodule(p_device, p_vertex_shader);
		vertex_shader.stage = vk::ShaderStageFlagBits::eVertex;
		vertex_shader.entry_name = "main";

		fragment_shader.shader_module = load_shadermodule(p_device, p_fragment_shader);
		fragment_shader.stage = vk::ShaderStageFlagBits::eFragment;
		fragment_shader.entry_name = "main";


		com::Vector<vk::PipelineShaderStageCreateInfo> l_shaderStages(2);
		l_shaderStages.Size = l_shaderStages.Capacity;
		vk::PipelineShaderStageCreateInfo& l_vertex_stage = l_shaderStages[0];
		l_vertex_stage = vk::PipelineShaderStageCreateInfo();
		*(vk::StructureType*)&l_vertex_stage.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
		l_vertex_stage.setStage(vk::ShaderStageFlagBits::eVertex);
		l_vertex_stage.setModule(vertex_shader.shader_module);
		l_vertex_stage.setPName(vertex_shader.entry_name.c_str());


		vk::PipelineShaderStageCreateInfo& l_fragment_stage = l_shaderStages[1];
		l_fragment_stage = vk::PipelineShaderStageCreateInfo();
		*(vk::StructureType*)& l_fragment_stage.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
		l_fragment_stage.setStage(vk::ShaderStageFlagBits::eFragment);
		l_fragment_stage.setModule(fragment_shader.shader_module);
		l_fragment_stage.setPName(fragment_shader.entry_name.c_str());

		l_pipeline_graphcis_create_info.setStageCount((uint32_t)l_shaderStages.Size);
		l_pipeline_graphcis_create_info.setPStages(l_shaderStages.Memory);

		l_pipeline_graphcis_create_info.setPVertexInputState(&l_vertex_input_create);
		l_pipeline_graphcis_create_info.setPInputAssemblyState(&l_inputassembly_state);
		l_pipeline_graphcis_create_info.setPRasterizationState(&l_rasterization_state);
		l_pipeline_graphcis_create_info.setPColorBlendState(&l_blendattachment_state_create);
		l_pipeline_graphcis_create_info.setPMultisampleState(&l_multisample_state);
		l_pipeline_graphcis_create_info.setPViewportState(&l_viewport_state);
		l_pipeline_graphcis_create_info.setPDynamicState(&l_dynamicstates);

		this->pipeline = p_device.createGraphicsPipeline(vk::PipelineCache(), l_pipeline_graphcis_create_info);

		dispose_shaderModule(p_device, vertex_shader.shader_module);
		dispose_shaderModule(p_device, fragment_shader.shader_module);
	}

	inline void destroyPipeline(const vk::Device& p_device)
	{
		p_device.destroyPipeline(this->pipeline);
	}

	inline void destroyPipelineLayout(const vk::Device& p_device)
	{
		p_device.destroyPipelineLayout(this->pipeline_layout);
	}

	inline static vk::ShaderModule load_shadermodule(const vk::Device& p_device, const std::string& p_file_path)
	{
		size_t l_size;
		std::string l_shader_code{};
		

		std::ifstream l_stream(p_file_path, std::ios::binary | std::ios::in | std::ios::ate);
		if (l_stream.is_open())
		{
			l_size = l_stream.tellg();
			l_stream.seekg(0, std::ios::beg);
			l_shader_code.resize(l_size);
			l_stream.read((char*)l_shader_code.c_str(), l_size);
			l_stream.close();
		}

		if (l_shader_code.size() > 0)
		{
			vk::ShaderModuleCreateInfo l_shader_module_create_info;
			l_shader_module_create_info.setCodeSize(l_size);
			l_shader_module_create_info.setPCode((uint32_t*)l_shader_code.c_str());
			return p_device.createShaderModule(l_shader_module_create_info);
		}

		return nullptr;
	}

	inline static void dispose_shaderModule(const vk::Device& p_device, const vk::ShaderModule& p_shader_module)
	{
		p_device.destroyShaderModule(p_shader_module);
	}
};

enum Staging;
enum NotStaging;


template<class ElementType = char, class StagingUsage = Staging>
struct AGPUBuffer
{
	vk::DeviceMemory memory;
	vk::Buffer buffer;

	inline void dispose(const vk::Device& p_device)
	{
		if (this->memory)
		{
			p_device.freeMemory(this->memory);
			p_device.destroyBuffer(this->buffer);
			this->memory = nullptr;
			this->buffer = nullptr;
		}	
	}
};


template<class ElementType = char, class StagingUsage = Staging>
struct GPUBuffer {};

template<class ElementType>
struct GPUBuffer<ElementType, Staging>
{
	AGPUBuffer<ElementType> buffer;

	inline void allocate(size_t p_element_number, const void* p_source, vk::BufferUsageFlags p_usageflags, const RenderAPI& p_render)
	{
		CommandBuffer l_copy_cmd = CommandBuffer(p_render.device, p_render.command_pool, p_render.graphics_queue);
		l_copy_cmd.begin();

		AGPUBuffer<ElementType> l_staging_buffer;
		this->allocate_buffered(l_copy_cmd, p_element_number, p_source, p_usageflags, p_render, &l_staging_buffer);

		l_copy_cmd.flush(p_render.device);
		l_staging_buffer.dispose(p_render.device);
	}

	inline void allocate_buffered(CommandBuffer& p_commandBuffer, size_t p_element_number, const void* p_source, vk::BufferUsageFlags p_usageflags, const RenderAPI& p_render, 
		AGPUBuffer<ElementType>* out_staging_buffer)
	{
		size_t l_buffer_size = p_element_number * sizeof(ElementType);

		//STAGING
		vk::BufferCreateInfo l_buffercreate_info;
		l_buffercreate_info.setUsage(vk::BufferUsageFlagBits::eTransferSrc);
		l_buffercreate_info.setSize(l_buffer_size);
		out_staging_buffer->buffer = p_render.device.createBuffer(l_buffercreate_info);

		vk::MemoryRequirements l_requirements = p_render.device.getBufferMemoryRequirements(out_staging_buffer->buffer);

		vk::MemoryAllocateInfo l_memory_allocate_info;
		l_memory_allocate_info.setAllocationSize(l_requirements.size);
		l_memory_allocate_info.setMemoryTypeIndex(
			p_render.getMemoryTypeIndex(l_requirements.memoryTypeBits, vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)));

		void* l_staging_data;
		out_staging_buffer->memory = p_render.device.allocateMemory(l_memory_allocate_info);
		l_staging_data = p_render.device.mapMemory(out_staging_buffer->memory, 0, l_memory_allocate_info.allocationSize);
		memcpy(l_staging_data, p_source, l_buffer_size);
		p_render.device.unmapMemory(out_staging_buffer->memory);
		p_render.device.bindBufferMemory(out_staging_buffer->buffer, out_staging_buffer->memory, 0);


		//Actual buffer
		l_buffercreate_info.setUsage(p_usageflags | vk::BufferUsageFlags(vk::BufferUsageFlagBits::eTransferDst));
		this->buffer.buffer = p_render.device.createBuffer(l_buffercreate_info);

		l_requirements = p_render.device.getBufferMemoryRequirements(this->buffer.buffer);
		l_memory_allocate_info.setAllocationSize(l_requirements.size);
		l_memory_allocate_info.setMemoryTypeIndex(
			p_render.getMemoryTypeIndex(l_requirements.memoryTypeBits, vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eDeviceLocal)));
		this->buffer.memory = p_render.device.allocateMemory(l_memory_allocate_info);
		p_render.device.bindBufferMemory(this->buffer.buffer, this->buffer.memory, 0);

		vk::BufferCopy l_buffer_copy_regions;
		l_buffer_copy_regions.setSize(l_buffer_size);
		p_commandBuffer.command_buffer.copyBuffer(out_staging_buffer->buffer, this->buffer.buffer, 1, &l_buffer_copy_regions);
	}

	inline void dispose(const vk::Device& p_device)
	{
		this->buffer.dispose(p_device);
	}
};


template<class ElementType = char, class StagingUsage = Staging>
struct VertexBuffer
{
	GPUBuffer<ElementType, StagingUsage> buffer;

	VertexBuffer() {};

	inline void allocate(size_t p_element_number, const void* p_source, const RenderAPI& p_render)
	{
		this->buffer.allocate(p_element_number, p_source, vk::BufferUsageFlags(vk::BufferUsageFlagBits::eVertexBuffer), p_render);
	};


	inline void allocate_buffered(CommandBuffer& p_commandBuffer, size_t p_element_number, const void* p_source, const RenderAPI& p_render, AGPUBuffer<ElementType>* out_staging_buffer)
	{
		this->buffer.allocate_buffered(p_commandBuffer, p_element_number, p_source, vk::BufferUsageFlags(vk::BufferUsageFlagBits::eVertexBuffer), p_render, out_staging_buffer);
	};

	inline void dispose(const vk::Device& p_device)
	{
		this->buffer.dispose(p_device);
	}
};

template<class ElementType = char, class StagingUsage = Staging>
struct IndexBuffer
{
	GPUBuffer<ElementType, StagingUsage> buffer;

	IndexBuffer() {};

	inline void allocate(size_t p_element_number, const void* p_source, const RenderAPI& p_render)
	{
		this->buffer.allocate(p_element_number, p_source, vk::BufferUsageFlags(vk::BufferUsageFlagBits::eIndexBuffer), p_render);
	};

	inline void allocate_buffered(CommandBuffer& p_commandBuffer, size_t p_element_number, const void* p_source, const RenderAPI& p_render, AGPUBuffer<ElementType>* out_staging_buffer)
	{
		this->buffer.allocate_buffered(p_commandBuffer, p_element_number, p_source, vk::BufferUsageFlags(vk::BufferUsageFlagBits::eIndexBuffer), p_render, out_staging_buffer);
	};

	inline void dispose(const vk::Device& p_device)
	{
		this->buffer.dispose(p_device);
	}
};

struct Mesh
{
	VertexBuffer<Vertex> vertices;
	IndexBuffer<uint32_t> indices;
	size_t indices_length;

	Mesh(){}

	inline Mesh(const com::Vector<Vertex>& p_vertcies, const com::Vector<uint32_t>& p_indices, const RenderAPI& p_render)
	{
		CommandBuffer l_copy_cmd = CommandBuffer(p_render.device, p_render.command_pool, p_render.graphics_queue);
		l_copy_cmd.begin();

		AGPUBuffer<Vertex> l_vertexBuffer_staging;
		AGPUBuffer<uint32_t> l_indexBuffer_staging;
		this->vertices.allocate_buffered(l_copy_cmd, p_vertcies.Size, p_vertcies.Memory, p_render, &l_vertexBuffer_staging);
		this->indices.allocate_buffered(l_copy_cmd, p_indices.Size, p_indices.Memory, p_render, &l_indexBuffer_staging);
		l_copy_cmd.flush(p_render.device);
		l_vertexBuffer_staging.dispose(p_render.device);
		l_indexBuffer_staging.dispose(p_render.device);

		this->indices_length = p_indices.Size;
	}

	inline void dispose(const vk::Device& p_device)
	{
		this->vertices.dispose(p_device);
		this->indices.dispose(p_device);
	}
};

typedef size_t MaterialHandle;
struct Material
{
	//TODO, descriptor set

	Material(){}
};

typedef size_t RenderableObjectHandle;

struct RenderableObject
{
	com::PoolToken<Mesh> mesh;

	RenderableObject(){}
	RenderableObject(const com::PoolToken<Mesh>& p_mesh) : mesh(p_mesh) {	}

	inline void dispose()
	{
		this->mesh = com::PoolToken<Mesh>();
	}

	inline void draw(CommandBuffer& p_commandbuffer, com::Pool<Mesh>& p_mesh_heap)
	{
		vk::DeviceSize l_offsets[1] = { 0 };
		p_commandbuffer.command_buffer.bindVertexBuffers(0, 1, &p_mesh_heap[this->mesh].vertices.buffer.buffer.buffer, l_offsets);
		p_commandbuffer.command_buffer.bindIndexBuffer(p_mesh_heap[this->mesh].indices.buffer.buffer.buffer, 0, vk::IndexType::eUint32);
		p_commandbuffer.command_buffer.drawIndexed(p_mesh_heap[this->mesh].indices_length, 1, 0, 0, 1);
	}
};

struct RenderHeap
{
	com::OptionalPool<Shader> shaders;
	com::Pool<com::Vector<com::PoolToken<Optional<Material>>>> shaders_to_materials;

	com::OptionalPool<Material> materials;
	com::Pool<com::Vector<com::PoolToken<Optional<RenderableObject>>>> material_to_renderableobjects;

	com::OptionalPool<RenderableObject> renderableobjects;

	com::Pool<Mesh> meshes;

	inline com::PoolToken<Optional<Shader>> pushShader(Shader& p_shader)
	{
		com::PoolToken<Optional<Shader>> l_shader_handle = this->shaders.alloc_element(p_shader);
		this->shaders_to_materials.alloc_element(com::Vector<com::PoolToken<Optional<Material>>>());
		return l_shader_handle;
	}

	inline void disposeShader(const com::PoolToken<Optional<Shader>> p_shader, const vk::Device& p_device)
	{
		Optional<Shader>& l_shader = this->shaders[p_shader];
		l_shader.value.dispose(p_device);
		this->shaders.release_element(p_shader);

		com::Vector<com::PoolToken<Optional<Material>>>& l_shader_to_materials = this->shaders_to_materials[p_shader.Index];
		for (size_t i = 0; i < l_shader_to_materials.Size; i++)
		{
			this->disposeMaterial(l_shader_to_materials[i]);
		}
		this->shaders_to_materials.release_element(p_shader.Index);
	}

	inline com::PoolToken<Optional<Material>> pushMaterial(const com::PoolToken<Optional<Shader>> p_shaderhandle, Material& p_material)
	{
		com::PoolToken<Optional<Material>> l_material_handle = this->materials.alloc_element(p_material);
		this->material_to_renderableobjects.alloc_element(com::Vector<com::PoolToken<Optional<RenderableObject>>>());
		this->shaders_to_materials[p_shaderhandle.Index].push_back(l_material_handle);
		return l_material_handle;
	}

	inline void disposeMaterial(const com::PoolToken<Optional<Material>> p_material)
	{
		this->materials.release_element(p_material);

		com::Vector<com::PoolToken<Optional<RenderableObject>>>& l_material_to_renderableobjects = this->material_to_renderableobjects[p_material.Index];
		for (size_t i = 0; i < l_material_to_renderableobjects.Size; i++)
		{
			this->disposeRenderableObject(l_material_to_renderableobjects[i]);
		}
		this->material_to_renderableobjects.release_element(p_material.Index);
	}

	inline com::PoolToken<Optional<RenderableObject>> pushRendereableObject(const com::PoolToken<Optional<Material>> p_materialhandle, RenderableObject& p_renderableObject)
	{
		com::PoolToken<Optional<RenderableObject>> l_renderableobjet_handle = this->renderableobjects.alloc_element(p_renderableObject);
		this->material_to_renderableobjects[p_materialhandle.Index].push_back(l_renderableobjet_handle);
		return l_renderableobjet_handle;
	}

	inline void disposeRenderableObject(const com::PoolToken<Optional<RenderableObject>> p_renderableObject)
	{
		Optional<RenderableObject>& l_renderableobject = this->renderableobjects[p_renderableObject];
		l_renderableobject.value.dispose();
		this->renderableobjects.release_element(p_renderableObject);
	}
};

struct Render
{
	Window window;
	RenderAPI renderApi;

	RenderHeap heap;
	com::PoolToken<Optional<Shader>> shader;
	// vk::DescriptorSet descriptorset;
	com::PoolToken<Mesh> l_mesh;

	inline Render()
	{
		this->window = Window(800, 600, "MyGame");
		this->renderApi.init(window);
		this->shader = this->heap.pushShader(Shader(this->renderApi.device, this->renderApi.swap_chain.renderpass, "E:/GameProjects/CPPTestVS/Render/shader/TriVert.spv", "E:/GameProjects/CPPTestVS/Render/shader/TriFrag.spv"));
		this->createVertexBuffer();

		this->draw();
	};

	inline void dispose()
	{
		this->destroyVertexBuffer();
		this->heap.disposeShader(this->shader, this->renderApi.device);
		this->renderApi.dispose();
		this->window.dispose();
	};

private:
	
	inline void createVertexBuffer()
	{
		com::Vector<Vertex> l_vertexBuffer(3);
		l_vertexBuffer.Size = l_vertexBuffer.Capacity;
		l_vertexBuffer[0].position = vec3f(0.0f, -0.5f, 0.0f);
		l_vertexBuffer[0].color = vec3f(1.0f, 0.0f, 0.0f);
		l_vertexBuffer[1].position = vec3f(0.5f, 0.5f, 0.0f);
		l_vertexBuffer[1].color = vec3f(0.0f, 1.0f, 0.0f);
		l_vertexBuffer[2].position = vec3f(-0.5f, 0.5f, 0.0f);
		l_vertexBuffer[2].color = vec3f(0.0f, 0.0f, 1.0f);

		com::Vector<uint32_t> l_indicesBuffer(3);
		l_indicesBuffer.Size = l_indicesBuffer.Capacity;
		l_indicesBuffer[0] = 0; l_indicesBuffer[1] = 1; l_indicesBuffer[2] = 2;

		this->l_mesh = this->heap.meshes.alloc_element(Mesh(l_vertexBuffer, l_indicesBuffer, this->renderApi));
		com::PoolToken<Optional<Material>> l_material = this->heap.pushMaterial(this->shader, Material());
		this->heap.pushRendereableObject(l_material, RenderableObject(this->l_mesh));
	}

	inline void destroyVertexBuffer()
	{
		this->heap.meshes[this->l_mesh].dispose(this->renderApi.device);
		this->heap.meshes.release_element(this->l_mesh);
	}

	/*
	inline void createDescriptorSet()
	{
		vk::DescriptorSetAllocateInfo l_descriptorset_allocate;
		l_descriptorset_allocate.setDescriptorPool(this->renderApi.descriptor_pool);
		l_descriptorset_allocate.setDescriptorSetCount(1);
		l_descriptorset_allocate.setPSetLayouts(&this->shaderTest.descriptorset_layout);
		this->descriptorset = this->renderApi.device.allocateDescriptorSets(l_descriptorset_allocate)[0];

		vk::WriteDescriptorSet l_swritedescriptorset;
		l_swritedescriptorset.setDstSet(this->descriptorset);
		l_swritedescriptorset.setDescriptorCount(1);
		l_swritedescriptorset.setDescriptorType(vk::DescriptorType::eUniformBuffer);
		//l_swritedescriptorset.setPBufferInfo();
		l_swritedescriptorset.setDstBinding(0);
	}
	*/

	inline void draw()
	{
		uint32_t l_render_image_index = this->renderApi.swap_chain.getNextImage(this->renderApi.synchronization.present_complete_semaphore);
		this->renderApi.device.waitForFences(1, &this->renderApi.synchronization.draw_command_fences[l_render_image_index], true, UINT64_MAX);
		this->renderApi.device.resetFences(1, &this->renderApi.synchronization.draw_command_fences[l_render_image_index]);


		vk::ClearValue l_clear;
		l_clear.color.setFloat32({ 0.0f, 0.0f, 0.2f, 1.0f });

		vk::RenderPassBeginInfo l_renderpass_begin;
		l_renderpass_begin.setPNext(nullptr);
		l_renderpass_begin.setRenderPass(this->renderApi.swap_chain.renderpass.l_render_pass);
		vk::Rect2D l_renderArea;
		l_renderArea.setOffset(vk::Offset2D(0,0));
		l_renderArea.setExtent(this->renderApi.swap_chain.extend);
		l_renderpass_begin.setRenderArea(l_renderArea);
		l_renderpass_begin.setClearValueCount(1);
		l_renderpass_begin.setPClearValues(&l_clear);
		l_renderpass_begin.setFramebuffer(this->renderApi.swap_chain.framebuffers[l_render_image_index]);

		vk::Viewport l_viewport;
		l_viewport.setHeight(this->window.Height);
		l_viewport.setWidth(this->window.Width);
		l_viewport.setMinDepth(0.0f);
		l_viewport.setMaxDepth(1.0f);

		vk::Rect2D l_windowarea;
		l_windowarea.setOffset(vk::Offset2D(0, 0)); 
		l_windowarea.setExtent(vk::Extent2D(this->window.Width, this->window.Height));

		CommandBuffer& l_command_buffer = this->renderApi.draw_commandbuffers[l_render_image_index];
		l_command_buffer.begin();
		l_command_buffer.command_buffer.beginRenderPass(l_renderpass_begin, vk::SubpassContents::eInline);
		l_command_buffer.command_buffer.setViewport(0,1,&l_viewport);
		l_command_buffer.command_buffer.setScissor(0, 1, &l_windowarea);
		//l_command_buffer.command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->shaderTest.pipeline_layout, 0, 1, )

		for (size_t l_shader_index = 0; l_shader_index < this->heap.shaders.size(); l_shader_index++)
		{
			Optional<Shader>& l_shader = this->heap.shaders[l_shader_index];
			if (l_shader.hasValue)
			{
				l_command_buffer.command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, l_shader.value.pipeline);

				com::Vector<com::PoolToken<Optional<Material>>>& l_materials = this->heap.shaders_to_materials[l_shader_index];
				for (size_t l_material_index = 0; l_material_index < l_materials.Size; l_material_index ++)
				{
					Optional<Material>& l_material = this->heap.materials[l_materials[l_material_index]];
					if (l_material.hasValue)
					{
						com::Vector<com::PoolToken<Optional<RenderableObject>>>& l_renderableobjects = this->heap.material_to_renderableobjects[l_material_index];
						for (size_t l_renderableobject_index = 0; l_renderableobject_index < l_renderableobjects.Size; l_renderableobject_index++)
						{
							Optional<RenderableObject>& l_renderableobject = this->heap.renderableobjects[l_renderableobject_index];
							if (l_renderableobject.hasValue)
							{
								this->heap.renderableobjects[l_renderableobject_index].value.draw(l_command_buffer, this->heap.meshes);
							}
						}
					}
				}
				
			}
		}

		l_command_buffer.command_buffer.endRenderPass();

		l_command_buffer.end();


		vk::PipelineStageFlags l_pipelinestage_wait = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		vk::SubmitInfo l_submit;
		l_submit.setPWaitDstStageMask(&l_pipelinestage_wait);
		l_submit.setWaitSemaphoreCount(1);
		l_submit.setPWaitSemaphores(&this->renderApi.synchronization.present_complete_semaphore);
		l_submit.setSignalSemaphoreCount(1);
		l_submit.setPSignalSemaphores(&this->renderApi.synchronization.render_complete_semaphore);
		l_submit.setCommandBufferCount(1);
		l_submit.setPCommandBuffers(&l_command_buffer.command_buffer);


		this->renderApi.graphics_queue.submit(1, &l_submit, this->renderApi.synchronization.draw_command_fences[l_render_image_index]);
		this->renderApi.swap_chain.presentImage(this->renderApi.present_queue, l_render_image_index, this->renderApi.synchronization.render_complete_semaphore);
	}
};

RenderHandle create_render()
{
	return new Render();
};

void destroy_render(const RenderHandle& p_render)
{
	((Render*)p_render)->dispose();
	delete (Render*)p_render;
};

bool render_window_should_close(const RenderHandle& p_render)
{
	Render* l_render = (Render*)p_render;
	return rdwindow::window_should_close(l_render->window.Handle);
};

void render_window_pool_event(const RenderHandle& p_render)
{
	Render* l_render = (Render*)p_render;
	rdwindow::window_pool_event(l_render->window.Handle);
};

