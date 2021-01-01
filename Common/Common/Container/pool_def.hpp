#pragma once

#include <type_traits>
#include "vector_def.hpp"
#include "Common/Functional/Optional.hpp"
#include "Common/Memory/type_safety.hpp"

namespace com
{
	struct PoolToken : public SizetType
	{
		using SizetType::SizetType;
	};
	
	template<class TYPE>
	struct TPoolToken : public SizetType {
		using SizetType::SizetType;

		template<class CastedType>
		inline TPoolToken<CastedType> cast()
		{
			return TPoolToken<CastedType>(this->val);
		};

		template<class CastedType>
		inline const TPoolToken<CastedType> cast() const
		{
			return TPoolToken<CastedType>(this->val);
		};

		inline PoolToken to_pooltoken()
		{
			return PoolToken(this->val);
		};

		inline const PoolToken to_pooltoken() const
		{
			return PoolToken(this->val);
		};

		template<class OtherToken>
		inline bool equals(const OtherToken& p_other) const
		{
			return this->val == p_other.val;
		};
	};

	template<class TYPE, class Allocator = HeapAllocator>
	struct Pool
	{
		static_assert(std::is_base_of<IAllocator, Allocator>::value, "Allocator must implements IAllocator.");

		Vector<TYPE, Allocator> Memory;
		Vector<size_t, Allocator> FreeBlocks;

		Pool() = default;
		void allocate(size_t p_initialSize, const Allocator& p_allocator = Allocator());
		void free();
		void free_checked();
		Pool<TYPE, Allocator> clone();
		Pool<TYPE, Allocator> move();
		Vector<size_t, Allocator> clone_freeblocks();
		size_t size();
		TYPE& operator[](size_t i);
		TYPE& operator[](const TPoolToken<TYPE> i);
		TPoolToken<TYPE> alloc_element(const TYPE& p_element);
		void release_element(const TPoolToken<TYPE>& p_element);
		TYPE& resolve(const TPoolToken<TYPE>& p_element);
		TPoolToken<TYPE> get_next_freentry();
		bool is_token_free(const TPoolToken<TYPE>& p_element);
	};

	template<class TYPE, class Allocator = HeapAllocator>
	struct OptionalPool
	{
		static_assert(std::is_base_of<IAllocator, Allocator>::value, "Allocator must implements IAllocator.");

		Pool<Optional<TYPE>, Allocator> pool;

		OptionalPool();
		void allocate(size_t p_initialSize, const Allocator& p_allocator = Allocator());
		void free();

		OptionalPool<TYPE, Allocator> clone();
		OptionalPool<TYPE, Allocator> move();

		size_t size();
		Optional<TYPE>& operator[](size_t i);
		Optional<TYPE>& operator[](const TPoolToken<Optional<TYPE>> i);
		Optional<TYPE>& resolve(const TPoolToken<Optional<TYPE>> i);
		TPoolToken<Optional<TYPE>> alloc_element(const TYPE& p_element);
		void release_element(const TPoolToken<Optional<TYPE>>& p_element);
	};
}

