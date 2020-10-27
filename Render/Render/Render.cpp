#include <interface/Render/render.hpp>
#include <driver/Render/rdwindow.hpp>
#include "Math/math.hpp"
#include "Common/Container/vector.hpp"
#include <fstream>

using namespace Math;

struct Window
{
	WindowHandle Handle;
	short int Width;
	short int Height;

	Window() = default;
	inline Window(const short int p_width, const short int p_height, const std::string &p_title)
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

private:
	const vk::Instance *instance;
	const vk::PhysicalDevice* physicalDevice;
	const vk::Device *device;
	const vk::SurfaceKHR *surface;
	const Window *window;

	vk::SurfaceCapabilitiesKHR surface_capabilities;

public:
	inline SwapChain() = default;

	inline void init(const vk::Instance &p_instance, const vk::Device &p_device,
		const vk::PhysicalDevice& p_physical_device,
		const vk::SurfaceKHR& p_surface, const Window &p_window)
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
		
	}

	inline void dispose()
	{
		this->destroy_images();
		this->device->destroySwapchainKHR(this->handle);
	}

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
};

struct RenderAPI
{
	static const uint32_t QueueFamilyDefault = -1;

	vk::Instance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	vk::PhysicalDevice graphics_device;
	
	vk::Device device;
	vk::SurfaceKHR surface;

	vk::Queue graphics_queue;
	uint32_t graphics_queue_family = QueueFamilyDefault;

	vk::Queue present_queue;
	uint32_t present_queue_family = QueueFamilyDefault;

	SwapChain swap_chain;

	bool validationLayers_enabled;
	com::Vector<const char *> validation_layers;

	inline void init(const Window &p_window)
	{
		this->createInstance();
		this->createDebugCallback();
		this->createSurface(p_window);
		this->getPhysicalDevice();
		this->createPhysicalDevice();
		this->createSwapChain(p_window);
	};

	inline void dispose()
	{
		this->destroySwapChain();
		this->destroySurface();
		this->device.destroy();
		this->destroyDebugCallback();
		this->destroyInstance();
	}

private:
	inline void createInstance()
	{
		vk::ApplicationInfo l_app_info;
		l_app_info.setPApplicationName("test");

		vk::InstanceCreateInfo l_instance_create_info{};
		l_instance_create_info.setPApplicationInfo(&l_app_info);

		com::Vector<const char *> l_extensions;
		this->validationLayers(l_instance_create_info, this->validation_layers);
		this->extensions(l_instance_create_info, l_extensions);
		this->instance = vk::createInstance(l_instance_create_info);
	};

	inline void destroyInstance()
	{
		this->instance.destroy();
	};

	inline void validationLayers(vk::InstanceCreateInfo &p_instance_create_info, com::Vector<const char *> &p_validationLayers)
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

	inline void extensions(vk::InstanceCreateInfo &p_instance_create_info, com::Vector<const char *> &p_extensions)
	{
		uint32_t l_glfw_extension_count = 0;
		const char **l_glfw_extensions;
		l_glfw_extensions = glfwGetRequiredInstanceExtensions(&l_glfw_extension_count);

		p_extensions = com::Vector<const char *>(l_glfw_extension_count);
		p_extensions.insert_at(com::MemorySlice<const char *>(*l_glfw_extensions, (size_t)l_glfw_extension_count), 0);

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
		const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
		void *pUserData)
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
			vk::PhysicalDevice &l_physical_device = l_physical_devices[i];
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

	inline void createSwapChain(const Window &p_window)
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
};

struct Render
{
	Window window;
	RenderAPI renderApi;

	inline Render()
	{
		this->window = Window(800, 600, "MyGame");
		this->renderApi.init(window);
	};

	inline void dispose()
	{
		this->renderApi.dispose();
		this->window.dispose();
	};
};

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
	vk::RenderPass render_pass;

	Shader(const vk::Device& p_device)
	{
		Step vertex_shader;
		Step fragment_shader;

		vertex_shader.shader_module = load_shadermodule(p_device, "");
		vertex_shader.stage = vk::ShaderStageFlagBits::eVertex;
		vertex_shader.entry_name = "main";

		fragment_shader.shader_module = load_shadermodule(p_device, "");
		fragment_shader.stage = vk::ShaderStageFlagBits::eFragment;
		fragment_shader.entry_name = "main";

		this->createDescriptorSetLayout(p_device);
		this->createPipelineLayout(p_device);
	}

	inline void dispose(const vk::Device& p_device)
	{
		this->destroyPipelineLayout(p_device);
		this->destroyDescriptorSetLayout(p_device);
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

	inline void destroyPipelineLayout(const vk::Device& p_device)
	{
		p_device.destroyPipelineLayout(this->pipeline_layout);
	}

	/*
	vk::PipelineShaderStageCreateInfo build_vertex_stage_creation()
	{
		vk::PipelineShaderStageCreateInfo l_return;
		l_return.setModule(this->vertex_shader_module);
		l_return.setStage(vk::ShaderStageFlagBits::eVertex);
		l_return.setPName("main");
	}
	*/

	static vk::ShaderModule load_shadermodule(const vk::Device& p_device, const std::string& p_file_path)
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