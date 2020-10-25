#pragma once

#include "vector_def.hpp"

namespace com
{
	template<class TYPE>
	struct PoolToken
	{
		size_t Index;

		PoolToken(size_t p_index) : Index{p_index} {};
	};

	template<class TYPE, class Allocator = HeapAllocator>
	struct Pool
	{
		Vector<TYPE, Allocator> Memory;
		Vector<size_t, HeapAllocator> FreeBlocks;

		Pool();
		Pool(size_t p_initialSize, Allocator* p_allocator = &GlobalHeapAllocator);
		PoolToken<TYPE> alloc_element(const TYPE& p_element);
		void release_element(const PoolToken<TYPE>& p_element);
		TYPE& resolve(const PoolToken<TYPE>& p_element);
	};
}

