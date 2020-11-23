#pragma once

#include <Render/render.hpp>
#include <Render/assets.hpp>
#include <Render/rdwindow.hpp>
#include <optick.h>
#include <AssetServer/asset_server.hpp>
#include "Math/math.hpp"
#include "Common/Container/pool.hpp"
#include "Common/Container/vector.hpp"
#include "Common/Container/array_def.hpp"
#include "Common/Container/resource_map.hpp"
#include "Common/Container/hashmap.hpp"
#include "Common/Container/gptr.hpp"
#include "Common/Serialization/binary.hpp"
#include "Common/Memory/heap.hpp"
#include <fstream>
#include <stdlib.h>

using namespace Math;

struct CameraMatrices
{
	mat4f view;
	mat4f projection;
};

struct Vertex
{
	vec3f position;
	vec2f uv;
};

struct RenderWindow
{
	WindowHandle Handle;
	short int Width;
	short int Height;

	RenderWindow() = default;
	inline RenderWindow(const short int p_width, const short int p_height, const std::string& p_title)
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


struct ValidationLayer
{
	bool enabled;
	com::Vector<const char*> layers;
};



enum WriteMethod2
{
	// * HostWrite : Allow map
	HostWrite = 0,
	// * GPUWrite : Allow writing by copying from a source buffer.
	//              Cannot be accesses from from host.
	GPUWrite = 1
};


struct GPUMemoryWithOffset
{
	vk::DeviceMemory original_memory;
	char* original_mapped_memory;
	uint32_t memory_type;
	com::PoolToken allocated_chunk_token;
	size_t pagedbuffer_index;
};

template<unsigned WriteMethod>
struct PageGPUMemoryMappingFn {};

template<>
struct PageGPUMemoryMappingFn<WriteMethod2::HostWrite>
{
	inline static char* map(vk::Device p_device, const vk::DeviceMemory p_memory, const size_t p_size)
	{
		return (char*)p_device.mapMemory(p_memory, 0, p_size);
	}
};

template<>
struct PageGPUMemoryMappingFn<WriteMethod2::GPUWrite>
{
	inline static char* map(vk::Device p_device, const vk::DeviceMemory p_memory, const size_t p_size) { return nullptr; }
};

template<unsigned WriteMethod>
struct PageGPUMemory2
{
	struct GPUDeviceMemoryAllocator : public IAllocator
	{
		uint32_t memorytype;
		vk::Device device;
		vk::DeviceMemory root_memory;

		inline GPUDeviceMemoryAllocator() {}

		inline GPUDeviceMemoryAllocator(const uint32_t p_memorytype, vk::Device p_device)
		{
			this->memorytype = p_memorytype;
			this->device = p_device;
		};

		inline void* malloc(const size_t p_allocSize)
		{
			vk::MemoryAllocateInfo l_memoryallocate_info;
			l_memoryallocate_info.setAllocationSize(p_allocSize);
			l_memoryallocate_info.setMemoryTypeIndex(this->memorytype);
			this->root_memory = device.allocateMemory(l_memoryallocate_info);
			return PageGPUMemoryMappingFn<WriteMethod>::map(this->device, this->root_memory, p_allocSize);
		};

		inline void free(void* p_memory)
		{
			this->device.freeMemory(this->root_memory);
		};
	};

	GeneralPurposeHeap<GPUDeviceMemoryAllocator> heap;
	size_t page_index;

	inline void allocate(size_t p_page_index, size_t p_size, uint32_t p_memorytype, vk::Device p_device)
	{
		this->page_index = p_page_index;
		this->heap.allocate(p_size, GPUDeviceMemoryAllocator(p_memorytype, p_device));
	}

	inline void dispose(vk::Device p_device)
	{
		this->heap.dispose();
	}

	inline bool allocate_element(size_t p_size, size_t p_alignmenent_constaint, GPUMemoryWithOffset* out_chunk)
	{
		out_chunk->pagedbuffer_index = this->page_index;
		out_chunk->original_memory = this->heap.memory.allocator.root_memory;
		out_chunk->original_mapped_memory = this->heap.memory.Memory;

		return this->heap.allocate_element(p_size, AllocationAlignementConstraint(p_alignmenent_constaint), &out_chunk->allocated_chunk_token);
	}

	inline void release_element(const GPUMemoryWithOffset& p_buffer)
	{
		this->heap.release_element(p_buffer.allocated_chunk_token);
	}

};

template<unsigned WriteMethod>
struct PagedGPUMemory
{
	com::Vector<PageGPUMemory2<WriteMethod>> page_buffers;
	uint32_t memory_type;
	size_t chunk_size;

	inline void allocate(size_t p_chunksize, uint32_t p_memory_type)
	{
		this->memory_type = p_memory_type;
		this->chunk_size = p_chunksize;
		this->page_buffers.allocate(0);
	}

	inline void free(vk::Device p_device)
	{
		for (size_t i = 0; i < this->page_buffers.Size; i++)
		{
			this->page_buffers[i].dispose(p_device);
		}
		this->page_buffers.free();
	}

	inline bool allocate_element(size_t p_size, size_t p_alignmenent_constaint, vk::Device p_device, GPUMemoryWithOffset* out_chunk)
	{
		for (size_t i = 0; i < this->page_buffers.Size; i++)
		{
			if (this->page_buffers[i].allocate_element(p_size, p_alignmenent_constaint, out_chunk))
			{
				return true;
			}
		}

		this->page_buffers.push_back(PageGPUMemory2<WriteMethod>());
		this->page_buffers[this->page_buffers.Size - 1].allocate(this->page_buffers.Size - 1, this->chunk_size, this->memory_type, p_device);
		return this->page_buffers[this->page_buffers.Size - 1].allocate_element(p_size, p_alignmenent_constaint, out_chunk);
	}

	inline void free_element(const GPUMemoryWithOffset& p_buffer)
	{
		this->page_buffers[p_buffer.pagedbuffer_index].release_element(p_buffer);
	};

	inline GeneralPurposeHeapMemoryChunk& resolve(const GPUMemoryWithOffset& p_buffer)
	{
		return this->page_buffers[p_buffer.pagedbuffer_index].heap.resolve_allocated_chunk(p_buffer.allocated_chunk_token);
	};
};

struct PagedMemories
{
	PagedGPUMemory<WriteMethod2::GPUWrite> i7;
	PagedGPUMemory<WriteMethod2::HostWrite> i8;

	inline void allocate()
	{
		this->i7.allocate(16000000, 7);
		this->i8.allocate(16000000, 8);
	}

	inline void free(vk::Device p_device)
	{
		this->i7.free(p_device);
		this->i8.free(p_device);
	}

	inline bool allocate_element(size_t p_size, size_t p_alignmenent_constaint, uint32_t p_memory_type, vk::Device p_device, GPUMemoryWithOffset* out_chunk)
	{
		switch (p_memory_type)
		{
		case 7:
		{
			bool l_success = this->i7.allocate_element(p_size, p_alignmenent_constaint, p_device, out_chunk);
			out_chunk->memory_type = 7;
			return l_success;
		}
		break;
		case 8:
		{
			bool l_success = this->i8.allocate_element(p_size, p_alignmenent_constaint, p_device, out_chunk);
			out_chunk->memory_type = 8;
			return l_success;
		}
		break;
		}

		abort();

		return false;
	}

	inline void free_element(GPUMemoryWithOffset p_chunk)
	{
		switch (p_chunk.memory_type)
		{
		case 7:
		{
			this->i7.free_element(p_chunk);
		}
		break;
		case 8:
		{
			this->i8.free_element(p_chunk);
		}
		break;
		}
	}

	inline void resolve_allocated_chunk(const GPUMemoryWithOffset& p_chunk, GeneralPurposeHeapMemoryChunk** out_chunk)
	{
		switch (p_chunk.memory_type)
		{
		case 7:
		{
			*out_chunk = &this->i7.resolve(p_chunk);
			return;
		}
		break;
		case 8:
		{
			*out_chunk = &this->i8.resolve(p_chunk);
			return;
		}
		break;
		}

		*out_chunk = nullptr;
	};
};








struct Device
{
	static const uint32_t QueueFamilyDefault = -1;

	PagedMemories devicememory_allocator;

	vk::PhysicalDevice graphics_device;
	VkPhysicalDeviceMemoryProperties device_memory_properties;
	vk::Device device;

	vk::Queue graphics_queue;
	uint32_t graphics_queue_family = QueueFamilyDefault;

	vk::Queue present_queue;
	uint32_t present_queue_family = QueueFamilyDefault;

	inline void destroy()
	{
		this->devicememory_allocator.free(this->device);
		this->device.destroy();
	}

	inline uint32_t getMemoryTypeIndex(uint32_t p_typeBits, vk::MemoryPropertyFlags p_properties) const
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

	inline void createPhysicalDevice(const ValidationLayer& p_validation_layers)
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


		if (p_validation_layers.enabled)
		{
			l_device_create_info.setEnabledLayerCount((uint32_t)p_validation_layers.layers.Size);
			l_device_create_info.setPpEnabledLayerNames(p_validation_layers.layers.Memory);
		}

		com::Vector<const char*> l_devices_extensions;
		l_devices_extensions.allocate(1);
		l_devices_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		//l_devices_extensions.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
		{
			l_device_create_info.setEnabledExtensionCount((uint32_t)l_devices_extensions.Size);
			l_device_create_info.setPpEnabledExtensionNames(l_devices_extensions.Memory);

			this->device = this->graphics_device.createDevice(l_device_create_info);
			this->graphics_queue = this->device.getQueue(this->graphics_queue_family, 0);
			this->present_queue = this->device.getQueue(this->present_queue_family, 0);

			this->devicememory_allocator.allocate();
		}
		l_devices_extensions.free();
	}

	inline void getPhysicalDevice(vk::Instance p_instance, vk::SurfaceKHR p_surface)
	{
		auto l_physical_devices = p_instance.enumeratePhysicalDevices();
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
						auto l_supported_surface_formats = l_physical_device.getSurfaceFormatsKHR(p_surface);
						auto l_supported_present_modes = l_physical_device.getSurfacePresentModesKHR(p_surface);

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

								if (l_physical_device.getSurfaceSupportKHR(j, p_surface))
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
				this->device_memory_properties = l_physical_device.getMemoryProperties();
				this->graphics_queue_family = l_graphics_queue_family;
				this->present_queue_family = l_present_queue_family;
			}
		}
	}

	inline void destroyDevice()
	{
		this->devicememory_allocator.free(this->device);
	}
};


struct RenderPass
{
	vk::RenderPass l_render_pass;

	void create(const Device& p_device, vk::SurfaceFormatKHR p_surface_format, vk::Format p_depth_format)
	{
		com::Vector<vk::AttachmentDescription> l_attachments;
		l_attachments.allocate(2);
		l_attachments.Size = l_attachments.Capacity;

		com::Vector<vk::SubpassDescription> l_subpasses;
		l_subpasses.allocate(1);
		l_subpasses.Size = 1;
		{
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

			vk::AttachmentDescription& l_depth_attachment = l_attachments[1];
			l_depth_attachment = vk::AttachmentDescription();
			l_depth_attachment.setFormat(p_depth_format);
			l_depth_attachment.setSamples(vk::SampleCountFlagBits::e1);
			l_depth_attachment.setLoadOp(vk::AttachmentLoadOp::eClear);
			l_depth_attachment.setStoreOp(vk::AttachmentStoreOp::eDontCare);
			l_depth_attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
			l_depth_attachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
			l_depth_attachment.setInitialLayout(vk::ImageLayout::eUndefined);
			l_depth_attachment.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

			vk::AttachmentReference l_color_attachment_ref;
			l_color_attachment_ref.setAttachment(0);
			l_color_attachment_ref.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

			vk::AttachmentReference l_depth_atttachment_ref;
			l_depth_atttachment_ref.setAttachment(1);
			l_depth_atttachment_ref.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

			vk::SubpassDescription& l_color_subpass = l_subpasses[0];
			l_color_subpass = vk::SubpassDescription();
			l_color_subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
			l_color_subpass.setColorAttachmentCount(1);
			l_color_subpass.setPColorAttachments(&l_color_attachment_ref);
			l_color_subpass.setPDepthStencilAttachment(&l_depth_atttachment_ref);

			vk::RenderPassCreateInfo l_renderpass_create_info;
			l_renderpass_create_info.setAttachmentCount((uint32_t)l_attachments.Size);
			l_renderpass_create_info.setPAttachments(l_attachments.Memory);
			l_renderpass_create_info.setSubpassCount((uint32_t)l_subpasses.Size);
			l_renderpass_create_info.setPSubpasses(l_subpasses.Memory);

			this->l_render_pass = p_device.device.createRenderPass(l_renderpass_create_info);
		}
		l_subpasses.free();
		l_subpasses.free();
	}

	void dispose(const Device& p_device)
	{
		p_device.device.destroyRenderPass(this->l_render_pass);
	}
};

struct CommandBuffer
{
	vk::CommandBuffer command_buffer = nullptr;
	const vk::CommandPool* pool = nullptr;
	const vk::Queue* queue = nullptr;
	bool hasBegun = false;

	CommandBuffer() {}

