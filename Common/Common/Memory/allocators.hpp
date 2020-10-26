#pragma once

#include <stdlib.h>

struct HeapAllocator
{
	inline void* malloc(const size_t p_allocSize)
	{
		return ::operator new (p_allocSize);
	}

	inline void* realloc(void* p_initialMemory, const size_t p_allocSize)
	{
		return ::realloc(p_initialMemory, p_allocSize);
	}

	inline void free(void* p_memory)
	{
		::free(p_memory);
	}
};