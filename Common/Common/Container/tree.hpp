#pragma once

#include "tree.hpp"

#include "pool.hpp"
#include "tree_def.hpp"


inline void NTreeNode::allocate_as_root()
{
	this->parent = -1;
	this->childs.allocate(1);
};

inline void NTreeNode::allocate()
{
	this->parent = -1;
	this->childs.allocate(1);
};


inline void NTreeNode::allocate(const size_t p_parent_index, NTreeNode& p_parent, const size_t p_current_index)
{
	this->parent = p_parent_index;
	this->childs.allocate(1);
	p_parent.childs.push_back(p_current_index);
};



inline void NTreeNode::free()
{
	this->childs.free();
}

inline bool NTreeNode::has_parent()
{
	return this->parent != -1;
};

template<class ElementType, class Allocator>
inline void NTree<ElementType, Allocator>::allocate(size_t p_initialSize, const Allocator& p_allocator)
{
	this->Memory.allocate(p_initialSize, p_allocator);
	this->Indices.allocate(p_initialSize, p_allocator);
};

template<class ElementType, class Allocator>
inline void NTree<ElementType, Allocator>::free()
{
	this->Memory.free();
	this->Indices.free();
};

template<class ElementType, class Allocator>
inline NTreeResolve<ElementType> NTree<ElementType, Allocator>::resolve(com::PoolToken<ElementType> p_token)
{
	return NTreeResolve<ElementType>(
		this->Memory.resolve(p_token),
		this->Indices.resolve(com::PoolToken<NTreeNode>(p_token.Index))
		);
};

template<class ElementType, class Allocator>
inline NTreeResolve<ElementType> NTree<ElementType, Allocator>::resolve(com::PoolToken<NTreeNode> p_token)
{
	return NTreeResolve<ElementType>(
		this->Memory.resolve(com::PoolToken<ElementType>(p_token.Index)),
		this->Indices.resolve(p_token)
		);
};

template<class ElementType, class Allocator>
inline com::PoolToken<ElementType> NTree<ElementType, Allocator>::push_root_value(const ElementType& p_value)
{
	if (this->Memory.size() == 0)
	{
		auto l_allocated_item = this->Memory.alloc_element(p_value);
		NTreeNode l_node;
		l_node.allocate_as_root();
		this->Indices.alloc_element(l_node);
		return l_allocated_item;
	}
	else
	{
		this->Memory[0] = p_value;
		this->Indices[0].free();
		this->Indices[0].allocate_as_root();
		return com::PoolToken<ElementType>(0);
	}
};

template<class ElementType, class Allocator>
inline com::PoolToken<ElementType> NTree<ElementType, Allocator>::push_value(const com::PoolToken<ElementType> p_parent, const ElementType& p_value)
{
	auto l_allcoated_item = this->Memory.alloc_element(p_value);
	NTreeNode l_node;
	l_node.allocate(p_parent.Index, this->Memory.resolve(p_parent), l_allcoated_item.Index);
	this->Indices.alloc_element(l_node);
	return l_allcoated_item;
};

template<class ElementType, class Allocator>
inline com::PoolToken<ElementType> NTree<ElementType, Allocator>::push_value(const ElementType& p_value)
{
	auto l_allcoated_item = this->Memory.alloc_element(p_value);
	NTreeNode l_node;
	l_node.allocate();
	this->Indices.alloc_element(l_node);
	return l_allcoated_item;
};

template<class ElementType, class Allocator>
inline void NTree<ElementType, Allocator>::remove(const com::PoolToken<ElementType> p_value)
{
};


template<class ElementType, class NTreeType>
void NTreeTraversalIterator<ElementType, NTreeType>::allocate(NTreeType* p_tree, const com::PoolToken<NTreeNode>& p_start)
{
	this->ntree = p_tree;
	this->current_node = p_start;
	this->current_resolve = this->ntree->resolve(this->current_node);
	this->childscounter_stack.allocate(1);
	this->childscounter_stack.push_back(-1);
};

template<class ElementType, class NTreeType>
void NTreeTraversalIterator<ElementType, NTreeType>::free()
{
	this->childscounter_stack.free();
};

//TODO
template<class ElementType, class NTreeType>
bool NTreeTraversalIterator<ElementType, NTreeType>::move_next()
{
	size_t& l_counter = this->childscounter_stack[this->childscounter_stack.Size - 1];
	l_counter += 1;

	if (this->current_resolve.node->childs.Size != 0 && (l_counter <= this->current_resolve.node->childs.Size - 1))
	{
		this->childscounter_stack.push_back(0);
		size_t l_child_index = this->current_resolve.node->childs[l_counter];
		this->current_node = com::PoolToken<NTreeNode>(l_child_index);
		this->current_resolve = this->ntree->resolve(this->current_node);
		return true;
	}
	else
	{
		this->childscounter_stack.erase_at(this->childscounter_stack.Size - 1);
		if (this->childscounter_stack.Size == 1)
		{
			size_t l_parent_index = this->current_resolve.node->parent;
			this->current_node = com::PoolToken<NTreeNode>(l_parent_index);
			this->current_resolve = this->ntree->resolve(this->current_node);
			return true;
		}
		else
		{
			return this->move_next();
		}
	}

	if (this->childscounter_stack.Size > 0)
	{
		return true;
	}

	return false;
};