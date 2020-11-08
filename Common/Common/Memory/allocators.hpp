#pragma once

#include <stdlib.h>

struct IAllocator 
{
	void* malloc(const size_t p_allocSize);
	void* realloc(void* p_initialMemory, const size_t p_allocSize);
	void free(void* p_memory);
};

struct HeapAllocator : public IAllocator
{
	inline void* malloc(const size_t p_allocSize)
	{
		return ::malloc(p_allocSize);
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
