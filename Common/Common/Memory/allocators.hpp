#pragma once

#include <stdlib.h>

struct HeapAllocator
{
	inline void* malloc(const size_t p_allocSize)
	{
		return ::malloc(p_allocSize);
	}

	inline void* realloc(void* p_initialMemory, const size_t p_allocSize)
	{
		return ::realloc(p_initialMemory, p_allocSize);
	}
};

HeapAllocator GlobalHeapAllocator = HeapAllocator();

struct HeapOneTimeAllocator
{
	char Counter = 0;
	inline void* malloc(const size_t p_allocSize)
	{
		if (this->Counter > 0) { return nullptr; }
		this->Counter += 1;
		return malloc(p_allocSize);
	}

	inline void* realloc(void* p_initialMemory, const size_t p_allocSize)
	{
		return nullptr;
	}
};
