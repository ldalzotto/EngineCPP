#pragma once

#include <type_traits>
#include <Common/Memory/allocators.hpp>
#include <Common/Memory/memory_slice.hpp>

namespace com
{
	template<class TYPE, class Allocator = HeapAllocator>
	struct Vector
	{
		static_assert(std::is_base_of<IAllocator, Allocator>::value, "Allocator must implements IAllocator.");

		TYPE* Memory;
		size_t Size;
		size_t Capacity;

		Allocator allocator;

		Vector();
		void allocate(size_t p_initialSize, const Allocator& p_allocator = Allocator());
		void free();
		TYPE& operator[](size_t i);
		
		size_t capacity_in_bytes();
		char resize(const size_t p_newCapacity);
		char push_back(const TYPE &p_element);
		char insert_at(MemorySlice<TYPE>& p_elements, const size_t p_index);
		char insert_at(const TYPE& p_element, const size_t p_index);
		char erase_at(const size_t p_index);
		char swap(const size_t p_left, const size_t p_right);

	};

}