	inline CommandBuffer(const Device& p_device, const vk::CommandPool& p_command_pool, const vk::Queue& p_queue)
	{
		vk::CommandBufferAllocateInfo l_command_buffer_allocate_info;
		this->pool = &p_command_pool;
		this->queue = &p_queue;
		l_command_buffer_allocate_info.setCommandPool(*this->pool);
		l_command_buffer_allocate_info.setLevel(vk::CommandBufferLevel::ePrimary);
		l_command_buffer_allocate_info.setCommandBufferCount(1);
		auto l_command_buffers = p_device.device.allocateCommandBuffers(l_command_buffer_allocate_info);
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

	inline void submit()
	{
		this->end();
		vk::SubmitInfo l_wait_for_end_submit;
		l_wait_for_end_submit.setCommandBufferCount(1);
		l_wait_for_end_submit.setPCommandBuffers(&this->command_buffer);
		this->queue->submit(1, &l_wait_for_end_submit, nullptr);
	}

	inline void flush_sync()
	{
		this->end();
		vk::SubmitInfo l_wait_for_end_submit;
		l_wait_for_end_submit.setCommandBufferCount(1);
		l_wait_for_end_submit.setPCommandBuffers(&this->command_buffer);
		this->queue->submit(1, &l_wait_for_end_submit, nullptr);
		this->queue->waitIdle();
	}

	inline void dispose(const Device& p_device, vk::CommandPool& p_command_pool)
	{
		this->queue->waitIdle();
		p_device.device.freeCommandBuffers(p_command_pool, 1, &this->command_buffer);
		this->command_buffer = nullptr;
	}
};

struct Image
{
	vk::Image ImageMemory;
	vk::ImageSubresource ImageSubresource;
	vk::ImageSubresourceRange ImageSubresourceRange;
	Vector<2, int> ImageSize;

	inline Image() {};
	inline Image(vk::Image& p_image_memory, vk::ImageSubresource& p_image_subresource, vk::ImageSubresourceRange& p_image_subresource_range, Vector<2, int>& p_image_size,
		size_t p_pxixel_component_size, short int p_channel_nb) : ImageMemory{ p_image_memory }, ImageSubresource{ p_image_subresource }, ImageSubresourceRange{ p_image_subresource_range }, ImageSize{ p_image_size }
	{

	};
};

struct TextureSamplers
{
	vk::Sampler Default;

	inline void allocate(Device& p_device)
	{
		vk::SamplerCreateInfo l_sampler_create_info;
		l_sampler_create_info.setMagFilter(vk::Filter::eNearest);
		l_sampler_create_info.setMinFilter(vk::Filter::eNearest);
		l_sampler_create_info.setAddressModeU(vk::SamplerAddressMode::eRepeat);
		l_sampler_create_info.setAddressModeV(vk::SamplerAddressMode::eRepeat);
		l_sampler_create_info.setAddressModeW(vk::SamplerAddressMode::eRepeat);
		l_sampler_create_info.setBorderColor(vk::BorderColor::eIntOpaqueBlack);
		l_sampler_create_info.setUnnormalizedCoordinates(false);
		l_sampler_create_info.setCompareEnable(false);
		l_sampler_create_info.setCompareOp(vk::CompareOp::eAlways);
		l_sampler_create_info.setMipmapMode(vk::SamplerMipmapMode::eLinear);
		l_sampler_create_info.setMipLodBias(0.0f);
		l_sampler_create_info.setMaxLod(0.0f);
		l_sampler_create_info.setMinLod(0.0f);
		this->Default = p_device.device.createSampler(l_sampler_create_info);
	};

	inline void free(Device& p_device)
	{
		p_device.device.destroySampler(this->Default);
	};
};

struct MappedMemory2
{
	char* mapped_data = nullptr;
	size_t element_count = -1;

	inline void map(Device& p_device, const GPUMemoryWithOffset& p_memory, const size_t p_element_count)
	{
		if (!this->isMapped())
		{
			GeneralPurposeHeapMemoryChunk* l_chunk;
			p_device.devicememory_allocator.resolve_allocated_chunk(p_memory, &l_chunk);
			this->mapped_data = p_memory.original_mapped_memory + l_chunk->offset;
			this->element_count = p_element_count;
		}
	};

	inline void unmap(const Device& p_device, vk::DeviceMemory p_memory)
	{
		if (this->isMapped())
		{
			this->mapped_data = nullptr;
			this->element_count = -1;
		}
	}

	inline void copyFrom(const GPUMemoryWithOffset& p_memory, const GPtr& p_from)
	{
		memcpy((void*)this->mapped_data, (const void*)p_from.ptr, (this->element_count * p_from.element_size));
	};

	inline bool isMapped()
	{
		return this->mapped_data != nullptr;
	};
};

struct GPUBufferMemoryHost2
{
	GPUMemoryWithOffset memory;
	vk::Buffer buffer;
	MappedMemory2 mapped_memory;
	size_t capacity;

	inline void allocate(Device& p_device, size_t p_element_count, size_t p_element_size, vk::BufferUsageFlags p_usageflags)
	{
		this->capacity = p_element_count;

		vk::BufferCreateInfo l_buffercreate_info;
		l_buffercreate_info.setUsage(vk::BufferUsageFlags(p_usageflags | vk::BufferUsageFlagBits::eTransferSrc));
		l_buffercreate_info.setSize(p_element_count * p_element_size);

		this->buffer = p_device.device.createBuffer(l_buffercreate_info);

		vk::MemoryRequirements l_requirements = p_device.device.getBufferMemoryRequirements(this->buffer);

		p_device.devicememory_allocator.allocate_element(l_requirements.size, l_requirements.alignment, p_device.getMemoryTypeIndex(l_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostCoherent),
			p_device.device, &this->memory);

		this->map(p_device, p_element_count);
		this->bind(p_device);
	};

	inline void dispose(Device& p_device)
	{
		if (this->mapped_memory.isMapped())
		{
			this->unmap(p_device);
		}

		p_device.devicememory_allocator.free_element(this->memory);
		p_device.device.destroyBuffer(this->buffer);
		this->buffer = nullptr;
	};


	inline void bind(Device& p_device)
	{
		GeneralPurposeHeapMemoryChunk* l_chunk;
		p_device.devicememory_allocator.resolve_allocated_chunk(this->memory, &l_chunk);
		p_device.device.bindBufferMemory(this->buffer, this->memory.original_memory, l_chunk->offset);
	};

	inline void map(Device& p_device, size_t p_size_count)
	{
		this->mapped_memory.map(p_device, this->memory, p_size_count);
	};

	inline void unmap(const Device& p_device)
	{
		this->mapped_memory.unmap(p_device, this->memory.original_memory);
	};

	inline void push(const GPtr& p_from)
	{
		this->mapped_memory.copyFrom(this->memory, p_from);
	};
};

struct GPUMemoryDeferredCommandToken
{
	com::PoolToken completion_token = -1;
	size_t execution_index = -1;

	template<class StagedBufferWriteCommandAllocator>
	inline void allocate_completiontoken(StagedBufferWriteCommandAllocator& p_stagedbuffer_allocator)
	{
		this->completion_token = p_stagedbuffer_allocator.allocate_completiontoken();
	};

	template<class StagedBufferWriteCommandAllocator>
	inline void free(StagedBufferWriteCommandAllocator& p_stagedbuffer_allocator)
	{
		// The GPUWrite buffer has been disposed, but there is a staging buffer waiting to be executed.
		// This happens if buffer is allocated and disposted at the same frame.
		if (this->isWaitingForStaging(p_stagedbuffer_allocator))
		{
			// We invalidate staging command
			p_stagedbuffer_allocator.invalidate_command(this->execution_index);
		}

		p_stagedbuffer_allocator.release_completion_token(this->completion_token);
	};

	template<class StagedBufferWriteCommandAllocator>
	inline bool isWaitingForStaging(StagedBufferWriteCommandAllocator& p_stagedbuffer_allocator)
	{
		return !p_stagedbuffer_allocator.resolve_completion_token(this->completion_token);
	};
};

template<class ElementType>
struct GPUBufferMemoryGPU2
{
	GPUMemoryWithOffset memory;
	GPUMemoryDeferredCommandToken staging_writing;
	vk::Buffer buffer;
	size_t capacity;

	template<class StagedBufferWriteCommandAllocator>
	inline void allocate(StagedBufferWriteCommandAllocator& p_stagedbuffer_allocator,
		size_t p_element_number, vk::BufferUsageFlags p_usageflags, Device& p_device)
	{
		this->capacity = p_element_number;

		this->staging_writing.allocate_completiontoken(p_stagedbuffer_allocator);

		//Actual buffer
		vk::BufferCreateInfo l_buffercreate_info;
		l_buffercreate_info.setUsage(p_usageflags | vk::BufferUsageFlags(vk::BufferUsageFlagBits::eTransferDst));
		l_buffercreate_info.setSize(p_element_number * sizeof(ElementType));
		this->buffer = p_device.device.createBuffer(l_buffercreate_info);

		vk::MemoryRequirements l_requirements = p_device.device.getBufferMemoryRequirements(this->buffer);

		p_device.devicememory_allocator.allocate_element(l_requirements.size, l_requirements.alignment, p_device.getMemoryTypeIndex(l_requirements.memoryTypeBits, vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eDeviceLocal)),
			p_device.device, &this->memory);

		this->bind(p_device);
	};

	inline void bind(Device& p_device)
	{
		GeneralPurposeHeapMemoryChunk* l_chunk;
		p_device.devicememory_allocator.resolve_allocated_chunk(this->memory, &l_chunk);
		p_device.device.bindBufferMemory(this->buffer, this->memory.original_memory, l_chunk->offset);
	};

	template<class StagedBufferWriteCommandAllocator>
	inline void push(const ElementType* p_source, Device& p_device, StagedBufferWriteCommandAllocator& p_stagedbuffer_allocator)
	{
		this->staging_writing.execution_index = p_stagedbuffer_allocator.allocate_stagincommand<ElementType>(this->buffer, this->capacity, p_source, this->staging_writing.completion_token, p_device);
	};

	template<class StagedBufferWriteCommandAllocator>
	inline void dispose(Device& p_device, StagedBufferWriteCommandAllocator& p_stagedbuffer_allocator)
	{
		this->staging_writing.free(p_stagedbuffer_allocator);

		p_device.devicememory_allocator.free_element(this->memory);
		p_device.device.destroyBuffer(this->buffer);
		this->buffer = nullptr;
	};

};

template<class ElementType>
struct GPUImageMemoryGPU2
{
	GPUMemoryWithOffset memory;
	GPUMemoryDeferredCommandToken image_writing;
	vk::Image buffer;
	size_t capacity;

	template<class ImageBufferWriteCommandAllocator>
	inline void allocate(ImageBufferWriteCommandAllocator& p_imagewrite_allocator,
		const vk::ImageCreateInfo& p_imagecreateinfo, Device& p_device)
	{
		this->capacity = p_imagecreateinfo.extent.width * p_imagecreateinfo.extent.height * p_imagecreateinfo.extent.depth;
		this->buffer = p_device.device.createImage(p_imagecreateinfo);

		this->image_writing.allocate_completiontoken(p_imagewrite_allocator);

		//Actual buffer
		vk::MemoryRequirements l_memory_requirements = p_device.device.getImageMemoryRequirements(this->buffer);
		p_device.devicememory_allocator.allocate_element(l_memory_requirements.size, l_memory_requirements.alignment, p_device.getMemoryTypeIndex(l_memory_requirements.memoryTypeBits, vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eDeviceLocal)),
			p_device.device, &this->memory);

		this->bind(p_device);
	};

	inline void bind(Device& p_device)
	{
		GeneralPurposeHeapMemoryChunk* l_chunk;
		p_device.devicememory_allocator.resolve_allocated_chunk(this->memory, &l_chunk);
		p_device.device.bindImageMemory(this->buffer, this->memory.original_memory, l_chunk->offset);
	};

	template<class ImageBufferWriteCommandAllocator>
	inline void push(const ElementType* p_source, vk::ImageSubresource& p_image_subresource, vk::ImageSubresourceRange& p_image_subresource_range,
		Vector<2, int>& p_image_size, size_t p_pixel_size, Device& p_device, ImageBufferWriteCommandAllocator& p_imagewrite_allocator)
	{
        
		this->image_writing.execution_index = p_imagewrite_allocator.allocate_stagedimagewritecommand(this->buffer, p_image_subresource, p_image_subresource_range,
			p_image_size, (const char*)p_source, p_pixel_size, this->image_writing.completion_token, p_device);
	};

	template<class ImageBufferWriteCommandAllocator>
	inline void dispose(Device& p_device, ImageBufferWriteCommandAllocator& p_imagewrite_allocator)
	{
		this->image_writing.free(p_imagewrite_allocator);

		p_device.devicememory_allocator.free_element(this->memory);
		p_device.device.destroyImage(this->buffer);
		this->buffer = nullptr;
	};
};



template<class ElementType>
struct VertexMemory : public GPUBufferMemoryGPU2<ElementType>
{
	template<class StagedBufferWriteCommandAllocator>
	inline void allocate(size_t p_element_number, Device& p_device, StagedBufferWriteCommandAllocator& p_stagedbuffer_allocator)
	{
		GPUBufferMemoryGPU2<ElementType>::allocate(p_stagedbuffer_allocator, p_element_number, vk::BufferUsageFlags(vk::BufferUsageFlagBits::eVertexBuffer), p_device);
	};
};

template<class ElementType>
struct IndexMemory : public GPUBufferMemoryGPU2<ElementType>
{
	template<class StagedBufferWriteCommandAllocator>
	inline void allocate(size_t p_element_number, Device& p_device, StagedBufferWriteCommandAllocator& p_stagedbuffer_allocator)
	{
		GPUBufferMemoryGPU2<ElementType>::allocate(p_stagedbuffer_allocator, p_element_number, vk::BufferUsageFlags(vk::BufferUsageFlagBits::eIndexBuffer), p_device);
	};
};

struct UniformMemory_HostWrite : public GPUBufferMemoryHost2
{
	inline void allocate(size_t p_element_number, size_t p_element_size, Device& p_device)
	{
		GPUBufferMemoryHost2::allocate(p_device, p_element_number, p_element_size, vk::BufferUsageFlags(vk::BufferUsageFlagBits::eUniformBuffer));
	};
};

