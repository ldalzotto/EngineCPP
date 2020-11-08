#pragma once

#include <type_traits>
#include "allocators.hpp"
#include "Common/Container/pool.hpp"

struct GeneralPurposeHeapMemoryChunk
{
	size_t chunk_size;
	size_t offset;
};

template<class ReturnType>
struct IMemoryChunkMapper
{
	ReturnType map(const GeneralPurposeHeapMemoryChunk& p_chunk, com::PoolToken<GeneralPurposeHeapMemoryChunk>& p_chunktoken);
};

struct DefaultMemoryChunkMapper : public IMemoryChunkMapper<com::PoolToken<GeneralPurposeHeapMemoryChunk>&>
{
	inline com::PoolToken<GeneralPurposeHeapMemoryChunk>& map(const GeneralPurposeHeapMemoryChunk& p_chunk, com::PoolToken<GeneralPurposeHeapMemoryChunk>& p_chunktoken) { return p_chunktoken; }
};

//TODO - defragmentation (sorting free_chunks by offset and merging if neghbors)
template<class Allocator = HeapAllocator>
struct GeneralPurposeHeap
{
	static_assert(std::is_base_of<IAllocator, Allocator>::value, "Allocator must implements IAllocator.");

	Allocator allocator;
	size_t chunk_total_size;
	char* memory;
	com::Pool<GeneralPurposeHeapMemoryChunk> allocated_chunks;
	com::Vector<GeneralPurposeHeapMemoryChunk> free_chunks;

	inline void allocate(size_t p_size, Allocator& p_allocator = HeapAllocator())
	{
		this->chunk_total_size = p_size;
		this->allocator = p_allocator;
		this->memory = (char*)this->allocator.malloc(this->chunk_total_size);

		GeneralPurposeHeapMemoryChunk l_whole_chunk;
		l_whole_chunk.chunk_size = this->chunk_total_size;
		l_whole_chunk.offset = 0;

		this->allocated_chunks.allocate(0);
		this->free_chunks.allocate(0);

		this->free_chunks.push_back(l_whole_chunk);
	}

	inline void realloc(size_t p_newsize, Allocator& p_allocator = HeapAllocator())
	{
		if (this->chunk_total_size < p_newsize)
		{
			char* l_realloced_memory = (char*)p_allocator.realloc((void*)this->memory, p_newsize);
			if (l_realloced_memory)
			{
				this->memory = l_realloced_memory;
				GeneralPurposeHeapMemoryChunk l_new_free_block;
				l_new_free_block.chunk_size = p_newsize - this->chunk_total_size;
				l_new_free_block.offset = this->chunk_total_size;
				this->free_chunks.push_back(l_new_free_block);
			}
		}
	}

	inline void dispose()
	{
		this->allocator.free(this->memory);
		this->allocated_chunks.free();
		this->free_chunks.free();
	}

	template<class ElementType>
	inline void* resolve(com::PoolToken<GeneralPurposeHeapMemoryChunk> p_memory)
	{
		return (ElementType*)(this->memory + this->allocated_chunks[p_memory].offset);
	};

	//TODO -> having multiple free_chunks indexed by allocation size range ?
	//        to avoid fragmentation of huge chunk for a very small size.
	template<class ReturnType = com::PoolToken<GeneralPurposeHeapMemoryChunk>&, class MemoryChunkMapper = DefaultMemoryChunkMapper >
	inline bool allocate_element(size_t p_size, ReturnType* out_chunk, MemoryChunkMapper& p_memorychunk_mapper = DefaultMemoryChunkMapper())
	{
		// static_assert(std::is_base_of<IMemoryChunkMapper<ReturnType>, MemoryChunkMapper>::value, "MemoryChunkMapper must implements IMemoryChunkMapper.");

		for (size_t i = 0; i < this->free_chunks.Size; i++)
		{
			GeneralPurposeHeapMemoryChunk& l_chunk = this->free_chunks[i];

			if (l_chunk.chunk_size > p_size)
			{
				size_t l_split_size = l_chunk.chunk_size - p_size;

				GeneralPurposeHeapMemoryChunk l_new_allocated_chunk;
				l_new_allocated_chunk.chunk_size = l_chunk.chunk_size - l_split_size;
				l_new_allocated_chunk.offset = l_chunk.offset;

				*out_chunk = p_memorychunk_mapper.map(l_new_allocated_chunk, this->allocated_chunks.alloc_element(l_new_allocated_chunk));

				this->free_chunks[i].chunk_size = l_split_size;
				this->free_chunks[i].offset = l_new_allocated_chunk.offset + l_new_allocated_chunk.chunk_size;

				return true;
			}
			else if (l_chunk.chunk_size == p_size)
			{
				*out_chunk = p_memorychunk_mapper.map(l_chunk, this->allocated_chunks.alloc_element(l_chunk));
				this->free_chunks.erase_at(i);
				return true;
			}
		}

		return false;
	};

	inline void release_element(const com::PoolToken<GeneralPurposeHeapMemoryChunk> p_buffer)
	{
		this->free_chunks.push_back(this->allocated_chunks[p_buffer]);
		this->allocated_chunks.release_element(p_buffer);
	}
};
