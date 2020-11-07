#pragma once

#include "vector_def.hpp"
#include "pool_def.hpp"

struct NTreeNode
{
	size_t parent;
	com::Vector<size_t> childs;

	inline NTreeNode(){}

	void allocate_as_root();
	void allocate();
	void allocate(const size_t p_parent_index, NTreeNode& p_parent, const size_t p_current_index);
	void free();

	bool has_parent();
};

template<class ElementType>
struct NTreeResolve
{
	ElementType* element = nullptr;
	NTreeNode* node = nullptr;

	NTreeResolve(){}

	inline NTreeResolve(ElementType& p_element, NTreeNode& p_node)
	{
		this->element = &p_element;
		this->node = &p_node;
	};
};

template<class ElementType, class Allocator = HeapAllocator>
struct NTree
{
	com::Pool<ElementType, Allocator> Memory;
	com::Pool<NTreeNode, Allocator> Indices;

	void allocate(size_t p_initialSize, const Allocator& p_allocator = Allocator());
	void free();

	NTreeResolve<ElementType> resolve(com::PoolToken<ElementType> p_token);
	NTreeResolve<ElementType> resolve(com::PoolToken<NTreeNode> p_token);

	com::PoolToken<ElementType> push_root_value(const ElementType& p_value);
	com::PoolToken<ElementType> push_value(const com::PoolToken<ElementType> p_parent, const ElementType& p_value);
	com::PoolToken<ElementType> push_value(const ElementType& p_value);
	void remove(const com::PoolToken<ElementType> p_value);
};

template<class ElementType, class NTreeType>
struct NTreeTraversalIterator
{
	NTreeType* ntree;

	com::PoolToken<NTreeNode> current_node;
	NTreeResolve<ElementType> current_resolve;
	com::Vector<size_t> childscounter_stack;

	void allocate(NTreeType* p_tree, const com::PoolToken<NTreeNode>& p_start);
	void free();
	bool move_next();
};