template<class ElementType>
struct TUniformMemory_HostWrite : public UniformMemory_HostWrite
{
	inline void allocate(size_t p_element_number, Device& p_device)
	{
		UniformMemory_HostWrite::allocate(p_element_number, sizeof(ElementType), p_device);
	};

	inline void push(const ElementType* p_from)
	{
		UniformMemory_HostWrite::push(GPtr::fromType(p_from));
	};
};

struct GPUOnlyImageMemory : public GPUImageMemoryGPU2<char> {};


namespace RenderEnums
{
	enum DeferredCommandBufferExecutionType
	{
		UNDEFINED = 0,
		STAGING_BUFFER = 1,
		STAGING_IMAGE = 2,
		IMAGE_TRANSITION = 3
	};
}


struct StagedBufferWriteCommand
{
	inline static const RenderEnums::DeferredCommandBufferExecutionType Type = RenderEnums::DeferredCommandBufferExecutionType::STAGING_BUFFER;

	vk::Buffer buffer;
	size_t buffer_size;
	bool isAborted = false;
	GPUBufferMemoryHost2 staging_memory;

	com::PoolToken completion_token;

	inline void process_buffered(CommandBuffer& p_commandbuffer, com::Pool<bool>& p_stagin_completions, const Device& p_device)
	{
		if (!this->isAborted)
		{
			vk::BufferCopy l_buffer_copy_regions;
			l_buffer_copy_regions.setSize(buffer_size);
			p_commandbuffer.command_buffer.copyBuffer(this->staging_memory.buffer, this->buffer, 1, &l_buffer_copy_regions);
			p_stagin_completions.resolve(this->completion_token) = true;
		}
	};

	inline void dispose(Device& p_device)
	{
		this->staging_memory.dispose(p_device);
	};

	inline void invalidate()
	{
		// this->original_buffer_mappedmemory = nullptr;
		this->isAborted = true;
	};
};

template<class DeferredCommandType>
struct DeferredBufferCommands
{
	com::Vector<DeferredCommandType> commands;
	com::Vector<DeferredCommandType> lastframe_commands;

	inline void free()
	{
		this->commands.free();
		this->lastframe_commands.free();
	}

	inline void push_to_lastframe()
	{
		this->lastframe_commands.insert_at(com::MemorySlice<DeferredCommandType>(*this->commands.Memory, this->commands.Size), 0);
		this->commands.clear();
	};

	inline void dispose_lastframe_commands(Device& p_device)
	{
		for (size_t i = 0; i < this->lastframe_commands.Size; i++)
		{
			this->lastframe_commands[i].dispose(p_device);
		}
	}

	inline void dispose_all_commands(Device& p_device)
	{
		this->dispose_lastframe_commands(p_device);
		for (size_t i = 0; i < this->commands.Size; i++)
		{
			this->commands[i].dispose(p_device);
		}
	};
};

struct StagedImageWriteCommand
{
	inline static const RenderEnums::DeferredCommandBufferExecutionType Type = RenderEnums::DeferredCommandBufferExecutionType::STAGING_IMAGE;

	Image image;

	bool isAborted = false;
	GPUBufferMemoryHost2 staging_memory;

	com::PoolToken completion_token;

	inline void process_buffered(CommandBuffer& p_commandbuffer, com::Pool<bool>& p_stagin_completions, const Device& p_device)
	{
		if (!this->isAborted)
		{
			vk::BufferImageCopy l_buffer_image_copy;
			l_buffer_image_copy.setBufferOffset(0);
			l_buffer_image_copy.setBufferRowLength(0);
			l_buffer_image_copy.setBufferImageHeight(0);
			l_buffer_image_copy.imageSubresource.setAspectMask(this->image.ImageSubresource.aspectMask);
			l_buffer_image_copy.imageSubresource.setMipLevel(this->image.ImageSubresource.mipLevel);
			l_buffer_image_copy.imageSubresource.setBaseArrayLayer(this->image.ImageSubresourceRange.baseArrayLayer);
			l_buffer_image_copy.imageSubresource.setLayerCount(this->image.ImageSubresourceRange.layerCount);

			l_buffer_image_copy.setImageOffset(vk::Offset3D(0, 0, 0));
			l_buffer_image_copy.setImageExtent(vk::Extent3D(this->image.ImageSize.x, this->image.ImageSize.y, 1));

			p_commandbuffer.command_buffer.copyBufferToImage(this->staging_memory.buffer, this->image.ImageMemory, vk::ImageLayout::eTransferDstOptimal, 1, &l_buffer_image_copy);

			p_stagin_completions.resolve(this->completion_token) = true;
		}
	};

	inline void dispose(Device& p_device)
	{
		this->staging_memory.dispose(p_device);
	};

	inline void invalidate()
	{
		this->isAborted = true;
	};
};

struct TextureLayoutTransitionBarrierConfiguration {
	vk::AccessFlags src_access_mask;
	vk::AccessFlags dst_access_mask;

	vk::PipelineStageFlags src_stage;
	vk::PipelineStageFlags dst_stage;
};

template<vk::ImageLayout SourceLayout, vk::ImageLayout TargetLayout>
struct TransitionBarrierConfigurationBuilder
{
	inline static TextureLayoutTransitionBarrierConfiguration build();
};

template<>
struct TransitionBarrierConfigurationBuilder<vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal> {

	inline static TextureLayoutTransitionBarrierConfiguration build()
	{
		TextureLayoutTransitionBarrierConfiguration l_transition;
		l_transition.src_access_mask = vk::AccessFlags(0);
		l_transition.dst_access_mask = vk::AccessFlags(vk::AccessFlagBits::eTransferWrite);

		l_transition.src_stage = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eTopOfPipe);
		l_transition.dst_stage = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eTransfer);
		return l_transition;
	};
};

template<>
struct TransitionBarrierConfigurationBuilder<vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal> {

	inline static TextureLayoutTransitionBarrierConfiguration build()
	{
		TextureLayoutTransitionBarrierConfiguration l_transition;
		l_transition.src_access_mask = vk::AccessFlags(vk::AccessFlagBits::eTransferWrite);
		l_transition.dst_access_mask = vk::AccessFlags(vk::AccessFlagBits::eShaderRead);

		l_transition.src_stage = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eTransfer);
		l_transition.dst_stage = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eFragmentShader);
		return l_transition;
	};
};

struct TextureLayoutTransitionCommand
{
	inline static const RenderEnums::DeferredCommandBufferExecutionType Type = RenderEnums::DeferredCommandBufferExecutionType::IMAGE_TRANSITION;

	vk::ImageLayout source_layout;
	vk::ImageLayout target_layout;
	TextureLayoutTransitionBarrierConfiguration transition_barrier;
	vk::Image image;
	vk::ImageSubresourceRange image_subresource;

	com::PoolToken completion_token;
	bool isAborted = false;

	inline void process_buffered(CommandBuffer& p_commandbuffer, com::Pool<bool>& p_stagin_completions)
	{
		if (!this->isAborted)
		{
			vk::ImageMemoryBarrier l_image_memory_barrier;
			l_image_memory_barrier.setOldLayout(this->source_layout);
			l_image_memory_barrier.setNewLayout(this->target_layout);
			l_image_memory_barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
			l_image_memory_barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
			l_image_memory_barrier.setImage(this->image);
			l_image_memory_barrier.setSubresourceRange(this->image_subresource);

			l_image_memory_barrier.setSrcAccessMask(this->transition_barrier.src_access_mask);
			l_image_memory_barrier.setDstAccessMask(this->transition_barrier.dst_access_mask);

			p_commandbuffer.command_buffer.pipelineBarrier(this->transition_barrier.src_stage, this->transition_barrier.dst_stage,
				vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &l_image_memory_barrier);

			p_stagin_completions.resolve(this->completion_token) = true;
		}
	};

	inline void invalidate()
	{
		this->isAborted = true;
	};
};

struct DeferredCommandBufferExecution
{
	struct DeferredCommandBufferExecutionEntry
	{
		RenderEnums::DeferredCommandBufferExecutionType Type = RenderEnums::DeferredCommandBufferExecutionType::UNDEFINED;
		size_t CommandIndex = -1;

		inline DeferredCommandBufferExecutionEntry(const RenderEnums::DeferredCommandBufferExecutionType type,
			const size_t p_index) {
			this->Type = type;
			this->CommandIndex = p_index;
		};

	};

	typedef size_t DeferredCommandBufferExecutionIndex;

	com::Vector<DeferredCommandBufferExecutionEntry> command_execution_order;
	com::Pool<bool> commands_completion;

	DeferredBufferCommands<StagedBufferWriteCommand> stagingbuffers;
	DeferredBufferCommands<StagedImageWriteCommand> stagingimages;
	DeferredBufferCommands<TextureLayoutTransitionCommand> texturelayouttransitions;

	inline void allocate(const Device& p_device) {

		this->commands_completion.allocate(0);
	};

	inline void dispose(Device& p_device)
	{
		this->command_execution_order.free();
		this->stagingbuffers.free();
		this->stagingimages.free();
		this->texturelayouttransitions.free();
		this->commands_completion.free();
	};

	inline com::PoolToken allocate_completiontoken()
	{
		return this->commands_completion.alloc_element(true);
	};

	template<class DefferedCommandType, class DeferredBufferCommandsType>
	inline DeferredCommandBufferExecutionIndex push_command(DefferedCommandType& p_command, DeferredBufferCommandsType& p_commands)
	{
		p_commands.commands.push_back(p_command);
		size_t l_command_index = p_commands.commands.Size - 1;
		DeferredCommandBufferExecutionEntry l_entry = DeferredCommandBufferExecutionEntry(DefferedCommandType::Type, l_command_index);
		this->command_execution_order.push_back(l_entry);
		return this->command_execution_order.Size - 1;
	};

	template<class ElementType>
	inline DeferredCommandBufferExecution::DeferredCommandBufferExecutionIndex allocate_stagincommand(
		vk::Buffer p_buffer, size_t p_buffer_element_count, const ElementType* p_source, com::PoolToken& p_completion_token, Device& p_device)
	{
		StagedBufferWriteCommand l_command;
		l_command.buffer = p_buffer;
		l_command.buffer_size = p_buffer_element_count * sizeof(ElementType);
		l_command.staging_memory.allocate(p_device, p_buffer_element_count, sizeof(ElementType), vk::BufferUsageFlags());
		l_command.staging_memory.push(GPtr::fromType(p_source));
		l_command.completion_token = p_completion_token;

		this->commands_completion.resolve(p_completion_token) = false;

		return this->push_command(l_command, this->stagingbuffers);
	};

	inline DeferredCommandBufferExecution::DeferredCommandBufferExecutionIndex allocate_stagedimagewritecommand(
		vk::Image& p_image, vk::ImageSubresource& p_image_subresource,
		vk::ImageSubresourceRange& p_image_subresource_range, Vector<2, int>& p_image_size,
		const char* p_source, size_t p_pixel_size, com::PoolToken& p_completion_token, Device& p_device)
	{
		StagedImageWriteCommand l_command;
		l_command.image.ImageSize = p_image_size;
		l_command.image.ImageMemory = p_image;
		l_command.image.ImageSubresource = p_image_subresource;
		l_command.image.ImageSubresourceRange = p_image_subresource_range;

		l_command.staging_memory.allocate(p_device, (p_image_size.x * p_image_size.y * p_pixel_size), 1, vk::BufferUsageFlags());
		l_command.staging_memory.push(GPtr::fromType(p_source));

		l_command.completion_token = p_completion_token;

		this->commands_completion.resolve(p_completion_token) = false;
		return this->push_command(l_command, this->stagingimages);
	};

	inline DeferredCommandBufferExecution::DeferredCommandBufferExecutionIndex allocate_texturelayouttransitioncommand(vk::Image p_image,
		vk::ImageSubresourceRange p_image_subresource,
		vk::ImageLayout p_source_layout, vk::ImageLayout p_target_layout,
		TextureLayoutTransitionBarrierConfiguration& p_transition_barrier,
		com::PoolToken& p_completion_token)
	{
		TextureLayoutTransitionCommand l_command;
		l_command.image = p_image;
		l_command.image_subresource = p_image_subresource;
		l_command.source_layout = p_source_layout;
		l_command.target_layout = p_target_layout;
		l_command.transition_barrier = p_transition_barrier;
		l_command.completion_token = p_completion_token;

		this->commands_completion.resolve(p_completion_token) = false;
		return this->push_command(l_command, this->texturelayouttransitions);
	};


	inline void invalidate_command(DeferredCommandBufferExecutionIndex p_command_index)
	{
		DeferredCommandBufferExecutionEntry& l_current_command_order = this->command_execution_order[p_command_index];
		switch (l_current_command_order.Type)
		{
		case StagedBufferWriteCommand::Type:
		{
			this->stagingbuffers.commands[l_current_command_order.CommandIndex].invalidate();
		}
		break;
		case TextureLayoutTransitionCommand::Type:
		{
			this->texturelayouttransitions.commands[l_current_command_order.CommandIndex].invalidate();
		}
		break;
		case StagedImageWriteCommand::Type:
		{
			this->stagingimages.commands[l_current_command_order.CommandIndex].invalidate();
		}
		break;
		default:
			break;
		}
	};

	inline void process_all_buffers(CommandBuffer& p_commandbuffer, Device& p_device)
	{
		this->dispose_lastframe_commands(p_device);

		if (this->command_execution_order.Size > 0)
		{
			// this->command_buffer.begin();

			for (size_t i = 0; i < this->command_execution_order.Size; i++)
			{
				DeferredCommandBufferExecutionEntry& l_current_command_order = this->command_execution_order[i];
				switch (l_current_command_order.Type)
				{
				case StagedBufferWriteCommand::Type:
				{
					StagedBufferWriteCommand& l_command = this->stagingbuffers.commands[l_current_command_order.CommandIndex];
					l_command.process_buffered(p_commandbuffer, this->commands_completion, p_device);
				}
				break;
				case TextureLayoutTransitionCommand::Type:
				{
					TextureLayoutTransitionCommand& l_command = this->texturelayouttransitions.commands[l_current_command_order.CommandIndex];
					l_command.process_buffered(p_commandbuffer, this->commands_completion);
				}
				break;
				case StagedImageWriteCommand::Type:
				{
					StagedImageWriteCommand& l_command = this->stagingimages.commands[l_current_command_order.CommandIndex];
					l_command.process_buffered(p_commandbuffer, this->commands_completion, p_device);
				}
				break;
				default:
					break;
				}
			}

			this->stagingbuffers.push_to_lastframe();
			this->stagingimages.push_to_lastframe();

			this->command_execution_order.clear();
		}

	};

