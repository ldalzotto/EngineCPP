#pragma once

#include "varying_vector.hpp"

struct NestedVector_SizeByte
{
	size_t size;
	size_t capacity;

	inline static NestedVector_SizeByte build(const size_t& p_size, const size_t& p_capacity)
	{
		return NestedVector_SizeByte{ p_size, p_capacity };
	};

	inline static NestedVector_SizeByte build()
	{
		return NestedVector_SizeByte{ 0, 0 };
	};

	inline NestedVector_SizeByte* size_byte()
	{
		return this;
	};
};

template<class Nestedvector_ElementHeader_Type>
inline char* Nestedvector_ElementHeader_get_vector(Nestedvector_ElementHeader_Type* p_header)
{
	return ((char*)p_header) + sizeof(Nestedvector_ElementHeader_Type);
}

template<class VectorElementHeader = NestedVector_SizeByte, class Allocator = HeapAllocator>
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

	inline void free_checked()
	{
		this->varying_vector.free_checked();
	}

	inline size_t size()
	{
		return this->varying_vector.size();
	}
};

template<class VectorElementHeader = NestedVector_SizeByte, class Allocator = HeapAllocator>
inline void VectorOfVector_push_back_vector(VectorOfVector<VectorElementHeader, Allocator>* thiz)
{
	thiz->varying_vector.push_back(VectorElementHeader::build());
};

template<class VectorElementHeader = NestedVector_SizeByte, class Allocator = HeapAllocator>
inline VectorElementHeader* VectorOfVector_get_nested_vector_element_header(VectorOfVector<VectorElementHeader, Allocator>* thiz, const size_t& p_index)
{
	return (VectorElementHeader*)&thiz->varying_vector.get(p_index);
};

template<class VectorElementHeader = NestedVector_SizeByte, class Allocator = HeapAllocator>
inline com::Vector<char, NoAllocator> VectorOfVector_NestedVector_build_vector(VectorOfVector<VectorElementHeader, Allocator>* thiz, const size_t& p_nested_vector)
{
	VectorElementHeader* l_header = VectorOfVector_get_nested_vector_element_header(thiz, p_nested_vector);
	NestedVector_SizeByte* l_size = l_header->size_byte();
	VaryingVector2Chunk& l_chunk = thiz->varying_vector.chunks[p_nested_vector];
	return com::Vector<char, NoAllocator>::build(Nestedvector_ElementHeader_get_vector(l_header), l_size->size, l_chunk.size - sizeof(NestedVector_SizeByte), NoAllocator());
};

template<class VectorElementHeader = NestedVector_SizeByte, class Allocator = HeapAllocator>
inline void VectorOfVector_NestedVector_push_back(VectorOfVector<VectorElementHeader, Allocator>* thiz, const size_t p_nested_vector, const char* p_element, const size_t& p_size)
{
	NestedVector_SizeByte l_old_size = *(VectorOfVector_get_nested_vector_element_header(thiz, p_nested_vector)->size_byte());

	{
		VaryingVector2Chunk l_old_chunk = thiz->varying_vector.chunks[p_nested_vector];
		thiz->varying_vector.resize_element(p_nested_vector, l_old_chunk.size + p_size);
	}

	VaryingVector2Chunk& l_chunk = thiz->varying_vector.chunks[p_nested_vector];

	com::Vector<char, NoAllocator> l_vector = VectorOfVector_NestedVector_build_vector(thiz, p_nested_vector);
	l_vector.push_back_unsafe(com::MemorySlice<char>((char*)p_element, p_size));
	*(VectorOfVector_get_nested_vector_element_header(thiz, p_nested_vector)->size_byte()) = NestedVector_SizeByte::build(l_vector.Size, l_vector.Capacity);
};

