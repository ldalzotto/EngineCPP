#pragma once

#include "tree.hpp"

#include "pool.hpp"
#include "tree_def.hpp"


inline void NTreeNode::allocate_as_root(ChildsToken p_childs)
{
	this->parent = -1;
	this->index = 0;
	this->childs = p_childs;
};


inline void NTreeNode::allocate(const size_t p_current_index, const size_t p_parent_index, ChildsToken p_childs)
{
	this->parent = p_parent_index;
	this->index = p_current_index;
	this->childs = p_childs;
};

inline void NTreeNode::free()
{

}

inline bool NTreeNode::has_parent()
{
	return this->parent != -1;
};

template<class ElementType, class Allocator>
inline void tree_detach_from_tree(NTree<ElementType, Allocator>& p_tree, NTreeResolve<ElementType>& p_node)
{
	if (p_node.node->has_parent())
	{
		NTreeResolve<ElementType> l_parent = p_tree.resolve(p_node.node->parent);
		com::Vector<com::TPoolToken<NTreeNode>>& l_parent_childs = p_tree.get_childs(l_parent);
		for (size_t i = 0; i < l_parent_childs.Size; i++)
		{
			if (l_parent_childs[i].Index == p_node.node->index)
			{
				l_parent_childs.erase_at(i, 1);
				break;
			}
		}
	}
	p_node.node->parent = -1;
};

template<class ElementType, class Allocator>
inline void NTree<ElementType, Allocator>::allocate(size_t p_initialSize, const Allocator& p_allocator)
{
	this->Memory.allocate(p_initialSize, p_allocator);
	this->Indices.allocate(p_initialSize, p_allocator);
	this->Indices_childs.allocate(p_initialSize, p_allocator);
};

template<class ElementType, class Allocator>
inline NTree<ElementType, Allocator> NTree<ElementType, Allocator>::clone()
{
	NTree<ElementType, Allocator> l_return;
	l_return.Memory = this->Memory.clone();
	l_return.Indices = this->Indices.clone();
	l_return.Indices_childs = this->Indices_childs.clone();
	for (size_t i = 0; i < l_return.Indices_childs.size(); i++)
	{
		if (l_return.Indices_childs[i].hasValue)
		{
			l_return.Indices_childs[i].value = l_return.Indices_childs[i].value.clone();
		}
	}

	return l_return;
};


template<class ElementType, class Allocator>
inline void NTree<ElementType, Allocator>::free()
{
	this->Memory.free();
	this->Indices.free();
	for (size_t i = 0; i < this->Indices_childs.size(); i++)
	{
		auto& l_entry = this->Indices_childs[i];
		if (l_entry.hasValue)
		{
			l_entry.value.free();
		}
	}
	this->Indices.free();
};

template<class ElementType, class Allocator>
inline NTree<ElementType, Allocator> NTree<ElementType, Allocator>::move()
{
	NTree<ElementType, Allocator> l_target;
	l_target.Indices = this->Indices.move();
	l_target.Memory = this->Memory.move();
	l_target.Indices = this->Indices.move();
	return l_target;
};

template<class ElementType, class Allocator>
inline NTreeResolve<ElementType> NTree<ElementType, Allocator>::resolve(com::PoolToken p_token)
{
	return NTreeResolve<ElementType>(
		this->Memory.resolve(com::TPoolToken<ElementType>(p_token.Index)),
		this->Indices.resolve(com::TPoolToken<NTreeNode>(p_token.Index))
		);
};

template<class ElementType, class Allocator>
com::Vector<com::TPoolToken<NTreeNode>>& NTree<ElementType, Allocator>::get_childs(const NTreeResolve<ElementType>& p_node_resolve)
{
	auto& l_indives_childs = this->Indices_childs[p_node_resolve.node->childs];
	if (l_indives_childs.hasValue)
	{
		return l_indives_childs.value;
	}
	else
	{
		abort();
	}
};

template<class ElementType, class Allocator>
inline void tree_allocate_node(NTree<ElementType, Allocator>* p_tree, com::TPoolToken<NTreeNode> p_parent, const ElementType& p_value,
	com::TPoolToken<ElementType>* out_created_element, com::TPoolToken<NTreeNode>* out_created_index, NTreeNode::ChildsToken* out_created_childs)
{
	*out_created_element = p_tree->Memory.alloc_element(p_value);
	*out_created_childs = p_tree->Indices_childs.alloc_element(com::Vector<com::TPoolToken<NTreeNode>>());
	NTreeNode l_node;
	l_node.allocate(out_created_element->Index, p_parent.Index, *out_created_childs);
	*out_created_index = p_tree->Indices.alloc_element(l_node);
};

template<class ElementType, class Allocator>
inline void tree_allocate_node_as_root(NTree<ElementType, Allocator>* p_tree, const ElementType& p_value,
	com::TPoolToken<ElementType>* out_created_element, com::TPoolToken<NTreeNode>* out_created_index, NTreeNode::ChildsToken* out_created_childs)
{
	*out_created_element = p_tree->Memory.alloc_element(p_value);
	*out_created_childs = p_tree->Indices_childs.alloc_element(com::Vector<com::TPoolToken<NTreeNode>>());
	NTreeNode l_node;
	l_node.allocate_as_root(*out_created_childs);
	*out_created_index = p_tree->Indices.alloc_element(l_node);
};