	inline bool& resolve_completion_token(const com::PoolToken& p_completion_token)
	{
		return this->commands_completion.resolve(p_completion_token);
	};

	inline void release_completion_token(const com::PoolToken& p_completion_token)
	{
		this->commands_completion.release_element(p_completion_token);
	};

private:
	inline void dispose_lastframe_commands(Device& p_device)
	{
		this->stagingbuffers.dispose_lastframe_commands(p_device);
		this->stagingimages.dispose_lastframe_commands(p_device);
	};
};


struct DeferredCommandbufferExecutionToken
{
	size_t QueueIndex = -1;
	com::PoolToken CompletionToken = -1;

	inline DeferredCommandbufferExecutionToken() {}
	inline DeferredCommandbufferExecutionToken(const size_t p_queue_index, const com::PoolToken p_completiontoken)
	{
		this->QueueIndex = p_queue_index;
		this->CompletionToken = p_completiontoken;
	};

	inline void invalidate(DeferredCommandBufferExecution& p_execution)
	{
		p_execution.invalidate_command(this->QueueIndex);
		this->QueueIndex = -1;
	};

	inline bool isCompleted(DeferredCommandBufferExecution& p_execution)
	{
		return p_execution.resolve_completion_token(this->CompletionToken);
	};

	inline void free(DeferredCommandBufferExecution& p_execution)
	{
		if (this->isCompleted(p_execution))
		{
			this->invalidate(p_execution);
		};
		this->CompletionToken = -1;
	};
};


//TODO -> dissociating the present image (which is the final texture displayed to the application) with render render target image (which is the internal image buffer that is used to render models onto)
struct SwapChain
{
public:

	typedef struct _SwapChainBuffers {
		VkImage image;
		VkImageView view;
	} SwapChainBuffer;

	vk::SwapchainKHR handle;

	vk::SurfaceFormatKHR surface_format;
	vk::Format depth_format;

	vk::PresentModeKHR present_mode;
	vk::Extent2D extend;
	uint32_t image_count;

	std::vector<vk::Image> images;
	com::Vector<SwapChainBuffer> buffers;

	GPUOnlyImageMemory depth_image;
	vk::ImageView depth_image_view;

	SwapChainBuffer depth_stencil;

	RenderPass renderpass;

	com::Vector<vk::Framebuffer> framebuffers;

private:
	const vk::Instance* instance;
	const vk::PhysicalDevice* physicalDevice;
	Device* device;
	const vk::SurfaceKHR* surface;
	const RenderWindow* window;

	vk::SurfaceCapabilitiesKHR surface_capabilities;

public:
	inline SwapChain() = default;

	inline void init(const vk::Instance& p_instance, Device& p_device, DeferredCommandBufferExecution& p_staging_commands,
		const vk::PhysicalDevice& p_physical_device,
		const vk::SurfaceKHR& p_surface, const RenderWindow& p_window)
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
		this->handle = this->device->device.createSwapchainKHR(l_swapchain_create_info);

		this->create_images();
		this->create_depth_image(p_staging_commands);
		this->createRenderPass(p_device);
		this->create_framebuffers(p_device);
	}

	inline void dispose(DeferredCommandBufferExecution& p_staging_commands)
	{
		this->destroy_framebuffers();
		this->destroyRenderPass();
		this->destroy_depth_image(p_staging_commands);
		this->destroy_images();
		this->device->device.destroySwapchainKHR(this->handle);
	}

	inline uint32_t getNextImage(vk::Semaphore p_presentcomplete_semaphore)
	{
		return this->device->device.acquireNextImageKHR(this->handle, UINT64_MAX, p_presentcomplete_semaphore, nullptr).value;
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
		this->images = this->device->device.getSwapchainImagesKHR(this->handle);
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
			l_swapchainBuffer.view = this->device->device.createImageView(l_image_view_create_info);

			this->buffers.push_back(l_swapchainBuffer);
		}
	}

	inline void destroy_images()
	{
		for (int i = 0; i < this->buffers.Size; i++)
		{
			this->device->device.destroyImageView(this->buffers[i].view);
		}
		this->buffers.free();
	}

	inline void create_depth_image(DeferredCommandBufferExecution& p_staging_commands)
	{
		//TODO -> format check
		this->depth_format = vk::Format::eD16Unorm;

		vk::ImageCreateInfo l_depth_image_create_info;
		l_depth_image_create_info.setImageType(vk::ImageType::e2D);
		l_depth_image_create_info.setFormat(this->depth_format);
		l_depth_image_create_info.setExtent({ this->extend.width, this->extend.height, 1 });
		l_depth_image_create_info.setMipLevels(1);
		l_depth_image_create_info.setArrayLayers(1);
		l_depth_image_create_info.setSamples(vk::SampleCountFlagBits::e1);
		l_depth_image_create_info.setTiling(vk::ImageTiling::eOptimal);
		l_depth_image_create_info.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);
		l_depth_image_create_info.setInitialLayout(vk::ImageLayout::eUndefined);

		this->depth_image.allocate(p_staging_commands, l_depth_image_create_info, *this->device);

		vk::ImageViewCreateInfo l_depth_view_create_info;
		l_depth_view_create_info.setImage(this->depth_image.buffer);
		l_depth_view_create_info.setViewType(vk::ImageViewType::e2D);
		l_depth_view_create_info.setFormat(this->depth_format);
		l_depth_view_create_info.setComponents(vk::ComponentMapping());

		vk::ImageSubresourceRange l_image_subresource;
		l_image_subresource.setBaseMipLevel(0);
		l_image_subresource.setLevelCount(1);
		l_image_subresource.setBaseArrayLayer(0);
		l_image_subresource.setLayerCount(1);
		l_image_subresource.setAspectMask(vk::ImageAspectFlagBits::eDepth);
		l_depth_view_create_info.setSubresourceRange(l_image_subresource);

		this->depth_image_view = this->device->device.createImageView(l_depth_view_create_info);
	}

	inline void destroy_depth_image(DeferredCommandBufferExecution& p_staging_commands)
	{
		this->device->device.destroyImageView(this->depth_image_view);
		this->depth_image.dispose(*this->device, p_staging_commands);
	}

	inline void createRenderPass(const Device& p_device)
	{
		this->renderpass.create(p_device, this->surface_format, this->depth_format);
	}

	inline void destroyRenderPass()
	{
		this->renderpass.dispose(*this->device);
	}

	inline void create_framebuffers(const Device& p_device)
	{
		this->framebuffers.allocate(this->image_count);
		this->framebuffers.Size = this->framebuffers.Capacity;
		com::Vector<vk::ImageView> l_attachments;
		l_attachments.allocate(2);
		for (size_t i = 0; i < this->framebuffers.Size; i++)
		{
			l_attachments.Size = l_attachments.Capacity;
			l_attachments[0] = this->buffers[i].view;
			l_attachments[1] = this->depth_image_view;

			vk::FramebufferCreateInfo l_framebuffer_create;
			l_framebuffer_create.setRenderPass(this->renderpass.l_render_pass);
			l_framebuffer_create.setAttachmentCount((uint32_t)l_attachments.Size);
			l_framebuffer_create.setPAttachments(l_attachments.Memory);
			l_framebuffer_create.setWidth(this->window->Width);
			l_framebuffer_create.setHeight(this->window->Height);
			l_framebuffer_create.setLayers(1);

			this->framebuffers[i] = p_device.device.createFramebuffer(l_framebuffer_create);
		}
		l_attachments.free();
	}

	inline void destroy_framebuffers()
	{
		for (size_t i = 0; i < this->framebuffers.Size; i++)
		{
			this->device->device.destroyFramebuffer(this->framebuffers[i]);
		}
		this->framebuffers.free();
	}
};


struct ShaderParameterLayouts
{
	vk::DescriptorSetLayout uniformbuffer_vertex_layout;
	vk::DescriptorSetLayout uniformbuffer_layout;
	vk::DescriptorSetLayout texture_fragment_layout;
	
	inline void create_layouts(const Device& p_device)
	{
		{
			vk::DescriptorSetLayoutBinding l_layout_bindings[1];
			{
				vk::DescriptorSetLayoutBinding& l_camera_matrices_layout_binding = l_layout_bindings[0];
				l_camera_matrices_layout_binding.setBinding(0);
				l_camera_matrices_layout_binding.setDescriptorCount(1);
				l_camera_matrices_layout_binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
				l_camera_matrices_layout_binding.setStageFlags(vk::ShaderStageFlagBits::eVertex);
				l_camera_matrices_layout_binding.setPImmutableSamplers(nullptr);
			}

			vk::DescriptorSetLayoutCreateInfo l_descriptorset_layot_create;
			l_descriptorset_layot_create.setBindingCount(1);
			l_descriptorset_layot_create.setPBindings(l_layout_bindings);

			this->uniformbuffer_vertex_layout = p_device.device.createDescriptorSetLayout(l_descriptorset_layot_create);
		}
		{
			vk::DescriptorSetLayoutBinding l_layout_bindings[1];
			{
				vk::DescriptorSetLayoutBinding& l_camera_matrices_layout_binding = l_layout_bindings[0];
				l_camera_matrices_layout_binding.setBinding(0);
				l_camera_matrices_layout_binding.setDescriptorCount(1);
				l_camera_matrices_layout_binding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
				l_camera_matrices_layout_binding.setStageFlags(vk::ShaderStageFlagBits::eFragment);
				l_camera_matrices_layout_binding.setPImmutableSamplers(nullptr);
			}

			vk::DescriptorSetLayoutCreateInfo l_descriptorset_layot_create;
			l_descriptorset_layot_create.setBindingCount(1);
			l_descriptorset_layot_create.setPBindings(l_layout_bindings);

			this->texture_fragment_layout = p_device.device.createDescriptorSetLayout(l_descriptorset_layot_create);
		}
		{
			vk::DescriptorSetLayoutBinding l_layout_bindings[1];
			{
				vk::DescriptorSetLayoutBinding& l_camera_matrices_layout_binding = l_layout_bindings[0];
				l_camera_matrices_layout_binding.setBinding(0);
				l_camera_matrices_layout_binding.setDescriptorCount(1);
				l_camera_matrices_layout_binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
				l_camera_matrices_layout_binding.setStageFlags(vk::ShaderStageFlags(vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex));
				l_camera_matrices_layout_binding.setPImmutableSamplers(nullptr);
			}

			vk::DescriptorSetLayoutCreateInfo l_descriptorset_layot_create;
			l_descriptorset_layot_create.setBindingCount(1);
			l_descriptorset_layot_create.setPBindings(l_layout_bindings);

			this->uniformbuffer_layout = p_device.device.createDescriptorSetLayout(l_descriptorset_layot_create);
		}
	};

	inline void destroy_layouts(const Device& p_device)
	{
		p_device.device.destroyDescriptorSetLayout(uniformbuffer_vertex_layout);
		p_device.device.destroyDescriptorSetLayout(texture_fragment_layout);
		p_device.device.destroyDescriptorSetLayout(uniformbuffer_layout);
	};
};

struct RenderAPI
{
	vk::Instance instance;
	VkDebugUtilsMessengerEXT debugMessenger;

	Device device;
	ShaderParameterLayouts shaderparameter_layouts;

	vk::SurfaceKHR surface;

	SwapChain swap_chain;

	vk::CommandPool command_pool;
	vk::DescriptorPool descriptor_pool;

	com::Vector<CommandBuffer> draw_commandbuffers;
	DeferredCommandBufferExecution stagedbuffer_commands;

	TextureSamplers image_samplers;

	struct Sync
	{
		vk::Semaphore present_complete_semaphore;
		vk::Semaphore render_complete_semaphore;
		com::Vector<vk::Fence> draw_command_fences;
	};

	Sync synchronization;

	ValidationLayer validation_layer;



	RenderAPI() = default;

	inline void init(const RenderWindow& p_window)
	{
		this->createInstance();
		this->createDebugCallback();
		this->createSurface(p_window);
		this->device.getPhysicalDevice(this->instance, this->surface);
		this->device.createPhysicalDevice(this->validation_layer);
		this->createCommandBufferPool();
		this->create_stagin();

		this->createSwapChain(p_window);

		this->create_draw_commandbuffers();
		this->create_synchronization();
		this->create_descriptor_pool();
		this->create_global_descriptorset_layouts();
		this->create_imagesamplers();
	};

	inline void dispose()
	{
		this->destroy_imagesamplers();
		this->destroy_global_descriptorset_layouts();
		this->destroy_descriptor_pool();
		this->destroy_synchronization();
		this->destroy_draw_commandbuffers();

		this->destroySwapChain();
		this->destroy_stagin();
		this->destroyCommandBufferPool();
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

		com::Vector<const char*> l_extensions;
		{
			this->validationLayers(l_instance_create_info, this->validation_layer.layers);
			this->extensions(l_instance_create_info, l_extensions);
			this->instance = vk::createInstance(l_instance_create_info);
		}
		l_extensions.free();
	};

	inline void destroyInstance()
	{
		this->instance.destroy();
	};

