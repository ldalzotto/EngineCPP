#pragma once

#include "Common/Memory/heap.hpp"
#include "pool.hpp"


template<class ElementHeaderType>
struct VaryingPoolHeader
{
	ElementHeaderType header;
	com::TPoolToken<GeneralPurposeHeapMemoryChunk> chunk;
};


template<class ElementHeaderType, class Allocator = HeapAllocator>
struct VaryingPool
{
	GeneralPurposeHeap2<GeneralPurposeHeap2_Times2Allocation, Allocator> heap;
	com::Pool<VaryingPoolHeader<ElementHeaderType>, Allocator> memory;

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
		l_return.heap = this->heap.clone();
		l_return.memory = this->memory.clone();
		return l_return;
	};


	template<class ElementType>
	inline com::TPoolToken<VaryingPoolHeader<ElementHeaderType>> alloc_element(const ElementHeaderType& p_header, const ElementType& p_element)
	{
		return this->alloc_element(p_header, (const char*)&p_element, sizeof(ElementType));
	};

	inline com::TPoolToken<VaryingPoolHeader<ElementHeaderType>> alloc_element(const ElementHeaderType& p_header, const char* p_element, const size_t p_element_size)
	{
		VaryingPoolHeader<ElementHeaderType> l_header;
		this->heap.allocate_element(p_element_size, &l_header.chunk, p_element);
		l_header.header = p_header;
		return this->memory.alloc_element(l_header);
	};

	inline void release_element(const com::TPoolToken<VaryingPoolHeader<ElementHeaderType>>& p_element)
	{
		VaryingPoolHeader<ElementHeaderType>& l_header = this->memory[p_old_element];
		this->heap.release_element(l_header.chunk);
		this->memory.release_element(p_element);
	};

	inline void reallocate_element(const com::TPoolToken<VaryingPoolHeader<ElementHeaderType>>& p_old_element, const ElementHeaderType& p_header, const size_t p_element_size)
	{
		VaryingPoolHeader<ElementHeaderType>& l_header = this->memory[p_old_element];
		com::TPoolToken<GeneralPurposeHeapMemoryChunk> l_allocated_chunk;
		this->heap.reallocate_element(l_header.chunk, p_element_size, l_allocated_chunk);
		l_header.header = p_header;
		l_header.chunk = l_allocated_chunk;
	};

	inline VaryingPoolHeader<ElementHeaderType>& get_header(const com::TPoolToken<VaryingPoolHeader<ElementHeaderType>>& p_index)
	{
		return this->memory[p_index];
	};

	template<class ElementType>
	inline ElementType* get_element(const com::TPoolToken<VaryingPoolHeader<ElementHeaderType>>& p_index)
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
		return this->memory.size();
	};
};