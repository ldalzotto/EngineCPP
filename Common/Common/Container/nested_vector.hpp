#pragma once

#include "varying_vector.hpp"

template<class Allocator = HeapAllocator>
struct VectorOfVector
{
	VaryingVector2<Allocator> varying_vector;

	inline void allocate(const size_t& p_memory_size, const size_t& p_headers_size, const Allocator& p_allocator = HeapAllocator())
	{
		this->varying_vector.allocate(p_memory_size, p_headers_size, p_allocator);
	}

	inline void free()
	{
		this->varying_vector.free();
	}
};

struct NestedVector_Size
{
	size_t size;
	size_t capacity;

	inline static NestedVector_Size build(const size_t& p_size, const size_t& p_capacity)
	{
		return NestedVector_Size{ p_size, p_capacity };
	};
};

template<class Allocator = HeapAllocator>
inline void VectorOfVector_push_back_vector(VectorOfVector<Allocator>* thiz)
{
	NestedVector_Size l_size = NestedVector_Size::build(0, 0);
	thiz->varying_vector.push_back(l_size);
};


template<class Allocator = HeapAllocator>
inline NestedVector_Size* VectorOfVector_get_nested_vector_size(VectorOfVector<Allocator>* thiz, const size_t& p_index)
{
	return (NestedVector_Size*)&thiz->varying_vector.get(p_index);
};

template<class Allocator = HeapAllocator>
inline char* VectorOfVector_get_nested_vector_memory(VectorOfVector<Allocator>* thiz, const size_t& p_index)
{
	return (char*)&thiz->varying_vector.get(p_index) + sizeof(NestedVector_Size);
};

template<class Allocator = HeapAllocator>
inline com::Vector<char, NoAllocator> VectorOfVector_NestedVector_build_vector(VectorOfVector<Allocator>* thiz, const size_t& p_nested_vector)
{
	NestedVector_Size* l_size = VectorOfVector_get_nested_vector_size(thiz, p_nested_vector);
	VaryingVector2Chunk& l_chunk = thiz->varying_vector.chunks[p_nested_vector];
	return com::Vector<char, NoAllocator>::build(VectorOfVector_get_nested_vector_memory(thiz, p_nested_vector), l_size->size, l_chunk.size - sizeof(NestedVector_Size), NoAllocator());
};

template<class Allocator = HeapAllocator>
inline void VectorOfVector_NestedVector_push_back(VectorOfVector<Allocator>* thiz, const size_t p_nested_vector, const char* p_element, const size_t& p_size)
{
	NestedVector_Size l_old_size = *VectorOfVector_get_nested_vector_size(thiz, p_nested_vector);

	{
		VaryingVector2Chunk l_old_chunk = thiz->varying_vector.chunks[p_nested_vector];
		thiz->varying_vector.resize_element(p_nested_vector, l_old_chunk.size + p_size);
	}

	VaryingVector2Chunk& l_chunk = thiz->varying_vector.chunks[p_nested_vector];

	com::Vector<char, NoAllocator> l_vector = VectorOfVector_NestedVector_build_vector(thiz, p_nested_vector);
	l_vector.push_back_unsafe(com::MemorySlice<char>((char*)p_element, p_size));

	*VectorOfVector_get_nested_vector_size(thiz, p_nested_vector) = NestedVector_Size::build(l_vector.Size, l_vector.Capacity);
};

template<class ElementType, class Allocator = HeapAllocator>
inline ElementType& VectorOfVector_NestedVector_get(VectorOfVector<Allocator>* thiz, const size_t p_nested_vector, const size_t& p_index)
{
	char* l_memory = VectorOfVector_get_nested_vector_memory(thiz, p_nested_vector);
#if CONTAINER_BOUND_TEST
	{
		NestedVector_Size* l_size = VectorOfVector_get_nested_vector_size(thiz, p_nested_vector);
		if (p_index >= l_size->size)
		{
			abort();
		}
	}
#endif
	return (((ElementType*)l_memory)[p_index]);
};

template<class Allocator = HeapAllocator>
inline size_t VectorOfVector_NestedVector_size(VectorOfVector<Allocator>* thiz, const size_t p_nested_vector)
{
	return VectorOfVector_get_nested_vector_size(thiz, p_nested_vector)->size;
};


template<class ElementType>
struct TNestedVector
{
	size_t nested_vector_index;

	inline static TNestedVector<ElementType> build(const size_t& p_nested_vector_index)
	{
		return TNestedVector<ElementType>{p_nested_vector_index};
	};
};

template<class ElementType, class Allocator = HeapAllocator>
struct TVectorOfVector
{
	VectorOfVector<Allocator> vector_of_vector;

	inline void allocate(const size_t& p_memory_size, const size_t& p_headers_size, const Allocator& p_allocator = HeapAllocator())
	{
		this->vector_of_vector.allocate(p_memory_size, p_headers_size, p_allocator);
	}

	inline void free()
	{
		this->vector_of_vector.free();
	}

	inline TNestedVector<ElementType> push_back_vector()
	{
		VectorOfVector_push_back_vector(&this->vector_of_vector);
		return TNestedVector<ElementType>::build(this->vector_of_vector.varying_vector.size() - 1);
	};

	inline void nested_vector_push_back(const TNestedVector<ElementType>& p_nested_vector, const ElementType& p_element)
	{
		VectorOfVector_NestedVector_push_back(&this->vector_of_vector, p_nested_vector.nested_vector_index, (const char*)(&p_element), sizeof(ElementType));
	};

	inline ElementType& nested_vector_get(const TNestedVector<ElementType>& p_nested_vector, const size_t p_index)
	{
		return VectorOfVector_NestedVector_get<ElementType>(&this->vector_of_vector, p_nested_vector.nested_vector_index, p_index);
	};

	inline size_t nested_vector_size(const TNestedVector<ElementType>& p_nested_vector)
	{
		return VectorOfVector_NestedVector_size(&this->vector_of_vector, p_nested_vector);
	};
	
};