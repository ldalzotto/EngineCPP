#pragma once

#include <type_traits>
#include "allocators.hpp"
#include "Common/Container/pool.hpp"

struct GeneralPurposeHeapMemoryChunk
{
	size_t chunk_size = -1;
	size_t offset = -1;
};

struct AllocationAlignementConstraint
{
	size_t alignment_modulo;

	inline AllocationAlignementConstraint(const size_t p_alignment_modulo) { this->alignment_modulo = p_alignment_modulo; }
};

template<class ReturnType>
struct IMemoryChunkMapper
{
	ReturnType map(const GeneralPurposeHeapMemoryChunk& p_chunk, com::TPoolToken<GeneralPurposeHeapMemoryChunk>& p_chunktoken);
};

struct DefaultMemoryChunkMapper : public IMemoryChunkMapper<com::TPoolToken<GeneralPurposeHeapMemoryChunk>&>
{
	inline com::TPoolToken<GeneralPurposeHeapMemoryChunk>& map(const GeneralPurposeHeapMemoryChunk& p_chunk, com::TPoolToken<GeneralPurposeHeapMemoryChunk>& p_chunktoken) { return p_chunktoken; }
};

//TODO - defragmentation (sorting free_chunks by offset and merging if neghbors)
template<class Allocator = HeapAllocator>
struct GeneralPurposeHeap
{
	static_assert(std::is_base_of<IAllocator, Allocator>::value, "Allocator must implements IAllocator.");

	com::Vector<char, Allocator> memory;
	com::Pool<GeneralPurposeHeapMemoryChunk> allocated_chunks;
	com::Vector<GeneralPurposeHeapMemoryChunk> free_chunks;

	inline void allocate(size_t p_size, Allocator& p_allocator = HeapAllocator())
	{
		this->memory.allocate(p_size, p_allocator);
		this->memory.Size = this->memory.Capacity;

		GeneralPurposeHeapMemoryChunk l_whole_chunk;
		l_whole_chunk.chunk_size = this->memory.Size;
		l_whole_chunk.offset = 0;

		this->allocated_chunks.allocate(0);
		this->free_chunks.allocate(0);

		this->free_chunks.push_back(l_whole_chunk);
	}

	inline void realloc(size_t p_newsize, Allocator& p_allocator = HeapAllocator())
	{
		size_t l_old_size = this->memory.Size;
		if (this->memory.resize(p_newsize))
		{
			this->memory.Size = this->memory.Capacity;
			GeneralPurposeHeapMemoryChunk l_new_free_block;
			l_new_free_block.chunk_size = p_newsize - l_old_size;
			l_new_free_block.offset = l_old_size;
			this->free_chunks.push_back(l_new_free_block);
		}
	}

	inline GeneralPurposeHeap<Allocator> clone()
	{
		GeneralPurposeHeap<Allocator> l_return;
		l_return.memory = this->memory.clone();
		l_return.allocated_chunks = this->allocated_chunks.clone();
		l_return.free_chunks = this->free_chunks.clone();
		return l_return;
	};

	inline void dispose()
	{
		this->memory.free();
		this->allocated_chunks.free();
		this->free_chunks.free();
	};

	template<class ElementType>
	inline ElementType* map(com::TPoolToken<GeneralPurposeHeapMemoryChunk> p_memory)
	{
		return (ElementType*)(this->memory.Memory + this->allocated_chunks[p_memory].offset);
	};

	inline GeneralPurposeHeapMemoryChunk& resolve_allocated_chunk(const com::TPoolToken<GeneralPurposeHeapMemoryChunk> p_memory)
	{
		return this->allocated_chunks[p_memory];
	};

