#pragma once

#include "pool_def.hpp"

template<class TYPE>
struct DoubleLinkedListToken
{
	size_t index = -1;

	inline DoubleLinkedListToken(size_t p_index) { index = p_index; };
	inline bool isValid() { return this->index > -1; };
};

template<class TYPE>
struct DoubleLinkedListNode
{
	TYPE value;
	DoubleLinkedListToken<TYPE> before = DoubleLinkedListToken<TYPE>();
	DoubleLinkedListToken<TYPE> after = DoubleLinkedListToken<TYPE>();

	inline DoubleLinkedListNode(const TYPE& p_value, const DoubleLinkedListToken<TYPE>& p_before = DoubleLinkedListToken<TYPE>(),
		const DoubleLinkedListToken<TYPE>& p_after = DoubleLinkedListToken<TYPE>())
	{
		value = p_value;
		before = p_before;
		after = p_after;
	}
};

template<class TYPE, class Allocator = HeapAllocator>
struct DoubleLinkedList
{
	DoubleLinkedListToken<TYPE> Head = DoubleLinkedListToken<TYPE>();
	DoubleLinkedListToken<TYPE> Tail = DoubleLinkedListToken<TYPE>();

	com::Pool<DoubleLinkedListNode<TYPE>> Memory;
	DoubleLinkedListToken<TYPE> push_back(const TYPE& p_element);
	void erase(const DoubleLinkedListToken<TYPE>& p_token);
};