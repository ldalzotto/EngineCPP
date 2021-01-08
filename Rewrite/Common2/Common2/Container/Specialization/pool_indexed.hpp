#pragma once

template<class ElementType>
struct PoolIndexed
{
	Pool<ElementType> Pool;
	Vector<Token(ElementType)> Indices;
};

template<class ElementType>
inline PoolIndexed<ElementType> poolindexed_allocate_default()
{
	return PoolIndexed<ElementType>
	{
		pool_allocate<ElementType>(0),
		vector_allocate<Token(ElementType)>(0)
	};
};

template<class ElementType>
inline void poolindexed_free(PoolIndexed<ElementType>* p_indexed_pool)
{
	pool_free(&p_indexed_pool->Pool);
	vector_free(&p_indexed_pool->Indices);
};

template<class ElementType>
inline Token(ElementType) poolindexed_alloc_element(PoolIndexed<ElementType>* p_indexed_pool, const ElementType* p_element)
{
	Token(ElementType) l_token = pool_alloc_element(&p_indexed_pool->Pool, p_element);
	vector_push_back_element(&p_indexed_pool->Indices, &l_token);
	return l_token;
};

template<class ElementType>
inline void poolindexed_release_element(PoolIndexed<ElementType>* p_indexed_pool, const Token(ElementType)* p_element)
{
	pool_release_element(&p_indexed_pool->Pool, p_element);
	for (vector_loop(&p_indexed_pool->Indices, i))
	{
		if (vector_get(&p_indexed_pool->Indices, i)->tok == p_element->tok)
		{
			vector_erase_element_at(&p_indexed_pool->Indices, i);
			break;
		}
	};
};

template<class ElementType>
inline ElementType* poolindexed_get(PoolIndexed<ElementType>* p_indexed_pool, const Token(ElementType)* p_element)
{
	return pool_get(&p_indexed_pool->Pool, p_element);
};

