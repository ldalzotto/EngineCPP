#pragma once

//TODO -> delete when common2 migration is complete
namespace v2
{
	struct NTreeNode
	{
		Token(NTreeNode) index;
		Token(NTreeNode) parent;
		PoolOfVectorToken<Token(NTreeNode)> childs;

		inline static NTreeNode build(const Token(NTreeNode)* p_index, const Token(NTreeNode)* p_parent, const PoolOfVectorToken<Token(NTreeNode)>* p_childs)
		{
			return NTreeNode{ *p_index, *p_parent, *p_childs };
		};

		inline static NTreeNode build_index_childs(const Token(NTreeNode)* p_index, const  PoolOfVectorToken<Token(NTreeNode)>* p_childs)
		{
			return NTreeNode{ *p_index, token_build_default<NTreeNode>(), *p_childs };
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

			inline char has_parent()
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


		inline Resolve get(const Token(ElementType)* p_token)
		{
			return Resolve::build(
				this->Memory.get(p_token),
				this->Indices.get(cast(Token(NTreeNode)*, p_token))
			);
		};

		inline Resolve get_from_node(const Token(NTreeNode)* p_token)
		{
			return this->get(cast(Token(ElementType)*, p_token));
		};

		inline ElementType* get_value(const Token(ElementType)* p_token)
		{
			return this->Memory.get(p_token);
		};

		inline Slice<Token(NTreeNode)> get_childs(const NTreeChildsToken* p_child_token)
		{
			VectorOfVector_Element<Token(NTreeNode)> l_childs = this->Indices_childs.get_vector(p_child_token);
			return Slice<Token(NTreeNode)>::build_memory_elementnb(l_childs.Memory.Begin, l_childs.Header.Size);
		};

		inline Token(ElementType) push_root_value(const ElementType* p_element)
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

		inline Token(ElementType) push_root_value(const ElementType p_element)
		{
			return push_root_value(&p_element);
		};

		inline Token(ElementType) push_value(const ElementType* p_element, const Token(ElementType)* p_parent)
		{
			Token(ElementType) l_element;
			Token(NTreeNode) l_node;
			NTreeChildsToken l_childs;
			this->allocate_node(cast(Token(NTreeNode)*, p_parent), p_element, &l_element, &l_node, &l_childs);
			return l_element;
		};

		inline Token(ElementType) push_value(const ElementType p_element, const Token(ElementType)* p_parent)
		{
			return this->push_value(&p_element, p_parent);
		};

		inline Token(ElementType) push_value(const ElementType p_element, const Token(ElementType) p_parent)
		{
			return this->push_value(&p_element, &p_parent);
		};

		struct Traverse
		{
			constexpr static int STACK_SIZE = 100;

			NTree<ElementType>* tree;
			Token(NTreeNode) start_node;
			Resolve node_stack[STACK_SIZE];
			Slice<Token(NTreeNode)> node_childs[STACK_SIZE];
			size_t current_node_childs_iterator[STACK_SIZE];
			size_t stack_index;

			enum class State
			{
				START = 0,
				ITERATION = 1,
				END = 100
			} state;

			inline static Traverse build(NTree<ElementType>* p_tree, const Token(NTreeNode)* p_start_node)
			{
				Traverse l_traverse;
				l_traverse.state = State::START;
				l_traverse.tree = p_tree;
				l_traverse.stack_index = -1;
				l_traverse.start_node = *p_start_node;
				return l_traverse;
			};

			inline static Traverse build(NTree<ElementType>* p_tree, Token(NTreeNode) p_start_node)
			{
				return build(p_tree, &p_start_node);
			};

			inline static Traverse build_default(NTree<ElementType>* p_tree)
			{
				return build(p_tree, Token(NTreeNode){0});
			};

			inline State step()
			{
				switch (this->state)
				{
				case State::START:
				{
					this->take_root_node();
				}
				break;
				case State::ITERATION:
				{
					this->iterate();
				}
				break;
				case State::END:
				{
					abort();
				}
				break;
				}

				return this->state;
			};

			inline Resolve* get_current_node()
			{
				return &this->node_stack[this->stack_index];
			};

		private:

			inline void take_root_node()
			{

#if CONTAINER_BOUND_TEST
				assert_true(this->tree->Indices.get_size() != 0);
#endif

				this->push_node_to_stack(&this->start_node);
				this->state = State::ITERATION;
			};

			inline void push_node_to_stack(const Token(NTreeNode)* p_node)
			{
				this->stack_index += 1;

#if CONTAINER_BOUND_TEST
				if (this->stack_index >= STACK_SIZE)
				{
					abort();
				}
#endif

				this->node_stack[this->stack_index] = this->tree->get_from_node(p_node);
				this->node_childs[this->stack_index] = this->tree->get_childs(&this->node_stack[this->stack_index].Node->childs);
				this->current_node_childs_iterator[this->stack_index] = -1;
			};

			inline void pop_node_from_stack()
			{
				this->stack_index -= 1;
			};

			inline void iterate()
			{
				// this->current_node_childs_iterator += 1;
				Slice<Token(NTreeNode)>* l_childs = &this->node_childs[this->stack_index];
				size_t* l_child_iterator = &this->current_node_childs_iterator[this->stack_index];
				*l_child_iterator += 1;

				if (*l_child_iterator >= l_childs->Size)
				{
					this->pop_node_from_stack();
					if (this->stack_index != -1)
					{
						this->iterate();
						return;
					}
					else
					{
						this->state = State::END;
						return;
					}
				}

				this->push_node_to_stack(l_childs->get(*l_child_iterator));

			};
		};

		inline void remove_node(const Token(NTreeNode)* p_node)
		{
			Vector<Resolve> l_involved_nodes = Vector<Resolve>::allocate(0);
			Traverse l_foreach_node = Traverse::build(this, p_node);
			while (l_foreach_node.step() != Traverse::State::END)
			{
				l_involved_nodes.push_back_element(l_foreach_node.get_current_node());
			}

			this->detach_from_tree(l_involved_nodes.get(0));
			for (vector_loop(&l_involved_nodes, i))
			{
				Resolve* l_removed_node = l_involved_nodes.get(i);
				this->Memory.release_element(cast(Token(size_t)*, &l_removed_node->Node->index));
				this->Indices.release_element(&l_removed_node->Node->index);
				this->Indices_childs.release_vector(&l_removed_node->Node->childs);
			}

			l_involved_nodes.free();
		};

		inline void remove_node(const Token(NTreeNode) p_node)
		{
			this->remove_node(&p_node);
		};

	private:

		inline void allocate_node(const Token(NTreeNode)* p_parent, const ElementType* p_element, Token(ElementType)* out_created_element, Token(NTreeNode)* out_created_index, NTreeChildsToken* out_created_childs)
		{
			*out_created_element = this->Memory.alloc_element(p_element);
			*out_created_childs = this->Indices_childs.alloc_vector();
			*out_created_index = this->Indices.alloc_element_1v(NTreeNode::build(cast(Token(NTreeNode)*, out_created_element), p_parent, out_created_childs));

			this->Indices_childs.element_push_back_element(&this->get_from_node(p_parent).Node->childs, out_created_index);
		};

		inline void allocate_root_node(const ElementType* p_element, Token(ElementType)* out_created_element, Token(NTreeNode)* out_created_index, NTreeChildsToken* out_created_childs)
		{
			*out_created_element = this->Memory.alloc_element(p_element);
			*out_created_childs = this->Indices_childs.alloc_vector();
			*out_created_index = this->Indices.alloc_element_1v(NTreeNode::build_index_childs(cast(Token(NTreeNode)*, out_created_element), out_created_childs));
		};

		inline void detach_from_tree(Resolve* p_node)
		{
			if (p_node->has_parent())
			{
				Resolve l_parent = this->get_from_node(&p_node->Node->parent);
				VectorOfVector_Element<Token(NTreeNode)> l_parent_childs = this->Indices_childs.get_vector(&l_parent.Node->childs);
				for (loop(i, 0, l_parent_childs.Header.Size))
				{
					if (l_parent_childs.get(i)->tok == p_node->Node->index.tok)
					{
						this->Indices_childs.element_erase_element_at_always(&l_parent.Node->childs, i);
						break;
					}
				}
			}
			p_node->Node->parent = token_build_default<NTreeNode>();
		};

		inline void detach_from_tree(Resolve p_node)
		{
			this->detach_from_tree(&p_node);
		};
	};

}