	//TODO -> having multiple free_chunks indexed by allocation size range ?
	//        to avoid fragmentation of huge chunk for a very small size.
	template<class ReturnType = com::TPoolToken<GeneralPurposeHeapMemoryChunk>&, class MemoryChunkMapper = DefaultMemoryChunkMapper >
	inline bool allocate_element(size_t p_size, ReturnType* out_chunk, MemoryChunkMapper& p_memorychunk_mapper = DefaultMemoryChunkMapper())
	{
		// static_assert(std::is_base_of<IMemoryChunkMapper<ReturnType>, MemoryChunkMapper>::value, "MemoryChunkMapper must implements IMemoryChunkMapper.");

		for (size_t i = 0; i < this->free_chunks.Size; i++)
		{
			GeneralPurposeHeapMemoryChunk& l_chunk = this->free_chunks[i];

			if (l_chunk.chunk_size > p_size)
			{
				GeneralPurposeHeapMemoryChunk l_new_allocated_chunk;

				this->slice_memorychunk(l_chunk, l_chunk.offset + p_size, l_new_allocated_chunk, l_chunk);

				*out_chunk = p_memorychunk_mapper.map(l_new_allocated_chunk, this->allocated_chunks.alloc_element(l_new_allocated_chunk));

				return true;
			}
			else if (l_chunk.chunk_size == p_size)
			{
				*out_chunk = p_memorychunk_mapper.map(l_chunk, this->allocated_chunks.alloc_element(l_chunk));
				this->free_chunks.erase_at(i, 1);
				return true;
			}
		}

		return false;
	};

	inline bool allocate_element(size_t p_size, AllocationAlignementConstraint& p_alignment_constraint, com::PoolToken* out_token)
	{
		for (size_t i = 0; i < this->free_chunks.Size; i++)
		{
			GeneralPurposeHeapMemoryChunk& l_chunk = this->free_chunks[i];

			if (l_chunk.chunk_size > p_size)
			{
				size_t l_offset_modulo = (l_chunk.offset % p_alignment_constraint.alignment_modulo);
				if (l_offset_modulo == 0)
				{
					// create one free chunk (after)

					GeneralPurposeHeapMemoryChunk l_new_allocated_chunk;
					this->slice_memorychunk(l_chunk, l_chunk.offset + p_size, l_new_allocated_chunk, l_chunk);
					*out_token = this->allocated_chunks.alloc_element(l_new_allocated_chunk);
					return true;
				}
				else
				{
					size_t l_chunk_offset_delta = p_alignment_constraint.alignment_modulo - l_offset_modulo;
					// Does the offsetted new memory is able to be allocated in the chunk ?
					if (l_chunk.chunk_size > (p_size + l_chunk_offset_delta)) //offsetted chunk is in the middle of the free chunk
					{
						//create two free chunk (before and after)
						GeneralPurposeHeapMemoryChunk l_new_allocated_chunk, l_free_chunk, l_tmp_chunk;

						this->slice_memorychunk(l_chunk, l_chunk.offset + l_chunk_offset_delta, l_chunk, l_tmp_chunk);
						this->slice_memorychunk(l_tmp_chunk, l_tmp_chunk.offset + p_size, l_new_allocated_chunk, l_free_chunk);

						*out_token = this->allocated_chunks.alloc_element(l_new_allocated_chunk);
						this->free_chunks.push_back(l_free_chunk);

						return true;
					}
					else if (l_chunk.chunk_size == (p_size + l_chunk_offset_delta)) //offsetted chunk end matches perfectly the end of the free chunk
					{
						GeneralPurposeHeapMemoryChunk l_new_allocated_chunk;
						this->slice_memorychunk(l_chunk, l_chunk.offset + l_chunk_offset_delta, l_chunk, l_new_allocated_chunk);
						*out_token = this->allocated_chunks.alloc_element(l_new_allocated_chunk);
						
						return true;
					}
				}
			}
			else if (l_chunk.chunk_size == p_size)
			{
				size_t l_offset_modulo = (l_chunk.offset % p_alignment_constraint.alignment_modulo);
				if (l_offset_modulo == 0)
				{
					*out_token = this->allocated_chunks.alloc_element(l_chunk);
					this->free_chunks.erase_at(i, 1);
					return true;
				}
			}
		}

		return false;
	};

	inline void release_element(const com::TPoolToken<GeneralPurposeHeapMemoryChunk> p_buffer)
	{
		this->free_chunks.push_back(this->allocated_chunks[p_buffer]);
		this->allocated_chunks.release_element(p_buffer);
	};

private:
	void slice_memorychunk(const GeneralPurposeHeapMemoryChunk& p_source_chunk, size_t p_slice_offset, GeneralPurposeHeapMemoryChunk& out_left, GeneralPurposeHeapMemoryChunk& out_right)
	{
		size_t l_source_chunk_size = p_source_chunk.chunk_size;

		out_left.chunk_size = (p_slice_offset - p_source_chunk.offset);
		out_left.offset = p_source_chunk.offset;

		out_right.chunk_size = l_source_chunk_size - out_left.chunk_size;
		out_right.offset = p_slice_offset;
	}
};
