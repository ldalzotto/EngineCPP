#pragma once

#include "Common/Container/vector.hpp"

struct VaryingVector2Chunk
{
	size_t offset;
	size_t size;

	inline static VaryingVector2Chunk build(const size_t& p_offset, const size_t& p_size)
	{
		return VaryingVector2Chunk{ p_offset, p_size };
	}
};

template<class Allocator = HeapAllocator>
struct VaryingVector2
{
	com::Vector<char, Allocator> memory;
	com::Vector<VaryingVector2Chunk, Allocator> chunks;

	inline void allocate(const size_t& p_memory_size, const size_t& p_headers_size, const Allocator& p_allocator = HeapAllocator())
	{
		this->memory.allocate(p_memory_size, p_allocator);
		this->chunks.allocate(p_headers_size, p_allocator);
	};

	inline void free()
	{
		this->memory.free();
		this->chunks.free();
	};

	inline void free_checked()
	{
		this->memory.free_checked();
		this->chunks.free_checked();
	};

	inline void push_back(char* p_element, const size_t p_size)
	{
		this->chunks.push_back(VaryingVector2Chunk::build(this->memory.Size, p_size));
		this->memory.push_back(com::MemorySlice<char>(p_element, p_size));
	};

	template<class ElementType>
	inline void push_back(const ElementType& p_element)
	{
		this->push_back((char*)&p_element, sizeof(ElementType));
	};

	inline void erase_at(const size_t p_index, const size_t p_size)
	{
		VaryingVector2Chunk l_removed_chunk = VaryingVector2Chunk::build(this->chunks[p_index].offset, 0);

		for (size_t i = 0; i < p_size; i++)
		{
			l_removed_chunk.size += this->chunks[p_index + i].size;
		}

		for (size_t i = p_index + p_size; i < this->chunks.Size; i++)
		{
			this->chunks[i].offset -= l_removed_chunk.size;
		}


		this->memory.erase_at(l_removed_chunk.offset, l_removed_chunk.size);
		this->chunks.erase_at(p_index, p_size);
	};

	inline void resize_element(const size_t p_index, const size_t p_new_size)
	{
		VaryingVector2Chunk& l_updated_chunk = this->chunks[p_index];
		if (l_updated_chunk.size != p_new_size)
		{
			size_t l_size_delta = p_new_size - l_updated_chunk.size;
			if (((this->memory.size_in_bytes() + l_size_delta) >  this->memory.capacity_in_bytes()))
			{
				this->memory.resize((this->memory.Capacity * 2) + l_size_delta);
				this->resize_element(p_index, p_new_size);
			}
			else
			{
				if (p_new_size > l_updated_chunk.size)
				{
					this->memory.insert_at(com::MemorySlice<char>((char*)this, l_size_delta), l_updated_chunk.offset + l_updated_chunk.size);
				}
				else
				{
					this->memory.erase_at(l_updated_chunk.offset + l_updated_chunk.size + l_size_delta, -l_size_delta);
				}
				
				l_updated_chunk.size += l_size_delta;

				for (size_t i = p_index + 1; i < this->chunks.Size; i++)
				{
					this->chunks[i].offset += l_size_delta;
				}
				
			};
			
		}
	};

	inline size_t size()
	{
		return this->chunks.Size;
	};

	inline char& get(const size_t p_index)
	{
		return this->memory[this->chunks[p_index].offset];
	};
	
};
