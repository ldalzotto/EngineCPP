#pragma once

#include "pool_def.hpp"

namespace com
{
	template <class TYPE, class Allocator>
	inline void Pool<TYPE, Allocator>::allocate(size_t p_initialSize, const Allocator &p_allocator)
	{
		this->Memory.allocate(p_initialSize, p_allocator);
		this->FreeBlocks.allocate(0, p_allocator);
	}

	template <class TYPE, class Allocator>
	inline void Pool<TYPE, Allocator>::free()
	{
		this->Memory.free();
		this->FreeBlocks.free();
	}

	template <class TYPE, class Allocator>
	inline Pool<TYPE, Allocator> Pool<TYPE, Allocator>::clone()
	{
		Pool<TYPE, Allocator> l_target;
		l_target.FreeBlocks = this->FreeBlocks.clone();
		l_target.Memory = this->Memory.clone();
		return l_target;
	};
	template <class TYPE, class Allocator>
	inline Pool<TYPE, Allocator> Pool<TYPE, Allocator>::move()
	{
		Pool<TYPE, Allocator> l_target;
		l_target.FreeBlocks = this->FreeBlocks.move();
		l_target.Memory = this->Memory.move();
		return l_target;
	};

	template <class TYPE, class Allocator>
	inline Vector<size_t, HeapAllocator> Pool<TYPE, Allocator>::clone_freeblocks()
	{
		return this->FreeBlocks.clone();
	};

	template<class TYPE, class Allocator>
	inline size_t Pool<TYPE, Allocator>::size()
	{
		return this->Memory.Size;
	};

	template<class TYPE, class Allocator>
	inline TYPE& Pool<TYPE, Allocator>::operator[](size_t i)
	{
		return this->Memory.Memory[i];
	}

	template<class TYPE, class Allocator>
	inline TYPE& Pool<TYPE, Allocator>::operator[](const PoolToken<TYPE> i)
	{
		return this->Memory.Memory[i.Index];
	}
	
	template<class TYPE, class Allocator>
	inline PoolToken<TYPE> Pool<TYPE, Allocator>::alloc_element(const TYPE& p_element)
	{
		if (this->FreeBlocks.Size > 0)
		{
			size_t l_availableIndex = this->FreeBlocks.Memory[this->FreeBlocks.Size - 1];
			this->Memory.Memory[l_availableIndex] = p_element;
			return PoolToken<TYPE>(l_availableIndex);
		}
		else
		{
			this->Memory.push_back(p_element);
			return PoolToken<TYPE>(this->Memory.Size - 1);
		}
	}

	template<class TYPE, class Allocator>
	inline void Pool<TYPE, Allocator>::release_element(const PoolToken<TYPE>& p_element)
	{
		this->FreeBlocks.push_back(p_element.Index);
	}

	template<class TYPE, class Allocator>
	inline TYPE& Pool<TYPE, Allocator>::resolve(const PoolToken<TYPE>& p_element)
	{
		return this->Memory.Memory[p_element.Index];
	};


	template<class TYPE, class Allocator>
	inline OptionalPool<TYPE, Allocator>::OptionalPool()
	{
	}


	template <class TYPE, class Allocator>
	inline void OptionalPool<TYPE, Allocator>::allocate(size_t p_initialSize, const Allocator& p_allocator)
	{
		this->pool.allocate(p_initialSize, p_allocator);
	}

	template <class TYPE, class Allocator>
	inline void OptionalPool<TYPE, Allocator>::free()
	{
		this->pool.free();
	}

	template<class TYPE, class Allocator>
	inline size_t OptionalPool<TYPE, Allocator>::size()
	{
		return this->pool.size();
	};

	template<class TYPE, class Allocator>
	inline Optional<TYPE>& OptionalPool<TYPE, Allocator>::operator[](size_t i)
	{
		return this->pool[i];
	};

	template<class TYPE, class Allocator>
	inline Optional<TYPE>& OptionalPool<TYPE, Allocator>::operator[](const PoolToken<Optional<TYPE>> i)
	{
		return this->pool[i];
	};

	template<class TYPE, class Allocator>
	inline PoolToken<Optional<TYPE>> OptionalPool<TYPE, Allocator>::alloc_element(const TYPE& p_element)
	{
		Optional<TYPE> l_element = Optional<TYPE>(p_element);
		return this->pool.alloc_element(l_element);
	}

	template<class TYPE, class Allocator>
	inline void OptionalPool<TYPE, Allocator>::release_element(const PoolToken<Optional<TYPE>>& p_element)
	{
		this->pool.release_element(p_element);
		this->pool[p_element].clear();
	}
}