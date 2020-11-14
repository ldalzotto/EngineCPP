#pragma once

#include <type_traits>
#include "vector_def.hpp"
#include "Common/Functional/Optional.hpp"

namespace com
{
	template<class TYPE>
	struct PoolToken
	{
		size_t Index;

		PoolToken() : Index{(size_t)-1} {};
		PoolToken(size_t p_index) : Index{p_index} {};
	};

	template<class TYPE, class Allocator = HeapAllocator>
	struct Pool
	{
		static_assert(std::is_base_of<IAllocator, Allocator>::value, "Allocator must implements IAllocator.");

		Vector<TYPE, Allocator> Memory;
		Vector<size_t, HeapAllocator> FreeBlocks;

		Pool() = default;
		void allocate(size_t p_initialSize, const Allocator& p_allocator = Allocator());
		void free();
		Pool<TYPE, Allocator> clone();
		Pool<TYPE, Allocator> move();
		Vector<size_t, HeapAllocator> clone_freeblocks();
		size_t size();
		TYPE& operator[](size_t i);
		TYPE& operator[](const PoolToken<TYPE> i);
		PoolToken<TYPE> alloc_element(const TYPE& p_element);
		void release_element(const PoolToken<TYPE>& p_element);
		TYPE& resolve(const PoolToken<TYPE>& p_element);
	};

	template<class TYPE, class Allocator = HeapAllocator>
	struct OptionalPool
	{
		static_assert(std::is_base_of<IAllocator, Allocator>::value, "Allocator must implements IAllocator.");

		Pool<Optional<TYPE>, Allocator> pool;

		OptionalPool();
		void allocate(size_t p_initialSize, const Allocator& p_allocator = Allocator());
		void free();
		size_t size();
		Optional<TYPE>& operator[](size_t i);
		Optional<TYPE>& operator[](const PoolToken<Optional<TYPE>> i);
		PoolToken<Optional<TYPE>> alloc_element(const TYPE& p_element);
		void release_element(const PoolToken<Optional<TYPE>>& p_element);
	};
}

