#pragma once

#include "nested_vector.hpp"
#include "pool_def.hpp"

template<class ElementType, class Allocator = HeapAllocator>
struct PoolOfVector
{
	TVectorOfVector<ElementType, Allocator> Memory;
	com::Vector<TNestedVector<ElementType>, Allocator> FreeBlocks;

	inline void allocate(const size_t& p_memory_size, const size_t& p_headers_size, const size_t& p_freeblocks_size, const Allocator& p_allocator = HeapAllocator())
	{
		this->Memory.allocate(p_memory_size, p_headers_size, p_allocator);
		this->FreeBlocks.allocate(p_freeblocks_size, p_allocator);
	};

	inline void free()
	{
		this->Memory.free();
		this->FreeBlocks.free();
	};

	inline void free_checked()
	{
#if CONTAINER_MEMORY_TEST
		if (this->Memory.vector_of_vector.size() != this->FreeBlocks.Size)
		{
			abort();
		};
#endif

		this->Memory.free();
		this->FreeBlocks.free();
	};

	inline TNestedVector<ElementType> alloc_element()
	{
		if (this->FreeBlocks.Size > 0)
		{
			TNestedVector<ElementType> l_token = this->FreeBlocks[this->FreeBlocks.Size - 1];
			this->FreeBlocks.erase_at(this->FreeBlocks.Size - 1, 1);
			return l_token;
		}
		else
		{
			return this->Memory.push_back_vector();
		}
	};

	inline void release_element(const TNestedVector<ElementType>& p_token)
	{
		this->Memory.nested_vector_clear(p_token);
		this->FreeBlocks.push_back(p_token);
	};

};