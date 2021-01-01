#pragma once

#include "Common/Memory/heap.hpp"
#include "Common/Container/vector.hpp"

template<class ElementHeaderType>
struct VaryingVectorHeader
{
	ElementHeaderType header;
	com::TPoolToken<GeneralPurposeHeapMemoryChunk> chunk;
};


template<class ElementHeaderType, class Allocator = HeapAllocator>
struct VaryingVector
{
	GeneralPurposeHeap2<GeneralPurposeHeap2_Times2Allocation, Allocator> heap;
	com::Vector<VaryingVectorHeader<ElementHeaderType>, Allocator> memory;

	inline void allocate(size_t p_initial_size, Allocator& p_allocator = HeapAllocator())
	{
		this->heap.allocate(p_initial_size, p_allocator);
		this->memory.allocate(p_initial_size, p_allocator);
	};

	inline void free()
	{
		this->heap.dispose();
		this->memory.free();
	};

	inline VaryingVector<ElementHeaderType, Allocator> clone()
	{
		VaryingVector<ElementHeaderType, Allocator> l_return;
		l_return.heap    =  this->heap.clone()   ;
		l_return.memory  =  this->memory.clone() ;
		return l_return;
	};

	template<class ElementType>
	inline void push_back(const ElementHeaderType& p_header, const  ElementType& p_element)
	{
		this->push_back(p_header, (const char*)&p_element, sizeof(ElementType));
	};

	inline void push_back(const ElementHeaderType& p_header, const char* p_element, const size_t p_element_size)
	{
		VaryingVectorHeader<ElementHeaderType> l_header;
		this->heap.allocate_element(p_element_size, &l_header.chunk, p_element);
		l_header.header = p_header;
		this->memory.push_back(l_header);
	};

	inline VaryingVectorHeader<ElementHeaderType>& get_header(const size_t p_index)
	{
		return this->memory[p_index];
	};

	template<class ElementType>
	inline ElementType* get_element(const size_t p_index)
	{
		 return this->heap.map<ElementType>(this->get_header(p_index).chunk);
	};

	template<class ElementType>
	inline ElementType* get_element(com::TPoolToken<GeneralPurposeHeapMemoryChunk>& p_chunk)
	{
		return this->heap.map<ElementType>(p_chunk);
	};

	inline size_t size()
	{
		return this->memory.Size;
	};
};

struct VaryingVector2Chunk
{
	size_t offset;
	size_t size;

	inline static VaryingVector2Chunk build(const size_t& p_offset, const size_t& p_size)
	{
		return VaryingVector2Chunk{ p_offset, p_size };
	}
};

//TODO -> removing the VaryingVector (which is unnecessary complex) for this version
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

	inline void push_back(char* p_element, const size_t p_size)
	{
		this->chunks.push_back(VaryingVector2Chunk::build(this->memory.Size, p_size));
		this->memory.push_back(com::MemorySlice<char>(p_element, p_size));
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

	inline size_t size()
	{
		return this->chunks.Size;
	};

	inline char& get(const size_t p_index)
	{
		return this->memory[this->chunks[p_index].offset];
	};
	
};