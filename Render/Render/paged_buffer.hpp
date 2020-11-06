#pragma once

#include "Common/Container/vector.hpp"
#include "Common/Container/pool.hpp"
#include <Vulkan/vulkaninclude.hpp>


struct BufferChunk
{
	size_t chunk_size;
	size_t offset;
};

struct BufferWithOffset
{
	vk::DeviceMemory original_memory;
	char* original_mapped_memory;
	uint32_t memory_type;
	size_t offset;
	com::PoolToken<BufferChunk> allocated_chunk_token;
	size_t pagedbuffer_index;
};

struct PageBuffer
{
	size_t chunk_total_size;
	size_t page_index;
	vk::DeviceMemory root_memory;
	char* mapped_memory;
	com::Pool<BufferChunk> allocated_chunks;
	com::Vector<BufferChunk> free_chunks;

	inline void allocate(size_t p_page_index, size_t p_size, uint32_t p_memorytype, bool p_mapping, vk::Device p_device)
	{
		this->page_index = p_page_index;
		this->chunk_total_size = p_size;
		vk::MemoryAllocateInfo l_memoryallocate_info;
		l_memoryallocate_info.setAllocationSize(p_size);
		l_memoryallocate_info.setMemoryTypeIndex(p_memorytype);
		this->root_memory = p_device.allocateMemory(l_memoryallocate_info);

		if (p_mapping)
		{
			this->mapped_memory = (char*)p_device.mapMemory(this->root_memory, 0, this->chunk_total_size);
		}

		BufferChunk l_whole_chunk;
		l_whole_chunk.chunk_size = this->chunk_total_size;
		l_whole_chunk.offset = 0;
		this->free_chunks.push_back(l_whole_chunk);
	}

	inline void dispose(vk::Device p_device)
	{
		p_device.freeMemory(this->root_memory);
	}

	inline bool allocate_element(size_t p_size, BufferWithOffset* out_chunk)
	{
		for (size_t i = 0; i < this->free_chunks.Size; i++)
		{
			BufferChunk& l_chunk = this->free_chunks[i];

			if (l_chunk.chunk_size > p_size)
			{
				size_t l_split_size = l_chunk.chunk_size - p_size;


				BufferChunk l_new_allocated_chunk;
				l_new_allocated_chunk.chunk_size = l_chunk.chunk_size - l_split_size;
				l_new_allocated_chunk.offset = l_chunk.offset;

				out_chunk->pagedbuffer_index = this->page_index;
				out_chunk->allocated_chunk_token = this->allocated_chunks.alloc_element(l_new_allocated_chunk);
				out_chunk->offset = l_new_allocated_chunk.offset;
				out_chunk->original_memory = this->root_memory;
				out_chunk->original_mapped_memory = this->mapped_memory;

				this->free_chunks[i].chunk_size = l_split_size;
				this->free_chunks[i].offset = l_new_allocated_chunk.offset + l_new_allocated_chunk.chunk_size;

				return true;
			}
			else if (l_chunk.chunk_size == p_size)
			{
				out_chunk->pagedbuffer_index = this->page_index;
				out_chunk->allocated_chunk_token = this->allocated_chunks.alloc_element(l_chunk);
				out_chunk->offset = l_chunk.offset;
				out_chunk->original_memory = this->root_memory;
				out_chunk->original_mapped_memory = this->mapped_memory;

				this->free_chunks.erase_at(i);

				return true;
			}
		}

		return false;
	}

	inline void release_element(const BufferWithOffset& p_buffer)
	{
		this->free_chunks.push_back(this->allocated_chunks[p_buffer.allocated_chunk_token]);
		this->allocated_chunks.release_element(p_buffer.allocated_chunk_token);
	}
};

struct PagedBuffer
{
	com::Vector<PageBuffer> page_buffers;
	uint32_t memory_type;
	size_t chunk_size;

	inline void allocate(size_t p_chunksize, uint32_t p_memory_type)
	{
		this->memory_type = p_memory_type;
		this->chunk_size = p_chunksize;
	}

	inline void destroy(vk::Device p_device)
	{
		for (size_t i = 0; i < this->page_buffers.Size; i++)
		{
			this->page_buffers[i].dispose(p_device);
		}
	}

	inline bool allocate_element(size_t p_size, vk::Device p_device, bool p_mapping, BufferWithOffset* out_chunk)
	{
		for (size_t i = 0; i < this->page_buffers.Size; i++)
		{
			if (this->page_buffers[i].allocate_element(p_size, out_chunk))
			{
				return true;
			}
		}

		this->page_buffers.push_back(PageBuffer());
		this->page_buffers[this->page_buffers.Size - 1].allocate(this->page_buffers.Size - 1, this->chunk_size, this->memory_type, p_mapping, p_device);
		return this->page_buffers[this->page_buffers.Size - 1].allocate_element(p_size, out_chunk);
	}

	inline void free_element(const BufferWithOffset& p_buffer)
	{
		this->page_buffers[p_buffer.pagedbuffer_index].release_element(p_buffer);
	}
};

struct PagedBuffers
{
	PagedBuffer i7;
	PagedBuffer i8;

	inline void allocate()
	{
		this->i7.allocate(16000000, 7);
		this->i8.allocate(16000000, 8);
	}

	inline void destroy(vk::Device p_device)
	{
		this->i7.destroy(p_device);
		this->i8.destroy(p_device);
	}

	inline bool allocate_element(size_t p_size, uint32_t p_memory_type, vk::Device p_device, BufferWithOffset* out_chunk)
	{
		switch (p_memory_type)
		{
		case 7:
		{
			bool l_success = this->i7.allocate_element(p_size, p_device, false, out_chunk);
			out_chunk->memory_type = 7;
			return l_success;
		}
		break;
		case 8:
		{
			bool l_success = this->i8.allocate_element(p_size, p_device, true, out_chunk);
			out_chunk->memory_type = 8;
			return l_success;
		}
		break;
		}

		return false;
	}

	inline void free_element(BufferWithOffset p_chunk)
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
};