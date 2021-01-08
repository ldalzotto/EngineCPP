#pragma once

#define vector_erase_if_2_begin(VectorVariable, IteratorName, VectorElementVariableName) \
for (vector_loop_reverse((VectorVariable), IteratorName)) \
{ \
auto* VectorElementVariableName = (VectorVariable)->get(i);

#define vector_erase_if_2_end(VectorVariable, IteratorName, IfConditionVariableName) \
if ((IfConditionVariableName))\
{\
	(VectorVariable)->erase_element_at(IteratorName);\
};\
}


#define poolindexed_foreach_token_2_begin(PoolIndexedVariable, IteratorName, TokenVariableName) \
for (vector_loop((&(PoolIndexedVariable)->Indices), IteratorName)) \
{ \
auto* TokenVariableName = (PoolIndexedVariable)->Indices.get(IteratorName);

#define poolindexed_foreach_token_2_end() \
}