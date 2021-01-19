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
			return NTreeNode{ p_index, tk_bd(NTreeNode), p_childs };
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

			inline int8 has_parent() const
			{
				return tk_v(this->Node->parent) != -1;
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
				&this->Indices.get(tk_bf(NTreeNode, p_token))
			);
		};

		inline Resolve get_from_node(const Token(NTreeNode) p_token)
		{
			return this->get(tk_bf(ElementType, p_token));
		};

		inline ElementType& get_value(const Token(ElementType) p_token)
		{
			return this->Memory.get(p_token);
		};

		inline Slice<Token(NTreeNode)> get_childs(const NTreeChildsToken p_child_token)
		{
			return this->Indices_childs.get_vector(p_child_token);
		};

		inline Slice<Token(NTreeNode)> get_childs_from_node(const Token(NTreeNode) p_node)
		{
			return this->get_childs(this->get_from_node(p_node).Node->childs);
		};

		inline int8 add_child(const Resolve& p_parent, const  Resolve& p_new_child)
		{
			if (!tk_eq(p_parent.Node->index, p_new_child.Node->index))
			{
				this->detach_from_tree(p_new_child);

				p_new_child.Node->parent = p_parent.Node->index;
				this->Indices_childs.element_push_back_element(p_parent.Node->childs, p_new_child.Node->index);

				return 1;
			}
			return 0;
		};


		inline int8 add_child(const Token(ElementType) p_parent, const Token(ElementType) p_new_child)
		{
			Resolve l_new_child_value = this->get(p_new_child);
			return this->add_child(this->get(p_parent), l_new_child_value);
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
			this->allocate_node(tk_bf(NTreeNode, p_parent), p_element, &l_element, &l_node, &l_childs);
			return l_element;
		};

		template<class ForEachFunc>
		inline void traverse2(const Token(NTreeNode) p_current_node)
		{
			Resolve l_node = this->get(tk_bf(ElementType, p_current_node));
			ForEachFunc::foreach(l_node);
			Slice<Token(NTreeNode)> l_childs = this->get_childs(l_node.Node->childs);
			for (uimax i = 0; i < l_childs.Size; i++)
			{
				this->traverse2<ForEachFunc>(l_childs.get(i));
			};
		};

		template<class ForEachObj>
		inline void traverse2_stateful(const Token(NTreeNode) p_current_node, ForEachObj& p_foreach_obj)
		{
			Resolve l_node = this->get(tk_bf(ElementType, p_current_node));
			p_foreach_obj.foreach(l_node);
			Slice<Token(NTreeNode)> l_childs = this->get_childs(l_node.Node->childs);
			for (uimax i = 0; i < l_childs.Size; i++)
			{
				this->traverse2_stateful<ForEachObj>(l_childs.get(i), p_foreach_obj);
			};
		}

		inline void get_nodes(const Token(NTreeNode) p_start_node_included, Vector<Resolve>* in_out_nodes)
		{
			struct RemoveForEach {
				Vector<Resolve>* involved_nodes;
				inline void foreach(const Resolve& p_node) { this->involved_nodes->push_back_element(p_node); }
			};
			RemoveForEach l_remove_foreach = RemoveForEach {in_out_nodes};
			this->traverse2_stateful(p_start_node_included, l_remove_foreach);
		};

		inline void remove_node_recursively(const Token(NTreeNode) p_node)
		{
			Vector<Resolve> l_involved_nodes = Vector<Resolve>::allocate(0);
			this->get_nodes(p_node, &l_involved_nodes);

			Slice<Resolve> l_involved_nodes_slice = l_involved_nodes.to_slice();
			this->remove_nodes_and_detach(l_involved_nodes_slice);

			l_involved_nodes.free();
		};

		inline void remove_nodes(const Slice<Resolve>& p_removed_nodes)
		{
			for (loop(i, 0, p_removed_nodes.Size))
			{
				const Resolve& l_removed_node = p_removed_nodes.get(i);
				this->Memory.release_element(tk_bf(ElementType, l_removed_node.Node->index));
				this->Indices.release_element(l_removed_node.Node->index);
				this->Indices_childs.release_vector(l_removed_node.Node->childs);
			}
		};

		inline void remove_nodes_and_detach(Slice<Resolve>& p_removed_nodes)
		{
			this->detach_from_tree(p_removed_nodes.get(0));
			this->remove_nodes(p_removed_nodes);
		};

		inline void make_node_orphan(Resolve& p_node)
		{
			this->detach_from_tree(p_node);
		};

	private:

		inline void allocate_node(const Token(NTreeNode) p_parent, const ElementType& p_element, Token(ElementType)* out_created_element, Token(NTreeNode)* out_created_index, NTreeChildsToken* out_created_childs)
		{
			*out_created_element = this->Memory.alloc_element(p_element);
			*out_created_childs = this->Indices_childs.alloc_vector();
			*out_created_index = this->Indices.alloc_element(NTreeNode::build(tk_bf(NTreeNode, *out_created_element), p_parent, *out_created_childs));

			this->Indices_childs.element_push_back_element(this->get_from_node(p_parent).Node->childs, *out_created_index);
		};

		inline void allocate_root_node(const ElementType& p_element, Token(ElementType)* out_created_element, Token(NTreeNode)* out_created_index, NTreeChildsToken* out_created_childs)
		{
			*out_created_element = this->Memory.alloc_element(p_element);
			*out_created_childs = this->Indices_childs.alloc_vector();
			*out_created_index = this->Indices.alloc_element(NTreeNode::build_index_childs(tk_bf(NTreeNode, *out_created_element), *out_created_childs));
		};

		inline void detach_from_tree(const Resolve& p_node)
		{
			if (p_node.has_parent())
			{
				Resolve l_parent = this->get_from_node(p_node.Node->parent);
				Slice<Token(NTreeNode)> l_parent_childs = this->Indices_childs.get_vector(l_parent.Node->childs);
				for (loop(i, 0, l_parent_childs.Size))
				{
					if (tk_eq(l_parent_childs.get(i), p_node.Node->index))
					{
						this->Indices_childs.element_erase_element_at_always(l_parent.Node->childs, i);
						break;
					}
				}
			}
			p_node.Node->parent = tk_bd(NTreeNode);
		};

	};

}




#define tree_traverse2_stateful_begin(ElementType, StateParameters, ForEachObjStructName) \
struct ForEachObjStructName\
{\
	StateParameters; \
	inline void foreach(const NTree<ElementType>::Resolve& p_node) \
	{

#define tree_traverse2_stateful_end(ElementType, TreeVariable, StartToken, StateParameterValues, ForEachObjStructName) \
	};\
};\
auto l_foreach_obj_##ForEachObjStructName = ForEachObjStructName{ StateParameterValues }; \
(TreeVariable)->traverse2_stateful((StartToken), l_foreach_obj_##ForEachObjStructName);

#define tree_traverse2_begin(ElementType, ForEachObjStructName) \
struct ForEachObjStructName\
{\
	inline static void foreach(const NTree<ElementType>::Resolve& p_node) \
	{

#define tree_traverse2_end(ElementType, TreeVariable, StartToken, ForEachObjStructName) \
	};\
};\
(TreeVariable)->traverse2<ForEachObjStructName>((StartToken));