	inline void validationLayers(vk::InstanceCreateInfo& p_instance_create_info, com::Vector<const char*>& p_validationLayers)
	{
#if !NDEBUG
		this->validation_layer.enabled = true;
#else
		this->validation_layer.enabled = false;
#endif
		if (this->validation_layer.enabled)
		{
			p_validationLayers.allocate(1);
			p_validationLayers.Size = p_validationLayers.Capacity;
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

		p_extensions.allocate(l_glfw_extension_count);

		auto l_memoryslice = com::MemorySlice<const char*>(*l_glfw_extensions, (size_t)l_glfw_extension_count);
		p_extensions.insert_at(l_memoryslice, 0);

		if (this->validation_layer.enabled)
		{
			p_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		p_instance_create_info.setEnabledExtensionCount((uint32_t)p_extensions.Size);
		p_instance_create_info.setPpEnabledExtensionNames(p_extensions.Memory);
	}

	inline void createDebugCallback()
	{
		if (this->validation_layer.enabled)
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
		if (this->validation_layer.enabled)
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

	inline void createSwapChain(const RenderWindow& p_window)
	{
		this->swap_chain.init(this->instance, this->device, this->stagedbuffer_commands, this->device.graphics_device, this->surface, p_window);
	}

	inline void destroySwapChain()
	{
		this->swap_chain.dispose(this->stagedbuffer_commands);
	}

	inline void createSurface(const RenderWindow& p_window)
	{

#ifdef _WIN32
		vk::Win32SurfaceCreateInfoKHR l_surfeca_create_info;
		l_surfeca_create_info.setHwnd(rdwindow::get_window_native(p_window.Handle));
		l_surfeca_create_info.setHinstance(GetModuleHandle(nullptr));
		this->surface = this->instance.createWin32SurfaceKHR(l_surfeca_create_info);
#endif
#ifdef linux
		vk::XlibSurfaceCreateInfoKHR l_surfeca_create_info;
		l_surfeca_create_info.setWindow(rdwindow::get_window_native(p_window.Handle));
		l_surfeca_create_info.setDpy(XOpenDisplay(nullptr));
		this->surface = this->instance.createXlibSurfaceKHR(l_surfeca_create_info);
#endif
	};

	inline void destroySurface()
	{
		this->instance.destroySurfaceKHR(this->surface);
	}

	inline void createCommandBufferPool()
	{
		vk::CommandPoolCreateInfo l_command_pool_create_info;
		l_command_pool_create_info.setQueueFamilyIndex(this->device.graphics_queue_family);
		l_command_pool_create_info.setFlags(vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer));
		this->command_pool = this->device.device.createCommandPool(l_command_pool_create_info);
	}

	inline void destroyCommandBufferPool()
	{
		this->device.device.destroyCommandPool(this->command_pool);
	}

	inline void create_draw_commandbuffers()
	{
		this->draw_commandbuffers.allocate(this->swap_chain.framebuffers.Size);
		this->draw_commandbuffers.Size = this->draw_commandbuffers.Capacity;
		for (int i = 0; i < this->draw_commandbuffers.Size; i++)
		{
			this->draw_commandbuffers[i] = CommandBuffer(this->device, this->command_pool, this->device.graphics_queue);
		}
	}

	inline void destroy_draw_commandbuffers()
	{
		for (int i = 0; i < this->draw_commandbuffers.Size; i++)
		{
			this->draw_commandbuffers[i].dispose(this->device, this->command_pool);
		}
		this->draw_commandbuffers.free();
	}

	inline void create_synchronization()
	{
		vk::SemaphoreCreateInfo l_semaphore_create_info;
		l_semaphore_create_info.setPNext(nullptr);

		this->synchronization.present_complete_semaphore = this->device.device.createSemaphore(l_semaphore_create_info);
		this->synchronization.render_complete_semaphore = this->device.device.createSemaphore(l_semaphore_create_info);

		vk::FenceCreateInfo l_fence_create_info;
		l_fence_create_info.setFlags(vk::FenceCreateFlags(vk::FenceCreateFlagBits::eSignaled));

		this->synchronization.draw_command_fences.allocate(this->draw_commandbuffers.Size);
		this->synchronization.draw_command_fences.Size = this->synchronization.draw_command_fences.Capacity;
		for (int i = 0; i < this->synchronization.draw_command_fences.Size; i++)
		{
			this->synchronization.draw_command_fences[i] = this->device.device.createFence(l_fence_create_info);
		}
	}

	inline void destroy_synchronization()
	{
		for (int i = 0; i < this->synchronization.draw_command_fences.Size; i++)
		{
			this->device.device.destroyFence(this->synchronization.draw_command_fences[i]);
		}

		this->device.device.destroySemaphore(this->synchronization.present_complete_semaphore);
		this->device.device.destroySemaphore(this->synchronization.render_complete_semaphore);

		this->synchronization.draw_command_fences.free();
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
		l_descriptor_pool_create_info.setFlags(vk::DescriptorPoolCreateFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet));
		l_descriptor_pool_create_info.setMaxSets(1000);

		this->descriptor_pool = this->device.device.createDescriptorPool(l_descriptor_pool_create_info);
	}

	inline void destroy_descriptor_pool()
	{
		this->device.device.destroyDescriptorPool(this->descriptor_pool);
	}

	inline void create_global_descriptorset_layouts()
	{
		this->shaderparameter_layouts.create_layouts(this->device);
	}

	inline void destroy_global_descriptorset_layouts()
	{
		this->shaderparameter_layouts.destroy_layouts(this->device);
	}

	inline void create_stagin()
	{
		this->stagedbuffer_commands.allocate(this->device);
	}

	inline void destroy_stagin()
	{
		this->stagedbuffer_commands.dispose(this->device);
	}

	inline void create_imagesamplers()
	{
		this->image_samplers.allocate(this->device);
	}

	inline void destroy_imagesamplers()
	{
		this->image_samplers.free(this->device);
	}
};


struct ShaderModule
{
	size_t key;
	vk::ShaderModule shader_module;
};

struct Shader
{
	size_t key;

	struct Step
	{
		vk::ShaderModule shader_module;
		std::string entry_name;
		vk::ShaderStageFlagBits stage;
	};

	vk::PipelineLayout pipeline_layout;
	vk::Pipeline pipeline;

	Shader() : pipeline_layout(nullptr), pipeline(nullptr)
	{

	}

	Shader(const size_t p_key, const ShaderModule& p_vertex_shader, const ShaderModule& p_fragment_shader, 
		const ShaderCompareOp::Type p_compare_op, const RenderPass& p_render_pass, const RenderAPI& p_render_api)
	{
		this->key = p_key;
		this->createPipelineLayout(p_render_api);
		this->createPipeline(p_render_api.device, p_render_pass, p_vertex_shader, p_fragment_shader, p_compare_op);
	}

	inline void dispose(const Device& p_device)
	{
		this->destroyPipeline(p_device);
		this->destroyPipelineLayout(p_device);

		this->pipeline = nullptr;
		this->pipeline_layout = nullptr;
	}

private:

	inline void createPipelineLayout(const RenderAPI& p_render_api)
	{
		vk::DescriptorSetLayout l_descriptorset_layouts[4];
		l_descriptorset_layouts[0] = p_render_api.shaderparameter_layouts.uniformbuffer_vertex_layout; //vertex
		l_descriptorset_layouts[1] = p_render_api.shaderparameter_layouts.uniformbuffer_vertex_layout; //uv
		l_descriptorset_layouts[2] = p_render_api.shaderparameter_layouts.texture_fragment_layout;
		l_descriptorset_layouts[3] = p_render_api.shaderparameter_layouts.uniformbuffer_layout;
		
		Array<vk::DescriptorSetLayout> l_descriptorset_layouts_arr = Array<vk::DescriptorSetLayout>(l_descriptorset_layouts, 4);
		vk::PipelineLayoutCreateInfo l_pipelinelayout_create_info;
		l_pipelinelayout_create_info.setSetLayoutCount((uint32_t)l_descriptorset_layouts_arr.Capacity);
		l_pipelinelayout_create_info.setPSetLayouts(l_descriptorset_layouts_arr.Memory);
		this->pipeline_layout = p_render_api.device.device.createPipelineLayout(l_pipelinelayout_create_info);
	}

	inline void destroyPipelineLayout(const Device& p_device)
	{
		p_device.device.destroyPipelineLayout(this->pipeline_layout);
	}

	inline void createPipeline(const Device& p_device, const RenderPass& p_renderPass,
		const ShaderModule& p_vertex_shader, const ShaderModule& p_fragment_shader, const ShaderCompareOp::Type p_compare_op)
	{
		com::Vector<vk::DynamicState> l_dynamicstates_enabled;
		l_dynamicstates_enabled.allocate(2);
		l_dynamicstates_enabled.Size = l_dynamicstates_enabled.Capacity;

		com::Vector<vk::VertexInputAttributeDescription> l_vertex_input_attributes;
		l_vertex_input_attributes.allocate(2);
		l_vertex_input_attributes.Size = l_vertex_input_attributes.Capacity;

		com::Vector<vk::PipelineShaderStageCreateInfo> l_shaderStages;
		l_shaderStages.allocate(2);
		l_shaderStages.Size = l_shaderStages.Capacity;
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
			l_rasterization_state.setFrontFace(vk::FrontFace::eCounterClockwise);
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


			l_dynamicstates_enabled[0] = vk::DynamicState::eViewport;
			l_dynamicstates_enabled[1] = vk::DynamicState::eScissor;
			vk::PipelineDynamicStateCreateInfo l_dynamicstates;
			l_dynamicstates.setDynamicStateCount((uint32_t)l_dynamicstates_enabled.Size);
			l_dynamicstates.setPDynamicStates(l_dynamicstates_enabled.Memory);

			vk::PipelineDepthStencilStateCreateInfo l_depthstencil_state;
			l_depthstencil_state.setDepthTestEnable(true);
			l_depthstencil_state.setDepthWriteEnable(true);
			l_depthstencil_state.setDepthCompareOp((vk::CompareOp)p_compare_op);
			l_depthstencil_state.setDepthBoundsTestEnable(false);
			vk::StencilOpState l_back;
			l_back.setCompareOp(vk::CompareOp::eAlways);
			l_back.setFailOp(vk::StencilOp::eKeep);
			l_back.setPassOp(vk::StencilOp::eKeep);
			l_depthstencil_state.setBack(l_back);
			l_depthstencil_state.setFront(l_back);
			l_depthstencil_state.setStencilTestEnable(false);

			vk::PipelineMultisampleStateCreateInfo l_multisample_state;
			l_multisample_state.setRasterizationSamples(vk::SampleCountFlagBits::e1);
			l_multisample_state.setPSampleMask(nullptr);



			vk::VertexInputBindingDescription l_vertex_input_binding;
			l_vertex_input_binding.setBinding(0);
			l_vertex_input_binding.setStride(sizeof(Vertex));
			l_vertex_input_binding.setInputRate(vk::VertexInputRate::eVertex);

			//vertex input
			l_vertex_input_attributes[0].setBinding(0);
			l_vertex_input_attributes[0].setLocation(0);
			l_vertex_input_attributes[0].setFormat(vk::Format::eR32G32B32Sfloat);
			l_vertex_input_attributes[0].setOffset(offsetof(Vertex, position));

			l_vertex_input_attributes[1].setBinding(0);
			l_vertex_input_attributes[1].setLocation(1);
			l_vertex_input_attributes[1].setFormat(vk::Format::eR32G32Sfloat);
			l_vertex_input_attributes[1].setOffset(offsetof(Vertex, uv));

			vk::PipelineVertexInputStateCreateInfo l_vertex_input_create;
			l_vertex_input_create.setVertexBindingDescriptionCount(1);
			l_vertex_input_create.setPVertexBindingDescriptions(&l_vertex_input_binding);
			l_vertex_input_create.setVertexAttributeDescriptionCount((uint32_t)l_vertex_input_attributes.Size);
			l_vertex_input_create.setPVertexAttributeDescriptions(l_vertex_input_attributes.Memory);

			Step vertex_shader;
			Step fragment_shader;

			vertex_shader.shader_module = p_vertex_shader.shader_module;
			vertex_shader.stage = vk::ShaderStageFlagBits::eVertex;
			vertex_shader.entry_name = "main";

			fragment_shader.shader_module = p_fragment_shader.shader_module;
			fragment_shader.stage = vk::ShaderStageFlagBits::eFragment;
			fragment_shader.entry_name = "main";


			vk::PipelineShaderStageCreateInfo& l_vertex_stage = l_shaderStages[0];
			l_vertex_stage = vk::PipelineShaderStageCreateInfo();
			*(vk::StructureType*)&l_vertex_stage.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
			l_vertex_stage.setStage(vk::ShaderStageFlagBits::eVertex);
			l_vertex_stage.setModule(vertex_shader.shader_module);
			l_vertex_stage.setPName(vertex_shader.entry_name.c_str());


			vk::PipelineShaderStageCreateInfo& l_fragment_stage = l_shaderStages[1];
			l_fragment_stage = vk::PipelineShaderStageCreateInfo();
			*(vk::StructureType*)&l_fragment_stage.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
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
			l_pipeline_graphcis_create_info.setPDepthStencilState(&l_depthstencil_state);
			l_pipeline_graphcis_create_info.setPDynamicState(&l_dynamicstates);

			this->pipeline = p_device.device.createGraphicsPipeline(vk::PipelineCache(), l_pipeline_graphcis_create_info);

		}
		l_dynamicstates_enabled.free();
		l_vertex_input_attributes.free();
		l_shaderStages.free();
	}

	inline void destroyPipeline(const Device& p_device)
	{
		p_device.device.destroyPipeline(this->pipeline);
	}
};


struct Mesh
{
	size_t key;
	VertexMemory<Vertex> vertices;
	IndexMemory<uint32_t> indices;
	size_t indices_length;

	Mesh() {}

