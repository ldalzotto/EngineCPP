#pragma once

#include "varying_pool.hpp"
#include "token.hpp"

struct PoolOfVector_ElementHeader
{
	size_t Size;
	size_t Capacity;

	inline static PoolOfVector_ElementHeader build(size_t p_size, size_t p_capacity)
	{
		return PoolOfVector_ElementHeader{ p_size, p_capacity };
	};
};

template<class Allocator = HeapAllocator>
struct PoolOfVector
{
	VaryingPool<PoolOfVector_ElementHeader, Allocator> varying_vector;

	inline static PoolOfVector<Allocator> allocate(size_t p_initial_size, Allocator& p_allocator = HeapAllocator())
	{
		PoolOfVector<Allocator> l_return;
		l_return.varying_vector.allocate(p_initial_size, p_allocator);
		return l_return;
	};

	inline void free()
	{
		this->varying_vector.free();
	};

	inline PoolOfVector<Allocator> clone()
	{
		return PoolOfVector<Allocator>{ this->varying_vector.clone() };
	};

	template<class ElementType>
	inline TToken<com::Vector<ElementType, NoAllocator>> allocate_vector(size_t p_capacity)
	{
		PoolOfVector_ElementHeader l_element_header = PoolOfVector_ElementHeader::build(0, p_capacity);
		return TToken_build<com::Vector<ElementType, NoAllocator>>(this->varying_vector.alloc_element(l_element_header, (char*)this, sizeof(ElementType) * l_element_header.Capacity).val);
	};

	template<class ElementType>
	inline void vector_push_back(TToken<com::Vector<ElementType, NoAllocator>>& p_vector, const ElementType& p_element)
	{
		VaryingPoolHeader<PoolOfVector_ElementHeader>& l_header = this->varying_vector.get_header(p_vector.val);
		char* l_memory = this->varying_vector.get_element<char>(l_header.chunk);

		com::Vector<ElementType, NoAllocator> l_vector = com::Vector<ElementType, NoAllocator>::build((ElementType*)l_memory, l_header.header.Size, l_header.header.Capacity, NoAllocator());
		if (!l_vector.push_back(p_element))
		{
			size_t l_new_capacity = l_header.header.Capacity == 0 ? 1 : l_header.header.Capacity * 2;
			this->varying_vector.reallocate_element(p_vector.val, PoolOfVector_ElementHeader::build(l_header.header.Size, l_new_capacity), sizeof(ElementType) * l_new_capacity);
			this->vector_push_back<ElementType>(p_vector, p_element);
		}
		else
		{
			l_header.header.Size = l_vector.Size;
		}
	};

	template<class ElementType>
	inline ElementType& vector_get(TToken<com::Vector<ElementType, NoAllocator>>& p_vector, const size_t p_index)
	{
		VaryingPoolHeader<PoolOfVector_ElementHeader>& l_header = this->varying_vector.get_header(p_vector.val);
		char* l_memory = this->varying_vector.get_element<char>(l_header.chunk);
		com::Vector<ElementType, NoAllocator> l_vector = com::Vector<ElementType, NoAllocator>::build((ElementType*)l_memory, l_header.header.Size, l_header.header.Capacity, NoAllocator());
		return l_vector.operator[](p_index);
	};

};