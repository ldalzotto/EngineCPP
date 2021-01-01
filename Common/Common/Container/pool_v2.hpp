#pragma once

#include "token.hpp"

#include "vector_def.hpp"


template<class TYPE, class Allocator = HeapAllocator>
struct Pool_v2
{
	com::Vector<TYPE, Allocator> Memory;
	com::Vector<size_t, Allocator> FreeBlocks;
};

/*
	Pool<TYPE, Allocator> clone();
	Pool<TYPE, Allocator> move();
	Vector<size_t, Allocator> clone_freeblocks();
	size_t size();
	TPoolToken<TYPE> get_next_freentry();
*/

#define Pool_v2_template_header template<class TYPE, class Allocator = HeapAllocator>
#define Pool_v2_type Pool_v2<TYPE, Allocator>

Pool_v2_template_header
inline void pool_allocate(Pool_v2_type* thiz, const Allocator& p_allocator = Allocator())
{
	thiz->Memory.allocate(0, p_allocator);
	thiz->FreeBlocks.allocate(0, p_allocator);
};

Pool_v2_template_header
inline void pool_allocate(Pool_v2_type* thiz, const size_t p_initialiSize, const Allocator& p_allocator = Allocator())
{
	thiz->Memory.allocate(p_initialSize, p_allocator);
	thiz->FreeBlocks.allocate(0, p_allocator);
};

Pool_v2_template_header
inline void pool_free(Pool_v2_type* thiz)
{
	thiz->Memory.free();
	thiz->FreeBlocks.free();
};

Pool_v2_template_header
inline void pool_free_checked(Pool_v2_type* thiz)
{
#if CONTAINER_MEMORY_TEST
	if (thiz->Memory.Size != thiz->FreeBlocks.Size)
	{
		abort();
	}
#endif

	pool_free(thiz);
};

Pool_v2_template_header
inline TYPE* pool_get(Pool_v2_type* thiz, const TToken<TYPE>* p_token)
{
	return &thiz->Memory[p_token->val];
};


Pool_v2_template_header
inline TToken<TYPE> pool_alloc_element(Pool_v2_type* thiz, const TYPE* p_element)
{
	if (thiz->FreeBlocks.Size > 0)
	{
		size_t l_availableIndex = thiz->FreeBlocks[thiz->FreeBlocks.Size - 1];
		thiz->Memory[l_availableIndex] = *p_element;
		thiz->FreeBlocks.erase_at(thiz->FreeBlocks.Size - 1, 1);
		return TToken_build<TYPE>(l_availableIndex);
	}
	else
	{
		thiz->Memory.push_back(*p_element);
		return TToken_build<TYPE>(thiz->Memory.Size - 1);
	}
};

Pool_v2_template_header
inline void pool_release_element(Pool_v2_type* thiz, const TToken<TYPE>* p_element)
{
	thiz->FreeBlocks.push_back(p_element->val);
};

Pool_v2_template_header
inline bool pool_is_token_free(Pool_v2_type* thiz, const TToken<TYPE>* p_element)
{
	for (size_t i = 0; i < thiz->FreeBlocks.Size; i++)
	{
		if (thiz->FreeBlocks[i] == p_element->val)
		{
			return true;
		}
	}
	return false;
};