	inline Mesh(const size_t p_key, const com::Vector<Vertex>& p_vertcies, const com::Vector<uint32_t>& p_indices, RenderAPI& p_render)
	{
		this->key = p_key;
		this->vertices.allocate(p_vertcies.Size, p_render.device, p_render.stagedbuffer_commands);

		this->indices.allocate(p_indices.Size, p_render.device, p_render.stagedbuffer_commands);
		{
			this->vertices.push(p_vertcies.Memory, p_render.device, p_render.stagedbuffer_commands);
			this->indices.push(p_indices.Memory, p_render.device, p_render.stagedbuffer_commands);
		}

		this->indices_length = p_indices.Size;
	}

	inline void dispose(RenderAPI& p_render, Device& p_device)
	{
		this->vertices.dispose(p_device, p_render.stagedbuffer_commands);
		this->indices.dispose(p_device, p_render.stagedbuffer_commands);
	}
};

struct Texture
{
	size_t key;

	size_t channel_size;
	size_t channel_nb;

	GPUOnlyImageMemory image_buffer;
	vk::ImageSubresourceRange image_subresource_range;

	vk::ImageSubresource image_subresource;
	vk::ImageView image_view;
	Vector<2, int> image_size;

	com::Vector<DeferredCommandbufferExecutionToken> layout_transitions;

	inline void allocate(const size_t p_key,
		const size_t p_chanel_size, const size_t p_channel_nb,
		const int p_width, const int p_height, const char* p_pixels, DeferredCommandBufferExecution& p_commandbuffer_execution, Device& p_device)
	{
		this->key = p_key;
		this->channel_size = p_chanel_size;
		this->channel_nb = p_channel_nb;
		this->image_size = Vector<2, int>(p_width, p_height);
		this->image_subresource_range.setAspectMask(vk::ImageAspectFlags(vk::ImageAspectFlagBits::eColor));
		this->image_subresource_range.setBaseMipLevel(0);
		this->image_subresource_range.setLevelCount(1);
		this->image_subresource_range.setBaseArrayLayer(0);
		this->image_subresource_range.setLayerCount(1);

		this->image_subresource.setAspectMask(this->image_subresource_range.aspectMask);
		this->image_subresource.setArrayLayer(this->image_subresource_range.baseArrayLayer);
		this->image_subresource.setMipLevel(this->image_subresource_range.baseMipLevel);

		vk::ImageCreateInfo l_image_create;
		l_image_create.setImageType(vk::ImageType::e2D);
		l_image_create.setExtent(vk::Extent3D(this->image_size.x, this->image_size.y, 1));
		l_image_create.setMipLevels(this->image_subresource_range.levelCount);
		l_image_create.setArrayLayers(this->image_subresource_range.layerCount);

		//TODO -> this must be calculated from PixelChanelType and ChannelNb
		l_image_create.setFormat(vk::Format::eR8G8B8A8Srgb);

		l_image_create.setTiling(vk::ImageTiling::eOptimal);
		l_image_create.setInitialLayout(vk::ImageLayout::eUndefined);

		l_image_create.setUsage(vk::ImageUsageFlags(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled));
		l_image_create.setSharingMode(vk::SharingMode::eExclusive);

		l_image_create.setSamples(vk::SampleCountFlagBits::e1);

		this->image_buffer.allocate(p_commandbuffer_execution, l_image_create, p_device);

		this->layout_transitions.push_back(this->layout_transition<vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal>(p_commandbuffer_execution, p_device));
		this->push_pixels(p_pixels, p_commandbuffer_execution, p_device);
		this->layout_transitions.push_back(this->layout_transition<vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal>(p_commandbuffer_execution, p_device));

		vk::ImageViewCreateInfo l_imageview_create_info;
		l_imageview_create_info.setImage(this->image_buffer.buffer);
		l_imageview_create_info.setViewType(vk::ImageViewType::e2D);
		l_imageview_create_info.setFormat(l_image_create.format);
		l_imageview_create_info.setSubresourceRange(this->image_subresource_range);
		this->image_view = p_device.device.createImageView(l_imageview_create_info);
	};

	inline void push_pixels(const char* p_image_buffer, DeferredCommandBufferExecution& p_commandbuffer_execution, Device& p_device)
	{
		this->image_buffer.push(p_image_buffer, this->image_subresource, this->image_subresource_range,
			this->image_size, this->channel_nb * this->channel_size, p_device, p_commandbuffer_execution);
	};

	inline void free(DeferredCommandBufferExecution& p_commandbuffer_execution, Device& p_device)
	{
		this->image_buffer.dispose(p_device, p_commandbuffer_execution);

		for (size_t i = 0; i < this->layout_transitions.Size; i++)
		{
			if (this->layout_transitions[i].isCompleted(p_commandbuffer_execution))
			{
				this->layout_transitions[i].invalidate(p_commandbuffer_execution);
			}
		}
		this->layout_transitions.free();

		p_device.device.destroyImageView(this->image_view);
	};

private:
	template<vk::ImageLayout SourceLayout, vk::ImageLayout TargetLayout>
	inline DeferredCommandbufferExecutionToken layout_transition(DeferredCommandBufferExecution& p_commandbuffer_execution, Device& p_device)
	{
		DeferredCommandbufferExecutionToken l_token;
		l_token.CompletionToken = p_commandbuffer_execution.commands_completion.alloc_element(false);

		l_token.QueueIndex =
			p_commandbuffer_execution.allocate_texturelayouttransitioncommand(this->image_buffer.buffer, this->image_subresource_range, SourceLayout, TargetLayout,
				TransitionBarrierConfigurationBuilder<SourceLayout, TargetLayout>::build(), l_token.CompletionToken);
		return l_token;
	};
};


template<class ElementType>
struct GlobalUniformBuffer
{
	uint32_t descriptorset_index;
	uint32_t binding_index;

	vk::DescriptorSetLayout descriptorset_layout;
	vk::PipelineLayout pipeline_layout;
	vk::DescriptorSet descriptor_set;
	TUniformMemory_HostWrite<ElementType> memory;

	inline void create(vk::ShaderStageFlags p_shader_stage, const uint32_t p_descriptorset_index, const uint32_t p_binding_idnex, Device& p_device, const vk::DescriptorPool p_descriptorpool)
	{
		this->descriptorset_index = p_descriptorset_index;
		this->binding_index = p_binding_idnex;

		vk::DescriptorSetLayoutBinding l_layout_bindings[1];
		{
			vk::DescriptorSetLayoutBinding& l_camera_matrices_layout_binding = l_layout_bindings[0];
			l_camera_matrices_layout_binding.setBinding(0);
			l_camera_matrices_layout_binding.setDescriptorCount(1);
			l_camera_matrices_layout_binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
			l_camera_matrices_layout_binding.setStageFlags(p_shader_stage);
			l_camera_matrices_layout_binding.setPImmutableSamplers(nullptr);
		}

		vk::DescriptorSetLayoutCreateInfo l_descriptorset_layot_create;
		l_descriptorset_layot_create.setBindingCount(1);
		l_descriptorset_layot_create.setPBindings(l_layout_bindings);

		this->descriptorset_layout = p_device.device.createDescriptorSetLayout(l_descriptorset_layot_create);

		vk::PipelineLayoutCreateInfo l_pipelinelayout_create_info;
		l_pipelinelayout_create_info.setSetLayoutCount(1);
		l_pipelinelayout_create_info.setPSetLayouts(&this->descriptorset_layout);
		this->pipeline_layout = p_device.device.createPipelineLayout(l_pipelinelayout_create_info);

		vk::DescriptorSetAllocateInfo l_allocate_info;
		l_allocate_info.setDescriptorPool(p_descriptorpool);
		l_allocate_info.setDescriptorSetCount(1);
		l_allocate_info.setPSetLayouts(&this->descriptorset_layout);
		this->descriptor_set = p_device.device.allocateDescriptorSets(l_allocate_info)[0];

		this->memory.allocate(1, p_device);
	}

	inline void dispose(Device& p_device, const vk::DescriptorPool p_descriptorpool)
	{
		p_device.device.destroyDescriptorSetLayout(this->descriptorset_layout);
		this->descriptorset_layout = nullptr;
		p_device.device.destroyPipelineLayout(this->pipeline_layout);
		this->pipeline_layout = nullptr;

		p_device.device.freeDescriptorSets(p_descriptorpool, 1, &this->descriptor_set);
		this->descriptor_set = nullptr;
		this->memory.dispose(p_device);
	}

	inline void pushbuffer(const ElementType* p_source, const Device& p_device)
	{
		this->memory.push(p_source);
	}

	inline void bind(const Device& p_device)
	{
		vk::DescriptorBufferInfo l_descriptor_buffer_info;
		l_descriptor_buffer_info.setBuffer(this->memory.buffer);
		l_descriptor_buffer_info.setOffset(0);
		l_descriptor_buffer_info.setRange(this->memory.capacity * sizeof(ElementType));

		vk::WriteDescriptorSet l_write_descriptor_set;
		l_write_descriptor_set.setDstSet(this->descriptor_set);
		l_write_descriptor_set.setDescriptorCount(1);
		l_write_descriptor_set.setDescriptorType(vk::DescriptorType::eUniformBuffer);
		l_write_descriptor_set.setDstBinding(this->binding_index);
		l_write_descriptor_set.setPBufferInfo(&l_descriptor_buffer_info);

		p_device.device.updateDescriptorSets(1, &l_write_descriptor_set, 0, nullptr);
	}

	inline void bind_command(CommandBuffer& p_commandbuffer)
	{
		p_commandbuffer.command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->pipeline_layout, this->descriptorset_index, 1, &this->descriptor_set, 0, nullptr);
	}
};

struct ShaderUniformBufferParameter
{
	vk::DescriptorSet descriptor_set;
	UniformMemory_HostWrite memory;

	inline void create(RenderAPI& p_renderapi, size_t p_element_size, vk::DescriptorSetLayout& p_descriptorset_layout)
	{
		vk::DescriptorSetAllocateInfo l_allocate_info;
		l_allocate_info.setDescriptorPool(p_renderapi.descriptor_pool);
		l_allocate_info.setDescriptorSetCount(1);
		l_allocate_info.setPSetLayouts(&p_descriptorset_layout);
		this->descriptor_set = p_renderapi.device.device.allocateDescriptorSets(l_allocate_info)[0];

		this->memory.allocate(1, p_element_size, p_renderapi.device);
	}

	inline void dispose(Device& p_device, const vk::DescriptorPool p_descriptorpool)
	{
		p_device.device.freeDescriptorSets(p_descriptorpool, 1, &this->descriptor_set);
		this->descriptor_set = nullptr;
		this->memory.dispose(p_device);
	}
	
	inline char* get_buffer_ptr()
	{
		return this->memory.mapped_memory.mapped_data;
	};

	inline void pushbuffer(const GPtr& p_source, const Device& p_device)
	{
		this->memory.push(p_source);
	}

	inline void bind(const uint32_t p_dst_binding, const size_t p_element_size, const Device& p_device)
	{
		vk::DescriptorBufferInfo l_descriptor_buffer_info;
		l_descriptor_buffer_info.setBuffer(this->memory.buffer);
		l_descriptor_buffer_info.setOffset(0);
		l_descriptor_buffer_info.setRange(this->memory.capacity * p_element_size);

		vk::WriteDescriptorSet l_write_descriptor_set;
		l_write_descriptor_set.setDstSet(this->descriptor_set);
		l_write_descriptor_set.setDescriptorCount(1);
		l_write_descriptor_set.setDescriptorType(vk::DescriptorType::eUniformBuffer);
		l_write_descriptor_set.setDstBinding(p_dst_binding);
		l_write_descriptor_set.setPBufferInfo(&l_descriptor_buffer_info);

		p_device.device.updateDescriptorSets(1, &l_write_descriptor_set, 0, nullptr);
	}

	inline void bind_command(CommandBuffer& p_commandbuffer, uint32_t p_set_index, vk::PipelineLayout& p_pipeline_layout)
	{
		p_commandbuffer.command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_pipeline_layout,
			p_set_index, 1, &this->descriptor_set, 0, nullptr);
	}
};

template<class ElementType>
struct TShaderUniformBufferParameter : public ShaderUniformBufferParameter
{
	inline void create(RenderAPI& p_renderapi, vk::DescriptorSetLayout& p_descriptorset_layout)
	{
		ShaderUniformBufferParameter::create(p_renderapi, sizeof(ElementType), p_descriptorset_layout);
	};

	inline ElementType* get_buffer_ptr()
	{
		return ShaderUniformBufferParameter::get_buffer_ptr();
	};

	inline void pushbuffer(const ElementType* p_source, const Device& p_device)
	{
		ShaderUniformBufferParameter::pushbuffer(GPtr::fromType(p_source), p_device);
	};

	inline void bind(const uint32_t p_dst_binding, const Device& p_device)
	{
		ShaderUniformBufferParameter::bind(p_dst_binding, sizeof(ElementType), p_device);
	}
};

struct ShaderCombinedImageSamplerParameter
{
	vk::DescriptorSet descriptor_set;
	com::PoolToken texture;

	inline void create(const com::PoolToken& p_texture, RenderAPI& p_renderapi)
	{
		vk::DescriptorSetAllocateInfo l_allocate_info;
		l_allocate_info.setDescriptorPool(p_renderapi.descriptor_pool);
		l_allocate_info.setDescriptorSetCount(1);
		l_allocate_info.setPSetLayouts(&p_renderapi.shaderparameter_layouts.texture_fragment_layout);
		this->descriptor_set = p_renderapi.device.device.allocateDescriptorSets(l_allocate_info)[0];
		this->texture = p_texture;
	}

	inline void dispose(RenderAPI& p_renderapi)
	{
		p_renderapi.device.device.freeDescriptorSets(p_renderapi.descriptor_pool, 1, &this->descriptor_set);
		this->descriptor_set = nullptr;
		this->texture = com::PoolToken();
	}

