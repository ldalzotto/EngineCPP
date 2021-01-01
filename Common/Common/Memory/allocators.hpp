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

	inline void* calloc(const size_t p_allocationSize)
	{
		return ::calloc(1, p_allocationSize);
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

struct HeapZeroingAllocator : public IAllocator
{
	inline void* malloc(const size_t p_allocSize)
	{
		return this->calloc(p_allocSize);
	}

	inline void* calloc(const size_t p_allocationSize)
	{
		return ::calloc(1, p_allocationSize);
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


struct NoAllocator : public IAllocator
{
	inline void* malloc(const size_t p_allocSize)
	{
		return nullptr;
	}

	inline void* calloc(const size_t p_allocationSize)
	{
		return nullptr;
	}

	inline void* realloc(void* p_initialMemory, const size_t p_allocSize)
	{
		return nullptr;
	}

	inline void free(void* p_memory)
	{
		
	}
};