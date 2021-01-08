#pragma once

template<class ElementType>
using PoolOfVectorMemory_t = VectorOfVector<ElementType>;

template<class ElementType>
using PoolOfVectorToken = Token(VectorOfVector_Element<ElementType>);

template<class ElementType>
using PoolOfVectorFreeBlocks_t = Vector<PoolOfVectorToken<ElementType>>;

/*
	A PoolOfVector is a wrapped VectorOfVector with pool allocation logic.
	Any operation on nested vectors must be called with the (poolofvector_element_*) functions
*/
template<class ElementType>
struct PoolOfVector
{
	PoolOfVectorMemory_t<ElementType> Memory;
	PoolOfVectorFreeBlocks_t<ElementType> FreeBlocks;
};

template<class ElementType>
inline PoolOfVector<ElementType> poolofvector_allocate_default()
{
	return PoolOfVector<ElementType>
	{
		vectorofvector_allocate_default<ElementType>(),
		vector_allocate<PoolOfVectorToken<ElementType>>(0)
	};
};

template<class ElementType>
inline void poolofvector_free(PoolOfVector<ElementType>* p_poolofvector)
{
	vectorofvector_free(&p_poolofvector->Memory);
	vector_free(&p_poolofvector->FreeBlocks);
};


template<class ElementType>
inline char poolofvector_is_token_free(PoolOfVector<ElementType>* p_poolofvector, const PoolOfVectorToken<ElementType>* p_token)
{
	for (vector_loop(&p_poolofvector->FreeBlocks, i))
	{
		if (vector_get(&p_poolofvector->FreeBlocks, i)->tok == p_token->tok)
		{
			return 1;
		}
	}
	return 0;
};

template<class ElementType>
inline void _poolofvector_token_not_free_check(PoolOfVector<ElementType>* p_poolofvector, const PoolOfVectorToken<ElementType>* p_token)
{
#if CONTAINER_BOUND_TEST
	if (poolofvector_is_token_free(p_poolofvector, p_token))
	{
		abort();
	}
#endif
};

template<class ElementType>
inline PoolOfVectorToken<ElementType> poolofvector_alloc_vector_with_values(PoolOfVector<ElementType>* p_poolofvector, const Slice<ElementType>* p_initial_elements)
{
	if (!vector_empty(&p_poolofvector->FreeBlocks))
	{
		PoolOfVectorToken<ElementType> l_token = vector_get_rv(&p_poolofvector->FreeBlocks, p_poolofvector->FreeBlocks.Size - 1);
		vector_pop_back(&p_poolofvector->FreeBlocks);
		vectorofvector_element_push_back_array(&p_poolofvector->Memory, l_token.tok, p_initial_elements);
		return l_token;
	}
	else
	{
		vectorofvector_push_back_element(&p_poolofvector->Memory, p_initial_elements);
		return PoolOfVectorToken<ElementType>{ varyingvector_get_size(&p_poolofvector->Memory.varying_vector) - 1 };
	}
};

template<class ElementType>
inline PoolOfVectorToken<ElementType> poolofvector_alloc_vector(PoolOfVector<ElementType>* p_poolofvector)
{
	if (!vector_empty(&p_poolofvector->FreeBlocks))
	{
		PoolOfVectorToken<ElementType> l_token = vector_get_rv(&p_poolofvector->FreeBlocks, p_poolofvector->FreeBlocks.Size - 1);
		vector_pop_back(&p_poolofvector->FreeBlocks);
		return l_token;
	}
	else
	{
		vectorofvector_push_back(&p_poolofvector->Memory);
		return PoolOfVectorToken<ElementType>{ varyingvector_get_size(&p_poolofvector->Memory.varying_vector) - 1 };
	}
};

template<class ElementType>
inline void poolofvector_release_vector(PoolOfVector<ElementType>* p_poolofvector, const PoolOfVectorToken<ElementType>* p_token)
{
#if CONTAINER_BOUND_TEST
	_poolofvector_token_not_free_check(p_poolofvector, p_token);
#endif

	vectorofvector_element_clear(&p_poolofvector->Memory, p_token->tok);
	vector_push_back_element(&p_poolofvector->FreeBlocks, p_token);
};

template<class ElementType>
inline VectorOfVector_Element<ElementType> poolofvector_get_vector(PoolOfVector<ElementType>* p_poolofvector, const PoolOfVectorToken<ElementType>* p_token)
{
#if CONTAINER_BOUND_TEST
	_poolofvector_token_not_free_check(p_poolofvector, p_token);
#endif

	return vectorofvector_get(&p_poolofvector->Memory, p_token->tok);
};

template<class ElementType>
inline VectorOfVector_Element<ElementType> poolofvector_get_vector_1v(PoolOfVector<ElementType>* p_poolofvector, const PoolOfVectorToken<ElementType> p_token)
{
	return poolofvector_get_vector(p_poolofvector, &p_token);
};

template<class ElementType>
inline void poolofvector_element_push_back_element(PoolOfVector<ElementType>* p_poolofvector, const PoolOfVectorToken<ElementType>* p_token, const ElementType* p_element)
{
#if CONTAINER_BOUND_TEST
	_poolofvector_token_not_free_check(p_poolofvector, p_token);
#endif

	vectorofvector_element_push_back_element(&p_poolofvector->Memory, p_token->tok, p_element);
};

template<class ElementType>
inline void poolofvector_element_push_back_element_2v(PoolOfVector<ElementType>* p_poolofvector, const PoolOfVectorToken<ElementType>* p_token, const ElementType p_element)
{
	poolofvector_element_push_back_element(p_poolofvector, p_token, &p_element);
};