	inline void bind(const uint32_t p_dst_binding, const Device& p_device, const TextureSamplers& p_texutre_samplers, com::Pool<Texture>& p_texutre_heap)
	{
		Texture& l_texture = p_texutre_heap[this->texture];

		vk::DescriptorImageInfo l_descriptor_image_info;
		l_descriptor_image_info.setImageView(l_texture.image_view);
		l_descriptor_image_info.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		l_descriptor_image_info.setSampler(p_texutre_samplers.Default);

		vk::WriteDescriptorSet l_write_descriptor_set;
		l_write_descriptor_set.setDstSet(this->descriptor_set);
		l_write_descriptor_set.setDescriptorCount(1);
		l_write_descriptor_set.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
		l_write_descriptor_set.setDstBinding(p_dst_binding);
		l_write_descriptor_set.setPImageInfo(&l_descriptor_image_info);

		p_device.device.updateDescriptorSets(1, &l_write_descriptor_set, 0, nullptr);
	}

	inline void bind_command(CommandBuffer& p_commandbuffer, uint32_t p_set_index, vk::PipelineLayout& p_pipeline_layout)
	{
		p_commandbuffer.command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_pipeline_layout,
			p_set_index, 1, &this->descriptor_set, 0, nullptr);
	}
};

struct ShaderParameter
{
	enum Type
	{
		UNKNOWN = 0, UNIFORM = 1, TEXTURE = 2
	} type = Type::UNKNOWN;

	com::PoolToken parameter_token;

	inline ShaderParameter() {};
	inline ShaderParameter(Type p_type, const com::PoolToken p_token)
	{
		this->type = p_type;
		this->parameter_token = p_token;
	}
};

struct Material
{
	com::Vector<ShaderParameter> parameters;
	
	/*
	com::PoolToken diffuse_color;
	com::PoolToken diffuse_texture;
	*/

	Material() {};


	inline void add_image_parameter(const com::PoolToken& p_parameter)
	{
		this->parameters.push_back(ShaderParameter(ShaderParameter::Type::TEXTURE, p_parameter));
	};

	inline void add_uniform_parameter(const com::PoolToken& p_parameter)
	{
		this->parameters.push_back(ShaderParameter(ShaderParameter::Type::UNIFORM, p_parameter));
	};

	inline void free(RenderAPI& p_renderapi, com::Pool<ShaderUniformBufferParameter>& p_shader_uniformparameter_heap, com::Pool<ShaderCombinedImageSamplerParameter>& p_shader_texturesample_heap)
	{
		
		for (size_t i = 0; i < this->parameters.Size; i++)
		{
			ShaderParameter& l_parameter = this->parameters[i];
			switch (l_parameter.type)
			{
			case ShaderParameter::Type::UNIFORM:
			{
				p_shader_uniformparameter_heap[l_parameter.parameter_token].dispose(p_renderapi.device, p_renderapi.descriptor_pool);
				p_shader_uniformparameter_heap.release_element(l_parameter.parameter_token);
			}
			break;
			case ShaderParameter::Type::TEXTURE:
			{
				p_shader_texturesample_heap[l_parameter.parameter_token].dispose(p_renderapi);
				p_shader_texturesample_heap.release_element(l_parameter.parameter_token);
			}
			break;
			}
		}

		this->parameters.free();
	};

	inline void bind_command(CommandBuffer& p_command_buffer, size_t p_set_index, com::Pool<ShaderUniformBufferParameter>& p_shader_uniformparameter_heap,
		com::Pool<ShaderCombinedImageSamplerParameter>& p_shader_texturesample_heap, vk::PipelineLayout& p_pipeline_layout)
	{
		for (size_t i = 0; i < this->parameters.Size; i++)
		{
			ShaderParameter& l_parameter = this->parameters[i];
			switch (l_parameter.type)
			{
			case ShaderParameter::Type::UNIFORM:
			{
				p_shader_uniformparameter_heap.resolve(l_parameter.parameter_token).bind_command(p_command_buffer, p_set_index + i, p_pipeline_layout);
			}
			break;
			case ShaderParameter::Type::TEXTURE:
			{
				p_shader_texturesample_heap.resolve(l_parameter.parameter_token).bind_command(p_command_buffer, p_set_index + i, p_pipeline_layout);
			}
			break;
			}
		}
		
	};
};


struct RenderableObject
{
	com::PoolToken mesh;
	TShaderUniformBufferParameter<mat4f> model_matrix_buffer;

	RenderableObject() {}

	inline void allocate(const com::PoolToken& p_mesh, RenderAPI& p_render_api)
	{
		this->mesh = p_mesh;
		this->model_matrix_buffer.create(p_render_api, p_render_api.shaderparameter_layouts.uniformbuffer_vertex_layout);
		this->model_matrix_buffer.bind(0, p_render_api.device);
	}

	inline void free(Device& p_device, vk::DescriptorPool p_descriptor_pool)
	{
		this->model_matrix_buffer.dispose(p_device, p_descriptor_pool);
	}

	inline void pushModelMatrix(const mat4f p_model, const Device& p_device)
	{
		this->model_matrix_buffer.pushbuffer(&p_model, p_device);
	}

	inline void dispose(Device& p_device, vk::DescriptorPool p_descriptor_pool)
	{
		this->mesh = com::PoolToken();
		this->model_matrix_buffer.dispose(p_device, p_descriptor_pool);
	}

	inline void draw(CommandBuffer& p_commandbuffer, com::Pool<Mesh>& p_mesh_heap)
	{
		vk::DeviceSize l_offsets[1] = { 0 };
		p_commandbuffer.command_buffer.bindVertexBuffers(0, 1, &p_mesh_heap[this->mesh].vertices.buffer, l_offsets);
		p_commandbuffer.command_buffer.bindIndexBuffer(p_mesh_heap[this->mesh].indices.buffer, 0, vk::IndexType::eUint32);
		p_commandbuffer.command_buffer.drawIndexed((uint32_t)p_mesh_heap[this->mesh].indices_length, (uint32_t)1, (uint32_t)0, (uint32_t)0, (uint32_t)1);
	}
};

struct RenderHeap2
{
	RenderAPI* render_api;

	com::Pool<ShaderModule> shadermodules;

	com::OptionalPool<Shader> shaders;
	com::Pool<com::Vector<com::PoolToken>> shaders_to_materials;

	com::OptionalPool<Material> materials;
	com::Pool<com::Vector<com::PoolToken>> material_to_renderableobjects;

	com::OptionalPool<RenderableObject> renderableobjects;

	com::Pool<Mesh> meshes;
	com::Pool<Texture> textures;

	com::Pool<ShaderUniformBufferParameter> shader_uniform_parameters;
	com::Pool<ShaderCombinedImageSamplerParameter> shader_imagesample_parameters;

	struct Resource
	{
		struct ShaderModuleResourceAllocator
		{
			RenderHeap2* render_heap;
			AssetServerHandle asset_server;

			inline ShaderModuleResourceAllocator() {};

			inline ShaderModuleResourceAllocator(const AssetServerHandle& p_asset_server, RenderHeap2& p_render_heap)
			{
				this->asset_server = p_asset_server;
				this->render_heap = &p_render_heap;
			};

			inline com::PoolToken allocate(size_t p_key)
			{
				ShaderModule l_shader_module;
				l_shader_module.key = p_key;
				l_shader_module.shader_module = load_shadermodule(this->asset_server, this->render_heap->render_api->device, p_key);
				return this->render_heap->shadermodules.alloc_element(l_shader_module);
			};

			inline void free(com::PoolToken& p_shader_module)
			{
				dispose_shaderModule(this->render_heap->render_api->device, this->render_heap->shadermodules[p_shader_module].shader_module);
				this->render_heap->shadermodules.release_element(p_shader_module);
			};

		private:
			inline static vk::ShaderModule load_shadermodule(const AssetServerHandle& p_asset_server_handle, const Device& p_device, const size_t p_shadermodule_id)
			{
				com::Vector<char> l_shader_code = p_asset_server_handle.get_resource(p_shadermodule_id);
				{
					if (l_shader_code.Size > 0)
					{
						vk::ShaderModuleCreateInfo l_shader_module_create_info;
						l_shader_module_create_info.setCodeSize(l_shader_code.Size);
						l_shader_module_create_info.setPCode((uint32_t*)l_shader_code.Memory);

						vk::ShaderModule l_module = p_device.device.createShaderModule(l_shader_module_create_info);
						l_shader_code.free();
						return l_module;
					}
				}
				l_shader_code.free();
				return nullptr;
			}

			inline static void dispose_shaderModule(const Device& p_device, const vk::ShaderModule& p_shader_module)
			{
				p_device.device.destroyShaderModule(p_shader_module);
			}

		};

		struct ShaderResourceAllocator
		{
			RenderHeap2* render_heap;
			AssetServerHandle asset_server;

			inline ShaderResourceAllocator() {};

			inline ShaderResourceAllocator(const AssetServerHandle& p_asset_server, RenderHeap2& p_render_heap)
			{
				this->asset_server = p_asset_server;
				this->render_heap = &p_render_heap;
			};

			inline com::PoolToken allocate(const size_t& p_key)
			{
				com::Vector<char> l_shader_resource_binary = this->asset_server.get_resource(p_key);
				ShaderAsset l_shader_asset = ShaderAsset::deserialize(l_shader_resource_binary.Memory);
				l_shader_resource_binary.free();

				auto l_vertex_module = this->render_heap->allocate_shadermodule_internal(l_shader_asset.vertex);
				auto l_fragment_module = this->render_heap->allocate_shadermodule_internal(l_shader_asset.fragment);
				Shader l_shader = Shader(p_key,
					this->render_heap->shadermodules[l_vertex_module],
					this->render_heap->shadermodules[l_fragment_module],
					l_shader_asset.compare_op,
					this->render_heap->render_api->swap_chain.renderpass,
					*(this->render_heap->render_api)
				);

				this->render_heap->shaders_to_materials.alloc_element(com::Vector<com::PoolToken>());
				return this->render_heap->shaders.alloc_element(l_shader);
			};

			inline void free(const com::PoolToken& p_shader)
			{
				Optional<Shader>& l_shader = this->render_heap->shaders[p_shader];
				l_shader.value.dispose(this->render_heap->render_api->device);
				this->render_heap->shaders_to_materials[p_shader].free();
				this->render_heap->shaders_to_materials.release_element(p_shader);
				this->render_heap->shaders.release_element(p_shader);
			};
		};

		struct MeshResourceAllocator
		{
			RenderHeap2* render_heap;
			AssetServerHandle asset_server;

			inline MeshResourceAllocator() {};

			inline MeshResourceAllocator(const AssetServerHandle& p_asset_server, RenderHeap2& p_render_heap)
			{
				this->asset_server = p_asset_server;
				this->render_heap = &p_render_heap;
			};

			struct MeshAsset
			{
				com::Vector<Vertex> vertices;
				com::Vector<uint32_t> indices;

				inline void free()
				{
					this->vertices.free();
					this->indices.free();
				}

				inline void clear()
				{
					this->vertices.clear();
					this->indices.clear();
				}

				inline static MeshAsset cast_from(const char* p_data)
				{
					MeshAsset l_resource;

					size_t l_current_pointer = 0;
					l_resource.vertices = Serialization::Binary::deserialize_vector<Vertex>(l_current_pointer, p_data);
					l_resource.indices = Serialization::Binary::deserialize_vector<uint32_t>(l_current_pointer, p_data);
					return l_resource;
				};

				inline void sertialize_to(com::Vector<char>& out_target)
				{
					Serialization::Binary::serialize_vector(this->vertices, out_target);
					Serialization::Binary::serialize_vector(this->indices, out_target);
				};


			};

			inline com::PoolToken allocate(const size_t& p_key)
			{
				com::PoolToken l_mesh;
				com::Vector<char> l_mesh_asset_binary = this->asset_server.get_resource(p_key);
				{
					MeshAsset l_mesh_asset = MeshAsset::cast_from(l_mesh_asset_binary.Memory);
					l_mesh = this->render_heap->meshes.alloc_element(Mesh(p_key, l_mesh_asset.vertices, l_mesh_asset.indices, *this->render_heap->render_api));
				}
				l_mesh_asset_binary.free();
				return l_mesh;
			};

			inline void free(const com::PoolToken& p_mesh)
			{
				this->render_heap->meshes[p_mesh].dispose(*this->render_heap->render_api, this->render_heap->render_api->device);
				this->render_heap->meshes.release_element(p_mesh);
			};
		};

		struct TextureResourceAllocator
		{
			RenderHeap2* render_heap;
			AssetServerHandle asset_server;

			inline TextureResourceAllocator() {};

			inline TextureResourceAllocator(const AssetServerHandle& p_asset_server, RenderHeap2& p_render_heap)
			{
				this->asset_server = p_asset_server;
				this->render_heap = &p_render_heap;
			};

			struct TextureAsset
			{
				Vector<2, int> size;
				int channel_number;
				com::Vector<char> pixels;

				inline void free()
				{
					this->pixels.free();
				}

				inline void clear()
				{
					this->pixels.clear();
				}

				inline static TextureAsset cast_from(const char* p_data)
				{
					TextureAsset l_resource;

					size_t l_current_pointer = 0;
					l_resource.size = *Serialization::Binary::deserialize_field<Vector<2, int>>(l_current_pointer, p_data);
					l_resource.channel_number = *Serialization::Binary::deserialize_field<int>(l_current_pointer, p_data);
					l_resource.pixels = Serialization::Binary::deserialize_vector<char>(l_current_pointer, p_data);
					return l_resource;
				};

				inline void sertialize_to(com::Vector<char>& out_target)
				{
					Serialization::Binary::serialize_field<Vector<2, int>>(&this->size, out_target);
					Serialization::Binary::serialize_field<int>(&this->channel_number, out_target);
					Serialization::Binary::serialize_vector<char>(this->pixels, out_target);
				};
			};

