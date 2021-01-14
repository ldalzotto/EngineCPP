#pragma once

//TODO -> delete when common2 migration is complete
namespace v2
{
	struct NTreeNode
	{
		Token(NTreeNode) index;
		Token(NTreeNode) parent;
		PoolOfVectorToken<Token(NTreeNode)> childs;

		inline static NTreeNode build(const Token(NTreeNode) p_index, const Token(NTreeNode) p_parent, const PoolOfVectorToken<Token(NTreeNode)> p_childs)
		{
			return NTreeNode{ p_index, p_parent, p_childs };
		};

		inline static NTreeNode build_index_childs(const Token(NTreeNode) p_index, const  PoolOfVectorToken<Token(NTreeNode)> p_childs)
		{
			return NTreeNode{ p_index, token_build_default<NTreeNode>(), p_childs };
		};
	};

	using NTreeChildsToken = PoolOfVectorToken<Token(NTreeNode)>;

	/*
		A NTree is a hierarchy of objects with ( Parent 1 <----> N Child ) relation ship.
	*/
	template<class ElementType>
	struct NTree
	{
		Pool<ElementType> Memory;
		Pool<NTreeNode> Indices;
		PoolOfVector<Token(NTreeNode)> Indices_childs;

		struct Resolve
		{
			ElementType* Element;
			NTreeNode* Node;

			inline static Resolve build(ElementType* p_element, NTreeNode* p_node)
			{
				return Resolve{ p_element, p_node };
			};

			inline char has_parent() const
			{
				return this->Node->parent.tok != -1;
			};
		};

		inline static NTree<ElementType> allocate_default()
		{
			return NTree<ElementType>
			{
				Pool<ElementType>::allocate(0),
					Pool<NTreeNode>::allocate(0),
					VectorOfVector<Token(NTreeNode)>::allocate_default()
			};
		};

		inline void free()
		{
			this->Memory.free();
			this->Indices.free();
			this->Indices_childs.free();
		};


		inline Resolve get(const Token(ElementType) p_token)
		{
			return Resolve::build(
				&this->Memory.get(p_token),
				&this->Indices.get(token_cast_v(NTreeNode, p_token))
			);
		};

		inline Resolve get_from_node(const Token(NTreeNode) p_token)
		{
			return this->get(token_cast_v(ElementType, p_token));
		};

		inline ElementType& get_value(const Token(ElementType) p_token)
		{
			return this->Memory.get(p_token);
		};

		inline Slice<Token(NTreeNode)> get_childs(const NTreeChildsToken p_child_token)
		{
			VectorOfVector_Element<Token(NTreeNode)> l_childs = this->Indices_childs.get_vector(p_child_token);
			return Slice<Token(NTreeNode)>::build_memory_elementnb(l_childs.Memory.Begin, l_childs.Header.Size);
		};

		inline Token(ElementType) push_root_value(const ElementType& p_element)
		{
#if CONTAINER_BOUND_TEST
			assert_true(this->Memory.get_size() == 0);
#endif
			Token(ElementType) l_element;
			Token(NTreeNode) l_node;
			NTreeChildsToken l_childs;
			this->allocate_root_node(p_element, &l_element, &l_node, &l_childs);
			return l_element;
		};

		inline Token(ElementType) push_value(const ElementType& p_element, const Token(ElementType) p_parent)
		{
			Token(ElementType) l_element;
			Token(NTreeNode) l_node;
			NTreeChildsToken l_childs;
			this->allocate_node(token_cast_v(NTreeNode, p_parent), p_element, &l_element, &l_node, &l_childs);
			return l_element;
		};

		template<class ForEachFunc>
		inline void traverse2(const Token(NTreeNode) p_current_node)
		{
			Resolve l_node = this->get(token_cast_p(ElementType, p_current_node));
			ForEachFunc::foreach(l_node);
			Slice<Token(NTreeNode)> l_childs = this->get_childs(&l_node.Node->childs);
			for (size_t i = 0; i < l_childs.Size; i++)
			{
				this->traverse2<ForEachFunc>(*l_childs.get(i));
			};
		};