template<class ElementType, class Allocator>
inline com::TPoolToken<ElementType> NTree<ElementType, Allocator>::push_root_value(const ElementType& p_value)
{
	if (this->Memory.size() == 0)
	{
		com::TPoolToken<ElementType> l_created_element;
		com::TPoolToken<NTreeNode> l_created_index;
		NTreeNode::ChildsToken l_created_childs;
		tree_allocate_node_as_root(this, p_value, &l_created_element, &l_created_index, &l_created_childs);
		return l_created_element;
	}
	else
	{
		return com::TPoolToken<ElementType>(-1);
	}
};

template<class ElementType, class Allocator>
inline com::TPoolToken<ElementType> NTree<ElementType, Allocator>::push_value(const com::TPoolToken<NTreeNode> p_parent, const ElementType& p_value)
{
	com::TPoolToken<ElementType> l_created_element; 
	com::TPoolToken<NTreeNode> l_created_index;
	NTreeNode::ChildsToken l_created_childs;
	tree_allocate_node(this, p_parent, p_value, &l_created_element, &l_created_index, &l_created_childs);

	//this->Indices.resolve()
	this->Indices_childs.resolve(this->Indices[p_parent].childs).value.push_back(l_created_index);
	return l_created_element;
};

template<class ElementType, class Allocator>
inline com::TPoolToken<ElementType> NTree<ElementType, Allocator>::push_value(const ElementType& p_value)
{
	com::TPoolToken<ElementType> l_created_element;
	com::TPoolToken<NTreeNode> l_created_index;
	NTreeNode::ChildsToken l_created_childs;
	tree_allocate_node(this, com::TPoolToken<NTreeNode>(-1), p_value, &l_created_element, &l_created_index, &l_created_childs);
	return l_created_element;
};

template<class ElementType, class Allocator>
template<class NTreeForEach>
inline void NTree<ElementType, Allocator>::remove(com::PoolToken p_value, NTreeForEach& p_foreach_childs)
{
	struct RemoveFoearch
	{
		NTree<ElementType, Allocator>* tree;
		NTreeForEach* parameter_foreach;

		com::Vector<NTreeResolve<ElementType>> involved_nodes;

		inline RemoveFoearch() { };

		inline void allocate(NTree<ElementType, Allocator>* p_tree, NTreeForEach& p_parameter_foreach)
		{
			this->tree = p_tree;
			this->parameter_foreach = &p_parameter_foreach;
		};

		inline void free()
		{
			this->involved_nodes.free();
		};

		inline void foreach(NTreeResolve<ElementType>& p_node)
		{
			this->involved_nodes.push_back(p_node);
			this->parameter_foreach->foreach(p_node);
		};
	};

	RemoveFoearch l_removetree_foreach;
	l_removetree_foreach.allocate(this, p_foreach_childs);
	{
		this->traverse<RemoveFoearch>(p_value, l_removetree_foreach);

		tree_detach_from_tree(*this, this->resolve(p_value));

		for (size_t i = 0; i < l_removetree_foreach.involved_nodes.Size; i++)
		{
			NTreeResolve<ElementType>& l_resolve = l_removetree_foreach.involved_nodes[i];
			l_resolve.node->free();
			this->Indices.release_element(l_resolve.node->index);
			this->Memory.release_element(l_resolve.node->index);
		}
	}
	l_removetree_foreach.free();
};

template<class ElementType, class Allocator, class NTreeForEach>
inline void traverse_resolved(NTree<ElementType, Allocator>* p_tree, NTreeResolve<ElementType>& p_start, NTreeForEach& p_foreach)
{
	com::Vector<com::TPoolToken<NTreeNode>>& l_start_childs = p_tree->get_childs(p_start);
	for (size_t l_child_index = 0; l_child_index < l_start_childs.Size; l_child_index++)
	{
		com::PoolToken l_treenode_token = l_start_childs[l_child_index];
		NTreeResolve<ElementType> l_child = p_tree->resolve(l_treenode_token);
		com::Vector<com::TPoolToken<NTreeNode>>& l_child_childs = p_tree->get_childs(l_child);
		if (l_child_childs.Size > 0)
		{
			traverse_resolved(p_tree, l_child, p_foreach);
		}
		p_foreach.foreach(l_child);
	}
	p_foreach.foreach(p_start);
};

template<class ElementType, class Allocator>
template<class NTreeForEach>
inline void NTree<ElementType, Allocator>::traverse(com::PoolToken& p_start, NTreeForEach& p_foreach)
{
	// static_assert(std::is_base_of<INTreeForEach<ElementType>, NTreeForEach>::value, "NTreeForEach must implements INTreeForEach.");

	NTreeResolve<ElementType> l_start = this->resolve(p_start);
	com::Vector<com::TPoolToken<NTreeNode>>& l_start_childs = this->get_childs(l_start);
	for (size_t l_child_index = 0; l_child_index < l_start_childs.Size; l_child_index++)
	{
		NTreeResolve<ElementType> l_child = this->resolve(l_start_childs[l_child_index]);
		traverse_resolved<ElementType, Allocator, NTreeForEach>(this, l_child, p_foreach);
	}
	p_foreach.foreach(l_start);
};