			inline com::PoolToken allocate(const size_t& p_key)
			{
				com::PoolToken l_texture;
				com::Vector<char> l_texture_asset_binary = this->asset_server.get_resource(p_key);
				{
					TextureAsset l_texture_asset = TextureAsset::cast_from(l_texture_asset_binary.Memory);
					Texture l_texture_resource;
					l_texture_resource.allocate(p_key, sizeof(char), l_texture_asset.channel_number, l_texture_asset.size.x, l_texture_asset.size.y, l_texture_asset.pixels.Memory,
						this->render_heap->render_api->stagedbuffer_commands, this->render_heap->render_api->device);
					l_texture = this->render_heap->textures.alloc_element(l_texture_resource);
				}
				l_texture_asset_binary.free();
				return l_texture;
			};

			inline void free(const com::PoolToken& p_texture)
			{
				this->render_heap->textures[p_texture].free(this->render_heap->render_api->stagedbuffer_commands, this->render_heap->render_api->device);
				this->render_heap->textures.release_element(p_texture);
			};

		};

		ResourceMap<size_t, com::PoolToken, ShaderModuleResourceAllocator> shader_module_resources;
		ResourceMap<size_t, com::PoolToken, ShaderResourceAllocator> shader_resources;
		ResourceMap<size_t, com::PoolToken, MeshResourceAllocator> mesh_resources;
		ResourceMap<size_t, com::PoolToken, TextureResourceAllocator> texture_resources;

		inline void allocate(RenderHeap2& p_render_heap, AssetServerHandle p_asset_server)
		{
			this->shader_module_resources.allocate(1, ShaderModuleResourceAllocator(p_asset_server, p_render_heap));
			this->shader_resources.allocate(1, ShaderResourceAllocator(p_asset_server, p_render_heap));
			this->mesh_resources.allocate(1, MeshResourceAllocator(p_asset_server, p_render_heap));
			this->texture_resources.allocate(1, TextureResourceAllocator(p_asset_server, p_render_heap));
		};

		inline void free()
		{
			this->shader_module_resources.free();
			this->shader_resources.free();
			this->mesh_resources.free();
			this->texture_resources.free();
		};

	} resource;

public:

	inline void allocate(AssetServerHandle p_asset_server, RenderAPI& p_render_api)
	{
		this->render_api = &p_render_api;
		this->resource.allocate(*this, p_asset_server);
	};

	inline void free()
	{
		this->resource.free();
	};

	inline com::PoolToken allocate_shadermodule(const std::string& p_path)
	{
		return this->allocate_shadermodule_internal(Hash<std::string>::hash(p_path));
	};

	inline void free_shadermodule(const com::PoolToken& p_shader_module)
	{
		this->resource.shader_module_resources.free_resource(this->shadermodules[p_shader_module].key);
	};


	inline com::PoolToken allocate_shader(const std::string& p_key)
	{
		return this->allocate_shader(Hash<std::string>::hash(p_key));
	};

	inline com::PoolToken allocate_shader(const size_t p_key)
	{
		return this->resource.shader_resources.allocate_resource(p_key);
	};

	inline void free_shader(const com::PoolToken& p_shader)
	{
		this->resource.shader_resources.free_resource(this->shaders[p_shader].value.key);
	};

	inline com::PoolToken allocate_material(const com::PoolToken& p_shader)
	{
		com::PoolToken l_material_handle = this->materials.alloc_element(Material());
		this->material_to_renderableobjects.alloc_element(com::Vector<com::PoolToken>());
		this->shaders_to_materials[p_shader.Index].push_back(l_material_handle);
		return l_material_handle;
	};

	inline void material_add_image_parameter(const com::PoolToken& p_material, const com::PoolToken& p_texture)
	{
		ShaderCombinedImageSamplerParameter l_diffuse_texture;
		l_diffuse_texture.create(p_texture, *this->render_api);
		l_diffuse_texture.bind(0, this->render_api->device, this->render_api->image_samplers, this->textures);
		this->materials[p_material].value.add_image_parameter(this->shader_imagesample_parameters.alloc_element(l_diffuse_texture));
	};

	inline void material_add_uniform_parameter(const com::PoolToken& p_material, const GPtr& p_initial_value)
	{
		ShaderUniformBufferParameter l_diffuse_color;
		l_diffuse_color.create(*this->render_api, p_initial_value.element_size, this->render_api->shaderparameter_layouts.uniformbuffer_layout);
		l_diffuse_color.bind(0, p_initial_value.element_size, this->render_api->device);
		l_diffuse_color.pushbuffer(p_initial_value, this->render_api->device);
		this->materials[p_material].value.add_uniform_parameter(this->shader_uniform_parameters.alloc_element(l_diffuse_color));
	};

	inline void free_material(const com::PoolToken& p_material)
	{
		this->materials[p_material].value.free(*this->render_api, this->shader_uniform_parameters, this->shader_imagesample_parameters);
		this->materials.release_element(p_material);
		this->material_to_renderableobjects[p_material].free();
		this->material_to_renderableobjects.release_element(p_material);
	};

	inline com::PoolToken allocate_mesh(const std::string& p_path)
	{
		return this->allocate_mesh(Hash<std::string>::hash(p_path));
	};

	inline com::PoolToken allocate_mesh(const size_t& p_id)
	{
		return this->resource.mesh_resources.allocate_resource(p_id);
	};


	inline void free_mesh(const com::PoolToken& p_mesh)
	{
		this->resource.mesh_resources.free_resource(this->meshes[p_mesh].key);
	};

	inline com::PoolToken allocate_texture(const std::string& p_path)
	{
		return this->allocate_texture(Hash<std::string>::hash(p_path));
	};

	inline com::PoolToken allocate_texture(const size_t p_id)
	{
		return this->resource.texture_resources.allocate_resource(p_id);
	};

	inline void free_texture(const com::PoolToken& p_texture)
	{
		this->resource.texture_resources.free_resource(this->textures[p_texture].key);
	};

	inline com::PoolToken allocate_rendereableObject(const com::PoolToken& p_material, const com::PoolToken& p_mesh)
	{
		RenderableObject l_renderable_object;
		l_renderable_object.allocate(p_mesh, *this->render_api);
		com::PoolToken l_renderableobjet_handle = this->renderableobjects.alloc_element(l_renderable_object);
		this->material_to_renderableobjects[p_material.Index].push_back(l_renderableobjet_handle);
		return l_renderableobjet_handle;
	};

	inline void free_renderableObject(const com::PoolToken& p_renderableObject)
	{
		Optional<RenderableObject>& l_renderableobject = this->renderableobjects[p_renderableObject];
		l_renderableobject.value.dispose(this->render_api->device, this->render_api->descriptor_pool);
		this->renderableobjects.release_element(p_renderableObject);
	};

private:

	inline com::PoolToken allocate_shadermodule_internal(size_t p_key)
	{
		return this->resource.shader_module_resources.allocate_resource(p_key);
	};

};

struct Render
{
	RenderWindow window;
	RenderAPI renderApi;

	GlobalUniformBuffer<CameraMatrices> camera_matrices_globalbuffer;

	RenderHeap2 heap;

	inline Render(const AssetServerHandle p_asset_server)
	{
		this->window = RenderWindow(800, 600, "MyGame");
		this->renderApi.init(window);
		this->create_global_buffers();
		this->heap.allocate(p_asset_server, this->renderApi);
	};

	inline void dispose()
	{
		this->heap.free();
		this->destroy_global_buffers();
		this->renderApi.dispose();
		this->window.dispose();
	};


	inline void draw()
	{
		OPTICK_EVENT();

		uint32_t l_render_image_index = this->renderApi.swap_chain.getNextImage(this->renderApi.synchronization.present_complete_semaphore);

		this->renderApi.device.device.resetFences(1, &this->renderApi.synchronization.draw_command_fences[l_render_image_index]);

		CommandBuffer& l_command_buffer = this->renderApi.draw_commandbuffers[l_render_image_index];
		l_command_buffer.begin();

		this->renderApi.stagedbuffer_commands.process_all_buffers(l_command_buffer, this->renderApi.device);

		vk::ClearValue l_clear[2];
		l_clear[0].color.setFloat32({ 0.0f, 0.0f, 0.2f, 1.0f });
		l_clear[1].depthStencil.setDepth(1.0f);
		l_clear[1].depthStencil.setStencil((uint32_t)0.0f);

		vk::RenderPassBeginInfo l_renderpass_begin;
		l_renderpass_begin.setPNext(nullptr);
		l_renderpass_begin.setRenderPass(this->renderApi.swap_chain.renderpass.l_render_pass);
		vk::Rect2D l_renderArea;
		l_renderArea.setOffset(vk::Offset2D(0, 0));
		l_renderArea.setExtent(this->renderApi.swap_chain.extend);
		l_renderpass_begin.setRenderArea(l_renderArea);
		l_renderpass_begin.setClearValueCount(2);
		l_renderpass_begin.setPClearValues(l_clear);
		l_renderpass_begin.setFramebuffer(this->renderApi.swap_chain.framebuffers[l_render_image_index]);

		vk::Viewport l_viewport;
		l_viewport.setHeight(this->window.Height);
		l_viewport.setWidth(this->window.Width);
		//l_viewport.setX(this->window.Width);
		//l_viewport.setY(this->window.Height);
		l_viewport.setMinDepth(0.0f);
		l_viewport.setMaxDepth(1.0f);

		vk::Rect2D l_windowarea;
		l_windowarea.setOffset(vk::Offset2D(0, 0));
		l_windowarea.setExtent(vk::Extent2D(this->window.Width, this->window.Height));


		l_command_buffer.command_buffer.beginRenderPass(l_renderpass_begin, vk::SubpassContents::eInline);
		l_command_buffer.command_buffer.setViewport(0, 1, &l_viewport);
		l_command_buffer.command_buffer.setScissor(0, 1, &l_windowarea);


		this->camera_matrices_globalbuffer.bind_command(l_command_buffer);

		for (size_t l_shader_index = 0; l_shader_index < this->heap.shaders.size(); l_shader_index++)
		{
			com::PoolToken l_shader_heap = l_shader_index;
			Optional<Shader>& l_shader = this->heap.shaders[l_shader_index];
			if (l_shader.hasValue)
			{
				l_command_buffer.command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, l_shader.value.pipeline);

				com::Vector<com::PoolToken>& l_materials = this->heap.shaders_to_materials[l_shader_heap.Index];
				for (size_t l_material_index = 0; l_material_index < l_materials.Size; l_material_index++)
				{
					com::PoolToken l_material_heap_token = l_materials[l_material_index];
					Optional<Material>& l_material = this->heap.materials[l_material_heap_token];
					if (l_material.hasValue)
					{
						l_material.value.bind_command(l_command_buffer, 2, this->heap.shader_uniform_parameters, this->heap.shader_imagesample_parameters, l_shader.value.pipeline_layout);

						com::Vector<com::PoolToken>& l_renderableobjects = this->heap.material_to_renderableobjects[l_material_heap_token.Index];
						for (size_t l_renderableobject_index = 0; l_renderableobject_index < l_renderableobjects.Size; l_renderableobject_index++)
						{
							Optional<RenderableObject>& l_renderableobject = this->heap.renderableobjects[l_renderableobjects[l_renderableobject_index]];
							if (l_renderableobject.hasValue)
							{
								l_renderableobject.value.model_matrix_buffer.bind_command(l_command_buffer, 1, l_shader.value.pipeline_layout);
								l_renderableobject.value.draw(l_command_buffer, this->heap.meshes);
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

		this->renderApi.device.graphics_queue.submit(1, &l_submit, this->renderApi.synchronization.draw_command_fences[l_render_image_index]);
		this->renderApi.swap_chain.presentImage(this->renderApi.device.present_queue, l_render_image_index, this->renderApi.synchronization.render_complete_semaphore);

		this->renderApi.device.device.waitForFences(1, &this->renderApi.synchronization.draw_command_fences[l_render_image_index], true, UINT64_MAX);
	}

	inline void push_camera_buffer(const float p_fov, const float p_near, const float p_far, const vec3f& p_world_position, const vec3f& p_world_forward, const vec3f& p_world_up)
	{
		CameraMatrices l_cam_mat;
		l_cam_mat.view = view(p_world_position, p_world_forward, p_world_up);
		l_cam_mat.projection = perspective<float>(p_fov * DEG_TO_RAD, (float)this->renderApi.swap_chain.extend.width / (float)this->renderApi.swap_chain.extend.height, p_near, p_far);
		this->camera_matrices_globalbuffer.pushbuffer(&l_cam_mat, this->renderApi.device);
	};

private:

	inline void create_global_buffers()
	{
		this->camera_matrices_globalbuffer.create(vk::ShaderStageFlagBits::eVertex, 0, 0, this->renderApi.device, this->renderApi.descriptor_pool);
		this->camera_matrices_globalbuffer.bind(this->renderApi.device);
	}

	inline void destroy_global_buffers()
	{
		this->camera_matrices_globalbuffer.dispose(this->renderApi.device, this->renderApi.descriptor_pool);
	}

};

RenderHandle create_render(const AssetServerHandle p_assetserver_handle)
{
	return new Render(p_assetserver_handle);
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

void render_draw(const RenderHandle& p_render)
{
	Render* l_render = (Render*)p_render;
	l_render->draw();
};

void render_push_camera_buffer(const RenderHandle& p_render, const float p_fov, const float p_near, const float p_far, 
	const Math::vec3f& p_world_position, const Math::vec3f& p_world_forward, const Math::vec3f& p_world_up)
{
	Render* l_render = (Render*)p_render;
	l_render->push_camera_buffer(p_fov, p_near, p_far, p_world_position, p_world_forward, p_world_up);
};
