#pragma once

template<class ElementType>
using PoolMemory_t = Vector<ElementType>;

template<class ElementType>
using PoolFreeBlocks_t = Vector<Token(ElementType)>;

/*
	A Pool is a non continous Vector where elements are acessed via Tokens.
	Generated Tokens are unique from it's source Pool.
	Even if pool memory is reallocate, generated Tokens are still valid.
	/!\ It is very unsafe to store raw pointer of an element. Because is Pool memory is reallocated, then the pointer is no longer valid.
*/
template<class ElementType>
struct Pool
{
	PoolMemory_t<ElementType> memory;
	PoolFreeBlocks_t<ElementType> free_blocks;
};

template<class ElementType>
inline Pool<ElementType> pool_build(const PoolMemory_t<ElementType> p_memory, const PoolFreeBlocks_t<ElementType> p_free_blocks)
{
	return Pool<ElementType>{p_memory, p_free_blocks};
};

template<class ElementType>
inline Pool<ElementType> pool_allocate(const size_t p_memory_capacity)
{
	return Pool<ElementType>{ vector_allocate<ElementType>(p_memory_capacity), vector_build(cast(Token(ElementType)*, NULL), 0) };
};

template<class ElementType>
inline void pool_free(Pool<ElementType>* p_pool)
{
	vector_free(&p_pool->memory);
	vector_free(&p_pool->free_blocks);
};

template<class ElementType>
inline size_t pool_get_size(const Pool<ElementType>* p_pool)
{
	return p_pool->memory.Size;
};

template<class ElementType>
inline size_t pool_get_capacity(const Pool<ElementType>* p_pool)
{
	return p_pool->memory.Span.Capacity;
};

template<class ElementType>
inline size_t pool_get_free_size(const Pool<ElementType>* p_pool)
{
	return p_pool->free_blocks.Size;
};

template<class ElementType>
inline ElementType* pool_get_memory(const Pool<ElementType>* p_pool)
{
	return p_pool->memory.Span.Memory;
};

template<class ElementType>
inline char pool_is_element_free(Pool<ElementType>* p_pool, const Token<ElementType>* p_token)
{
	for (vector_loop(&p_pool->free_blocks, i))
	{
		if (vector_get_rv(&p_pool->free_blocks, i).tok == p_token->tok)
		{
			return 1;
		};
	};

	return 0;
};

template<class ElementType>
inline char pool_is_element_free_1v(Pool<ElementType>* p_pool, const Token<ElementType> p_token)
{
	return pool_is_element_free(p_pool, &p_token);
};

template<class ElementType>
inline void _pool_element_free_check(Pool<ElementType>* p_pool, const Token<ElementType>* p_token)
{
#if CONTAINER_BOUND_TEST
	if (pool_is_element_free(p_pool, p_token))
	{
		abort();
	}
#endif
};

template<class ElementType>
inline void _pool_element_free_check_1v(Pool<ElementType>* p_pool, const Token<ElementType> p_token)
{
	_pool_element_free_check(p_pool, &p_token);
};

template<class ElementType>
inline void _pool_element_not_free_check(Pool<ElementType>* p_pool, const Token<ElementType>* p_token)
{
#if CONTAINER_BOUND_TEST
	if (pool_is_element_free(p_pool, p_token))
	{
		abort();
	}
#endif
};

template<class ElementType>
inline void _pool_element_not_free_check_1v(Pool<ElementType>* p_pool, const Token<ElementType> p_token)
{
	_pool_element_not_free_check(p_pool, &p_token);
};

template<class ElementType>
inline ElementType* pool_get(Pool<ElementType>* p_pool, const Token<ElementType>* p_token)
{
#if CONTAINER_BOUND_TEST
	_pool_element_free_check(p_pool, p_token);
#endif

	return vector_get(&p_pool->memory, p_token->tok);
};

template<class ElementType>
inline ElementType* pool_get_1v(Pool<ElementType>* p_pool, const Token<ElementType> p_token)
{
	return pool_get(p_pool, &p_token);
};

template<class ElementType>
inline ElementType pool_get_rv1v(Pool<ElementType>* p_pool, const Token<ElementType> p_token)
{
	return *pool_get(p_pool, &p_token);
};

template<class ElementType>
inline Token<ElementType> pool_alloc_element_empty(Pool<ElementType>* p_pool)
{
	if (!vector_empty(&p_pool->free_blocks))
	{
		Token(ElementType) l_availble_token = vector_get_rv(&p_pool->free_blocks, p_pool->free_blocks.Size - 1);
		vector_pop_back(&p_pool->free_blocks);
		return l_availble_token;
	}
	else
	{
		vector_push_back_element_empty(&p_pool->memory);
		return Token(ElementType) { p_pool->memory.Size - 1 };
	}
}

template<class ElementType>
inline Token<ElementType> pool_alloc_element(Pool<ElementType>* p_pool, const ElementType* p_element)
{
	if (!vector_empty(&p_pool->free_blocks))
	{
		Token(ElementType) l_availble_token = vector_get_rv(&p_pool->free_blocks, p_pool->free_blocks.Size - 1);
		vector_pop_back(&p_pool->free_blocks);
		*vector_get(&p_pool->memory, l_availble_token.tok) = *p_element;
		return l_availble_token;
	}
	else
	{
		vector_push_back_element(&p_pool->memory, p_element);
		return Token(ElementType) {p_pool->memory.Size - 1};
	}
};


template<class ElementType>
inline Token<ElementType> pool_alloc_element_1v(Pool<ElementType>* p_pool, const ElementType p_element)
{
	return pool_alloc_element(p_pool, &p_element);
}

template<class ElementType>
inline void pool_release_element(Pool<ElementType>* p_pool, const Token<ElementType>* p_token)
{
#if CONTAINER_BOUND_TEST
	_pool_element_not_free_check(p_pool, p_token);
#endif

	vector_push_back_element(&p_pool->free_blocks, p_token);
};

template<class ElementType>
inline void pool_release_element_1v(Pool<ElementType>* p_pool, const Token<ElementType> p_token)
{
	pool_release_element(p_pool, &p_token);
};

#define pool_loop(PoolVariable, Iteratorname) size_t Iteratorname = 0; Iteratorname < pool_get_size(PoolVariable); Iteratorname++