template<class VectorElementHeader = NestedVector_SizeByte, class Allocator = HeapAllocator>
inline void VectorOfVector_NestedVector_erase_at(VectorOfVector<VectorElementHeader, Allocator>* thiz, const size_t p_nested_vector, const size_t& p_index, const size_t& p_size)
{
	com::Vector<char, NoAllocator> l_vector = VectorOfVector_NestedVector_build_vector(thiz, p_nested_vector);
	l_vector.erase_at(p_index, p_size);
	*(VectorOfVector_get_nested_vector_element_header(thiz, p_nested_vector)->size_byte()) = NestedVector_SizeByte::build(l_vector.Size, l_vector.Capacity);
};

template<class ElementType, class VectorElementHeader = NestedVector_SizeByte, class Allocator = HeapAllocator>
inline ElementType& VectorOfVector_NestedVector_get(VectorOfVector<VectorElementHeader, Allocator>* thiz, const size_t p_nested_vector, const size_t& p_index)
{
	VectorElementHeader* l_header = VectorOfVector_get_nested_vector_element_header(thiz, p_nested_vector);
	char* l_memory = Nestedvector_ElementHeader_get_vector(l_header);
#if CONTAINER_BOUND_TEST
	{
		NestedVector_SizeByte* l_size = l_header->size_byte();
		if (p_index >= l_size->size)
		{
			abort();
		}
	}
#endif
	return (((ElementType*)l_memory)[p_index]);
};




struct TNestedVector_ElementHeader
{
	NestedVector_SizeByte _size_byte;
	size_t element_count;

	inline static TNestedVector_ElementHeader build()
	{
		return TNestedVector_ElementHeader{ NestedVector_SizeByte::build() , 0 };
	};

	inline NestedVector_SizeByte* size_byte()
	{
		return &this->_size_byte;
	};
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
	VectorOfVector<TNestedVector_ElementHeader, Allocator> vector_of_vector;

	inline void allocate(const size_t& p_memory_size, const size_t& p_headers_size, const Allocator& p_allocator = HeapAllocator())
	{
		this->vector_of_vector.allocate(p_memory_size, p_headers_size, p_allocator);
	}

	inline void free()
	{
		this->vector_of_vector.free();
	}

	inline void free_checked()
	{
		this->vector_of_vector.free_checked();
	}

	inline TNestedVector<ElementType> push_back_vector()
	{
		VectorOfVector_push_back_vector(&this->vector_of_vector);
		return TNestedVector<ElementType>::build(this->vector_of_vector.varying_vector.size() - 1);
	};

	inline void nested_vector_push_back(const TNestedVector<ElementType>& p_nested_vector, const ElementType& p_element)
	{
		VectorOfVector_NestedVector_push_back(&this->vector_of_vector, p_nested_vector.nested_vector_index, (const char*)(&p_element), sizeof(ElementType));
		VectorOfVector_get_nested_vector_element_header(&this->vector_of_vector, p_nested_vector.nested_vector_index)->element_count += 1;
	};

	inline void nested_vector_erase_at(const TNestedVector<ElementType>& p_nested_vector, const size_t& p_index, const size_t& p_size)
	{
		VectorOfVector_NestedVector_erase_at(&this->vector_of_vector, p_nested_vector.nested_vector_index, p_index * sizeof(ElementType), p_size * sizeof(ElementType));
		VectorOfVector_get_nested_vector_element_header(&this->vector_of_vector, p_nested_vector.nested_vector_index)->element_count -= p_size;
	};

	inline ElementType& nested_vector_get(const TNestedVector<ElementType>& p_nested_vector, const size_t p_index)
	{
		return VectorOfVector_NestedVector_get<ElementType>(&this->vector_of_vector, p_nested_vector.nested_vector_index, p_index * sizeof(ElementType));
	};

	inline size_t nested_vector_size(const TNestedVector<ElementType>& p_nested_vector)
	{
		return VectorOfVector_get_nested_vector_element_header(&this->vector_of_vector, p_nested_vector.nested_vector_index)->element_count;
	};
	
};