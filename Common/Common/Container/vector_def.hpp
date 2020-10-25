#pragma once

#include <Common/Memory/allocators.hpp>
#include <Common/Memory/memory_slice.hpp>

namespace com
{
	template<class TYPE, class Allocator = HeapAllocator>
	struct Vector
	{
		TYPE* Memory;
		size_t Size;
		size_t Capacity;

		Allocator* allocator;

		Vector();
		Vector(size_t p_initialSize, Allocator* p_allocator = &GlobalHeapAllocator);
		void dispose();
		char resize(const size_t p_newCapacity);
		char pushBack(const TYPE& p_element);
		char insert_at(MemorySlice<TYPE>& p_elements, const size_t p_index);
		char insert_at(const TYPE& p_element, const size_t p_index);
		char erase_at(const size_t p_index);
		char swap(const size_t p_left, const size_t p_right);
	};

}