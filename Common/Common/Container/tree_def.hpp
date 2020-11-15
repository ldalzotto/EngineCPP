#pragma once

#include "vector_def.hpp"
#include "pool_def.hpp"

struct NTreeNode
{
	size_t index;
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
	template<class ElementType>
	struct INTreeForEach
	{
		virtual void foreach(NTreeResolve<ElementType>& p_resolve) = 0;
	};

	com::Pool<ElementType, Allocator> Memory;
	com::Pool<NTreeNode, Allocator> Indices;

	void allocate(size_t p_initialSize, const Allocator& p_allocator = Allocator());
	void free();

	NTree<ElementType, Allocator> move();

	NTreeResolve<ElementType> resolve(com::PoolToken p_token);

	com::PoolToken push_root_value(const ElementType& p_value);
	com::PoolToken push_value(const com::PoolToken p_parent, const ElementType& p_value);
	com::PoolToken push_value(const ElementType& p_value);
	void remove(const com::PoolToken p_value);

	template<class NTreeForEach>
	void traverse(com::PoolToken& p_start, NTreeForEach& p_foreach);

	template<class NTreeMap, class TargetElementType>
	NTree<TargetElementType> map(NTreeMap& p_map);
};
