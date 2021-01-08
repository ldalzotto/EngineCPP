#pragma once

/*
template<class VectorForeachIfConditionFn, class Closure, class ElementType>
inline void vector_foreach_if(Vector<ElementType>* p_vector, Closure* p_closure)
{
	for (vector_loop(p_vector, i))
	{
		ElementType* l_element = vector_get(p_vector, i);
		if (VectorForeachIfConditionFn::barrier(p_closure, i, l_element))
		{
			VectorForeachIfConditionFn::for_each(p_closure, i, l_element);
		};
	}
}

template<class VectorEraseIfConditionFn, class VectorEraseIfConditionClosure, class ElementType>
inline void vector_erase_if(Vector<ElementType>* p_vector, VectorEraseIfConditionClosure* p_closure)
{
	for (vector_loop_reverse(p_vector, i))
	{
		if (VectorEraseIfConditionFn::evaluate(p_closure, i, vector_get(p_vector, i)))
		{
			vector_erase_element_at(p_vector, i);
		};
	}
};

*/

#define vector_erase_if_2_begin(VectorVariable, IteratorName, VectorElementVariableName) \
for (vector_loop_reverse((VectorVariable), IteratorName)) \
{ \
auto* VectorElementVariableName = vector_get((VectorVariable), i);

#define vector_erase_if_2_end(VectorVariable, IteratorName, IfConditionVariableName) \
if ((IfConditionVariableName))\
{\
	vector_erase_element_at((VectorVariable), IteratorName);\
};\
}


/*
template<class ForEachFn, class ForEachFnClosure, class ElementType>
inline void poolindexed_foreach_token(PoolIndexed<ElementType>* p_indexed_pool, ForEachFnClosure* p_closure)
{
	for (vector_loop((&p_indexed_pool->Indices), i))
	{
		ForEachFn::for_each(p_closure, i, vector_get(&p_indexed_pool->Indices, i));
	}
};

template<class ForEachFn, class ForEachFnClosure, class ElementType>
inline void poolindexed_foreach_token_1v(PoolIndexed<ElementType>* p_indexed_pool, ForEachFnClosure p_closure)
{
	poolindexed_foreach_token<ForEachFn, ForEachFnClosure, ElementType>(p_indexed_pool, &p_closure);
};
*/

#define poolindexed_foreach_token_2_begin(PoolIndexedVariable, IteratorName, TokenVariableName) \
for (vector_loop((&(PoolIndexedVariable)->Indices), IteratorName)) \
{ \
auto* TokenVariableName = vector_get(&(PoolIndexedVariable)->Indices, IteratorName);

#define poolindexed_foreach_token_2_end() \
}