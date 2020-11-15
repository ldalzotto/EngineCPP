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
	this->index = p_current_index;
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
inline NTree<ElementType, Allocator> NTree<ElementType, Allocator>::move()
{
	NTree<ElementType, Allocator> l_target;
	l_target.Indices = this->Indices.move();
	l_target.Memory = this->Memory.move();
	return l_target;
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
inline com::PoolToken<ElementType> NTree<ElementType, Allocator>::push_value(const com::PoolToken<NTreeNode> p_parent, const ElementType& p_value)
{
	auto l_allcoated_item = this->Memory.alloc_element(p_value);
	NTreeNode l_node;
	l_node.allocate(p_parent.Index, this->Indices.resolve(p_parent), l_allcoated_item.Index);
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

template<class ElementType, class Allocator, class NTreeForEach>
inline void traverse_resolved(NTree<ElementType, Allocator>* p_tree, NTreeResolve<ElementType>& p_start, NTreeForEach& p_foreach)
{
	for (size_t l_child_index = 0; l_child_index < p_start.node->childs.Size; l_child_index++)
	{
		com::PoolToken<NTreeNode> l_treenode_token = com::PoolToken<NTreeNode>(p_start.node->childs[l_child_index]);
		NTreeResolve<ElementType> l_child = p_tree->resolve(l_treenode_token);
		if (l_child.node->childs.Size > 0)
		{
			traverse_resolved(p_tree, l_child, p_foreach);
		}
		p_foreach.foreach(l_child);
	}
	p_foreach.foreach(p_start);
};

template<class ElementType, class Allocator>
template<class NTreeForEach>
inline void NTree<ElementType, Allocator>::traverse(com::PoolToken<NTreeNode>& p_start, NTreeForEach& p_foreach)
{
	// static_assert(std::is_base_of<INTreeForEach<ElementType>, NTreeForEach>::value, "NTreeForEach must implements INTreeForEach.");

	NTreeResolve<ElementType> l_start = this->resolve(p_start);
	for (size_t l_child_index = 0; l_child_index < l_start.node->childs.Size; l_child_index++)
	{
		NTreeResolve<ElementType> l_child = this->resolve(com::PoolToken<NTreeNode>(l_start.node->childs[l_child_index]));
		traverse_resolved<ElementType, Allocator, NTreeForEach>(this, l_child, p_foreach);
	}
	p_foreach.foreach(l_start);
};


template<class ElementType, class Allocator>
template<class NTreeMap, class TargetElementType>
NTree<TargetElementType> NTree<ElementType, Allocator>::map(NTreeMap& p_map)
{
	struct MapTraverseFn
	{
		NTreeMap* map;
		NTree<TargetElementType>* target;

		inline MapTraverseFn() {};

		inline MapTraverseFn(NTreeMap& p_map, NTree<TargetElementType>& p_target)
		{
			this->map = &p_map;
			this->target = &p_target;
		};

		inline void foreach(NTreeResolve<ElementType>& p_node, com::PoolToken<NTreeNode>& p_node_index)
		{
			this->target->Memory[com::PoolToken<TargetElementType>(p_node_index.Index)] = this->map->map(p_node);
		};
	};



	NTree<TargetElementType> l_target;
	l_target.Indices = this->Indices.clone();
	l_target.Memory.FreeBlocks = this->Memory.clone_freeblocks();

	this->traverse(com::PoolToken<NTreeNode>(0), MapTraverseFn(p_map, l_target));

	return l_target;
};