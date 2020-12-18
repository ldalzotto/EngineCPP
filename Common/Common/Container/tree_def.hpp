#pragma once

#include "vector_def.hpp"
#include "pool_def.hpp"

struct NTreeNode
{
	typedef com::TPoolToken<com::Vector<com::TPoolToken<NTreeNode>>> ChildsToken;

	com::TPoolToken<NTreeNode> index;
	com::TPoolToken<NTreeNode> parent;
	ChildsToken  childs;

	inline NTreeNode(){}

	void allocate_as_root(ChildsToken p_childs);
	void allocate(const com::TPoolToken<NTreeNode> p_current_index, const com::TPoolToken<NTreeNode> p_parent_index, ChildsToken p_childs);
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
	com::Pool<com::Vector<com::TPoolToken<NTreeNode>>> Indices_childs;

	void allocate(size_t p_initialSize, const Allocator& p_allocator = Allocator());
	NTree<ElementType, Allocator> clone();
	void free();

	NTree<ElementType, Allocator> move();

	com::TPoolToken<NTreeNode> get_next_freenode();

	NTreeResolve<ElementType> resolve(com::TPoolToken<ElementType> p_token);
	NTreeResolve<ElementType> resolve(com::TPoolToken<NTreeNode> p_token);

	com::Vector<com::TPoolToken<NTreeNode>>& get_childs(const NTreeResolve<ElementType>& p_node_resolve);

	com::TPoolToken<ElementType> push_root_value(const ElementType& p_value);
	com::TPoolToken<ElementType> push_value(const com::TPoolToken<NTreeNode> p_parent, const ElementType& p_value);
	com::TPoolToken<ElementType> push_value(const ElementType& p_value);
	bool set_value_at_freenode(const com::TPoolToken<NTreeNode> p_node, const ElementType& p_value);

	template<class NTreeForEach>
	void remove(com::TPoolToken<NTreeNode>& p_value, NTreeForEach& p_foreach_childs);
	template<class NTreeForEach>
	void remove(com::TPoolToken<ElementType>& p_value, NTreeForEach& p_foreach_childs);

	template<class NTreeForEach>
	void traverse(com::TPoolToken<ElementType>& p_start, NTreeForEach& p_foreach);
	template<class NTreeForEach>
	void traverse(com::TPoolToken<NTreeNode>& p_start, NTreeForEach& p_foreach);
};