		template<class ForEachObj>
		inline void traverse2_stateful(const Token(NTreeNode) p_current_node, ForEachObj& p_foreach_obj)
		{
			Resolve l_node = this->get(token_cast_v(ElementType, p_current_node));
			p_foreach_obj.foreach(l_node);
			Slice<Token(NTreeNode)> l_childs = this->get_childs(l_node.Node->childs);
			for (size_t i = 0; i < l_childs.Size; i++)
			{
				this->traverse2_stateful<ForEachObj>(l_childs.get(i), p_foreach_obj);
			};
		}

		inline void remove_node(const Token(NTreeNode) p_node)
		{
			Vector<Resolve> l_involved_nodes = Vector<Resolve>::allocate(0);

			struct RemoveForEach {
				Vector<Resolve>* involved_nodes;
				inline void foreach(const Resolve& p_node) { this->involved_nodes->push_back_element(p_node); }
			};
			this->traverse2_stateful(p_node, RemoveForEach{ &l_involved_nodes });

			this->detach_from_tree(l_involved_nodes.get(0));
			for (vector_loop(&l_involved_nodes, i))
			{
				Resolve& l_removed_node = l_involved_nodes.get(i);
				this->Memory.release_element(token_cast_v(ElementType, l_removed_node.Node->index));
				this->Indices.release_element(l_removed_node.Node->index);
				this->Indices_childs.release_vector(l_removed_node.Node->childs);
			}

			l_involved_nodes.free();
		};

	private:

		inline void allocate_node(const Token(NTreeNode) p_parent, const ElementType& p_element, Token(ElementType)* out_created_element, Token(NTreeNode)* out_created_index, NTreeChildsToken* out_created_childs)
		{
			*out_created_element = this->Memory.alloc_element(p_element);
			*out_created_childs = this->Indices_childs.alloc_vector();
			*out_created_index = this->Indices.alloc_element(NTreeNode::build(token_cast_v(NTreeNode, *out_created_element), p_parent, *out_created_childs));

			this->Indices_childs.element_push_back_element(this->get_from_node(p_parent).Node->childs, *out_created_index);
		};

		inline void allocate_root_node(const ElementType& p_element, Token(ElementType)* out_created_element, Token(NTreeNode)* out_created_index, NTreeChildsToken* out_created_childs)
		{
			*out_created_element = this->Memory.alloc_element(p_element);
			*out_created_childs = this->Indices_childs.alloc_vector();
			*out_created_index = this->Indices.alloc_element(NTreeNode::build_index_childs(token_cast_v(NTreeNode, *out_created_element), *out_created_childs));
		};

		inline void detach_from_tree(Resolve& p_node)
		{
			if (p_node.has_parent())
			{
				Resolve l_parent = this->get_from_node(p_node.Node->parent);
				VectorOfVector_Element<Token(NTreeNode)> l_parent_childs = this->Indices_childs.get_vector(l_parent.Node->childs);
				for (loop(i, 0, l_parent_childs.Header.Size))
				{
					if (l_parent_childs.get(i).tok == p_node.Node->index.tok)
					{
						this->Indices_childs.element_erase_element_at_always(l_parent.Node->childs, i);
						break;
					}
				}
			}
			p_node.Node->parent = token_build_default<NTreeNode>();
		};

	};

}




#define tree_traverse2_stateful_begin(ElementType, StateParameters, ForEachObjStructName) \
struct ForEachObjStructName\
{\
	StateParameters; \
	inline void foreach(const NTree<##ElementType##>::Resolve& p_node) \
	{

#define tree_traverse2_stateful_end(ElementType, TreeVariable, StartToken, StateParameterValues, ForEachObjStructName) \
	};\
};\
(TreeVariable)->traverse2_stateful((StartToken), ForEachObjStructName{ StateParameterValues });

#define tree_traverse2_begin(ElementType, ForEachObjStructName) \
struct ForEachObjStructName\
{\
	inline static void foreach(const NTree<##ElementType##>::Resolve& p_node) \
	{

#define tree_traverse2_end(ElementType, TreeVariable, StartToken, ForEachObjStructName) \
	};\
};\
(TreeVariable)->traverse2<##ForEachObjStructName##>((StartToken));