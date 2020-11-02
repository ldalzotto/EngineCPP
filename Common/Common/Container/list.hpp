#pragma once

#include "list_def.hpp"

template<class TYPE, class Allocator>
inline DoubleLinkedListToken<TYPE> DoubleLinkedList<TYPE, Allocator>::push_back(const TYPE& p_element)
{
	DoubleLinkedListNode<TYPE> l_node = DoubleLinkedListNode<TYPE>(p_element);
	com::PoolToken<DoubleLinkedListNode<TYPE>>& l_pool_token = this->Memory.alloc_element(l_node);

	if (this->Tail.isValid())
	{
		this->Memory[this->Tail.index].after = DoubleLinkedListToken<TYPE>(l_pool_token.Index);
		this->Memory[l_pool_token.Index].before = DoubleLinkedListToken<TYPE>(this->Tail.index);
		this->Tail = DoubleLinkedListToken<TYPE>(l_pool_token.Index);
		return this->Tail;
	}
	else
	{
		this->Head = DoubleLinkedListToken<TYPE>(l_pool_token.Index);
		this->Tail = this->Head;
		return this->Head;
	}
}

template<class TYPE, class Allocator>
inline void DoubleLinkedList<TYPE, Allocator>::erase(const DoubleLinkedListToken<TYPE>& p_token)
{
	DoubleLinkedListNode<TYPE>& l_node = this->Memory[p_token.index];
	if (l_node.before.isValid())
	{
		if (l_node.after.isValid())
		{
			this->Memory[l_node.before]
		}
		else
		{
			this->Memory[l_node.before].after = DoubleLinkedListNode<TYPE>();
			this->Tail = DoubleLinkedListToken<TYPE>(l_node.before);
		}
	}
	else if (l_node.after.isValid())
	{
		this->Memory[l_node.after].before = DoubleLinkedListNode<TYPE>();
		this->Head = DoubleLinkedListToken<TYPE>(l_node.after);
	}

	this->Memory.release_element(com::PoolToken<TYPE>(p_token.index));
}
