#pragma once

#include "Common/Memory/heap.hpp"

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
		l_return.heap = this->heap.clone();
		l_return.memory = this->memory.clone();
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