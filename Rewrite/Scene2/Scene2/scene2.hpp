#pragma once

#include "./scene2_header.hpp"

namespace v2
{
	inline Node::State Node::State::build(const char p_matrices_must_be_recalculated, const char p_haschanged_thisframe)
	{
		return State{ p_matrices_must_be_recalculated, p_haschanged_thisframe };
	};

	inline Node Node::build_default()
	{
		Node l_node;
		l_node.state = State{ true, false };
		return l_node;
	};

	inline Node Node::build(const State& p_state, const transform& p_local_transform)
	{
		return Node{
			p_state,
			p_local_transform
		};
	};

	inline void Node::mark_for_recaluclation()
	{
		this->state = State{ true, true };
	};


	template<class ComponentType>
	inline ComponentType* NodeComponentHeader::cast_to_object()
	{
		return cast(ComponentType*, this->get_component_object());
	};

	inline char* NodeComponentHeader::get_component_object()
	{
		return cast(char*, this) + sizeof(NodeComponentHeader);
	};


	inline SceneTree::Heap SceneTree::Heap::allocate_default()
	{
		return Heap{
			HeapMemory::allocate_default()
		};
	};

	inline void SceneTree::Heap::free()
	{
		this->component_heap.free();
	};

	inline Token(NodeComponentHeader) SceneTree::Heap::allocate_component(const Token(Node) p_node, const SceneNodeComponentType& p_component_type, const char* p_initial_value)
	{
		Slice<char> l_allocated_chunk;
		Token(SliceIndex) l_chunk_token = this->component_heap.allocate_empty_element_return_chunk(sizeof(NodeComponentHeader) + p_component_type.size, &l_allocated_chunk);
		((NodeComponentHeader*)l_allocated_chunk.Begin)->type = &p_component_type;
		slice_memcpy(l_allocated_chunk.slide_rv(sizeof(NodeComponentHeader)), Slice<char>::build_memory_elementnb((char*)p_initial_value, p_component_type.size));
		return tk_bf(NodeComponentHeader, l_chunk_token);
	};

	inline NodeComponentHeader* SceneTree::Heap::get_component(const Token(NodeComponentHeader) p_component_header)
	{
		return (NodeComponentHeader*)this->component_heap.get(tk_bf(SliceIndex, p_component_header)).Begin;
	};


	inline void SceneTree::Heap::free_component(const Token(NodeComponentHeader) p_component)
	{
		this->component_heap.release_element(tk_bf(SliceIndex, p_component));
	};



	inline SceneTree SceneTree::allocate_default()
	{
		SceneTree l_scene = SceneTree{
			Heap::allocate_default(),
			NTree<Node>::allocate_default(),
			PoolOfVector<Token(NodeComponentHeader)>::allocate_default()
		};

		l_scene.allocate_root_node();

		return l_scene;
	};

	inline void SceneTree::free()
	{
		this->heap.free();
		this->node_tree.free();
		this->node_to_components.free();
	};

	inline Token(Node) SceneTree::add_node(const transform& p_initial_local_transform, const Token(Node) p_parent)
	{
		return this->allocate_node(p_initial_local_transform, p_parent);
	};

	inline NodeEntry SceneTree::get_node(const Token(Node) p_node)
	{
		return this->node_tree.get(p_node);
	};

	inline NodeEntry SceneTree::get_node_parent(const NodeEntry& p_node)
	{
		return this->get_node(tk_bf(Node, p_node.Node->parent));
	};

	inline Slice<Token(Node)> SceneTree::get_node_childs(const NodeEntry& p_node)
	{
		Slice<Token(NTreeNode)> l_childs = this->node_tree.get_childs(p_node.Node->childs);
		return sliceoftoken_cast(Node, l_childs);
	};

	inline void SceneTree::add_child(const NodeEntry& p_parent, const NodeEntry& p_child)
	{
		if (this->node_tree.add_child(p_parent, p_child))
		{
			this->mark_node_for_recalculation_recursive(p_child);
		}
	};

	inline void SceneTree::remove_node(const NodeEntry& p_node)
	{
		this->free_node_recurvise(p_node);
	};



	inline Token(NodeComponentHeader) SceneTree::add_node_component(const Token(Node) p_node, const SceneNodeComponentType& p_type, const char* p_initial_value)
	{
		Token(NodeComponentHeader) l_component_header = this->heap.allocate_component(p_node, p_type, p_initial_value);
		// We push to it's corresponding array
		this->node_to_components.element_push_back_element(tk_bf(Slice<Token(NodeComponentHeader)>, p_node), l_component_header);
		return l_component_header;
	};

	inline Slice<Token(NodeComponentHeader)> SceneTree::get_node_components_token(const Token(Node) p_node)
	{
		return this->node_to_components.get_vector(tk_bf(Slice<Token(NodeComponentHeader)>, p_node));
	};

	inline char SceneTree::get_node_component(const Token(Node) p_node, const Token(NodeComponentHeader) p_component, NodeComponentHeader** out_component)
	{
		Slice<Token(NodeComponentHeader)> l_components = this->get_node_components_token(p_node);
		for (loop(i, 0, l_components.Size))
		{
			Token(NodeComponentHeader)& l_component = l_components.get(i);
			if (tk_eq(p_component, l_component))
			{
				*out_component = this->heap.get_component(l_component);
				return 1;
			}
		}

		*out_component = NULL;
		return 0;
	};

	inline char SceneTree::get_node_component_by_type(const Token(Node) p_node, const SceneNodeComponentType& p_type, NodeComponentHeader** out_component)
	{
		Slice<Token(NodeComponentHeader)> l_components = this->get_node_components_token(p_node);
		for (loop(i, 0, l_components.Size))
		{
			NodeComponentHeader* l_component = this->heap.get_component(l_components.get(i));
			if (l_component->type->id == p_type.id)
			{
				*out_component = l_component;
				return 1;
			}
		}

		*out_component = NULL;
		return 0;
	};

	/*
	inline char SceneTree::remove_node_component(const Token(Node) p_node, const Token(NodeComponentHeader) p_component)
	{
		Slice<Token(NodeComponentHeader)> l_components = this->get_node_components_token(p_node);
		for (loop(i, 0, l_components.Size))
		{
			if (tk_eq(p_component, l_components.get(i)))
			{
				this->node_to_components.element_erase_element_at_always(tk_bf(Slice<Token(NodeComponentHeader)>, p_node), i);
				return 1;
				break;
			}
		}
		return 0;
	};

	inline char SceneTree::remove_node_component_by_type(const Token(Node) p_node, const SceneNodeComponentType& p_type)
	{
		Slice<Token(NodeComponentHeader)> l_components = this->get_node_components_token(p_node);
		for (loop(i, 0, l_components.Size))
		{
			NodeComponentHeader* l_component = this->heap.get_component(l_components.get(i));
			if (l_component->type->id == p_type.id)
			{
				this->node_to_components.element_erase_element_at_always(tk_bf(Slice<Token(NodeComponentHeader)>, p_node), i);
				return 1;
				break;
			}
		}
		return 0;
	};
	*/

	inline void SceneTree::detach_all_node_components(const Token(Node) p_node)
	{
		this->node_to_components.element_clear(tk_bf(Slice<Token(NodeComponentHeader)>, p_node));
	};

	inline char SceneTree::detach_node_component_by_type(const Token(Node) p_node, const SceneNodeComponentType& p_type, Token(NodeComponentHeader)* out_component)
	{
		Slice<Token(NodeComponentHeader)> l_components = this->get_node_components_token(p_node);
		for (loop(i, 0, l_components.Size))
		{
			Token(NodeComponentHeader)& l_component_token = l_components.get(i);
			NodeComponentHeader* l_component = this->heap.get_component(l_component_token);
			if (l_component->type->id == p_type.id)
			{
				*out_component = l_component_token;
				this->node_to_components.element_erase_element_at_always(tk_bf(Slice<Token(NodeComponentHeader)>, p_node), i);
				return 1;
			}
		}
		*out_component = tk_bd(NodeComponentHeader);
		return 0;
	};

	inline void SceneTree::free_component(const Token(NodeComponentHeader) p_component)
	{
		this->heap.free_component(p_component);
	};

	inline void SceneTree::clear_nodes_state()
	{
		tree_traverse2_begin(Node, Foreach)
			p_node.Element->state.haschanged_thisframe = false;
		tree_traverse2_end(Node, &this->node_tree, tk_b(NTreeNode, 0), Foreach)
	};

	inline Token(Node) SceneTree::allocate_node(const transform& p_initial_local_transform, const Token(Node) p_parent)
	{
		Token(Node) l_node = this->node_tree.push_value(
			Node::build(Node::State::build(1, 1), p_initial_local_transform),
			p_parent
		);
		this->node_to_components.alloc_vector();
		return l_node;
	};

	inline Token(Node) SceneTree::allocate_root_node()
	{
		Token(Node) l_node = this->node_tree.push_root_value(
			Node::build(Node::State::build(1, 1), transform_const::ORIGIN)
		);
		this->node_to_components.alloc_vector();
		return l_node;
	};

	inline void SceneTree::mark_node_for_recalculation_recursive(const NodeEntry& p_node)
	{
		tree_traverse2_begin(Node, Foreach)
			p_node.Element->mark_for_recaluclation();
		tree_traverse2_end(Node, &this->node_tree, p_node.Node->index, Foreach)
	};

	inline void SceneTree::free_node_recurvise(const NodeEntry& p_node)
	{
		Vector<NodeEntry> l_deleted_nodes = Vector<NodeEntry>::allocate(0);
		this->node_tree.get_nodes(p_node.Node->index, &l_deleted_nodes);

		for (loop(i, 0, l_deleted_nodes.Size))
		{
			this->free_node_single(l_deleted_nodes.get(i));
		}

		Slice<NodeEntry> l_deleted_nodes_slice = l_deleted_nodes.to_slice();
		this->node_tree.remove_nodes_and_detach(l_deleted_nodes_slice);
		l_deleted_nodes.free();
	};

	inline void SceneTree::free_node_single(const NodeEntry& p_node)
	{
		Slice<Token(NodeComponentHeader)> l_node_components = this->get_node_components_token(tk_bf(Node, p_node.Node->index));
		for (loop(i, 0, l_node_components.Size))
		{
			this->heap.free_component(l_node_components.get(i));
		}
		this->node_to_components.release_vector(tk_bf(Slice<Token(NodeComponentHeader)>, p_node.Node->index));
	};







	inline Scene Scene::allocate_default()
	{
		return Scene{
			SceneTree::allocate_default(),
			Vector<Token(Node)>::allocate(0),
			Vector<ComponentEvent>::allocate(0)
		};
	};

	inline void Scene::free()
	{
		this->step_destroy_resource_only();

		this->tree.free();
		this->orphan_nodes.free();
		this->component_events.free();
	};

	inline void Scene::step_end()
	{
		this->destroy_orphan_nodes();
		this->destroy_component_events();
		this->tree.clear_nodes_state();
	};


	inline Token(Node) Scene::add_node(const transform& p_initial_local_transform, const Token(Node) p_parent)
	{
		return this->tree.add_node(p_initial_local_transform, p_parent);
	};

	inline NodeEntry Scene::get_node(const Token(Node) p_node)
	{
		return this->tree.get_node(p_node);
	};

	inline NodeEntry Scene::get_node_parent(const NodeEntry& p_node)
	{
		return this->tree.get_node_parent(p_node);
	};

	inline Slice<Token(Node)> Scene::get_node_childs(const NodeEntry& p_node)
	{
		return this->tree.get_node_childs(p_node);
	};

	inline void Scene::remove_node(const NodeEntry& p_node)
	{
		// get all components -> for pushing events
		Slice<Token(NodeComponentHeader)> l_node_component_tokens = this->tree.get_node_components_token(tk_bf(Node, p_node.Node->index));
		for (loop(i, 0, l_node_component_tokens.Size))
		{
			this->component_events.push_back_element(ComponentEvent{ ComponentEvent::State::REMOVED, tk_bf(Node, p_node.Node->index), l_node_component_tokens.get(i) });
		}
		this->tree.detach_all_node_components(tk_bf(Node, p_node.Node->index));

		NodeEntry l_node_copy = p_node;
		this->tree.node_tree.make_node_orphan(l_node_copy);
		this->orphan_nodes.push_back_element(tk_bf(Node, p_node.Node->index));
	};

	template<class ComponentType>
	inline Token(NodeComponentHeader) Scene::add_node_component_typed(const Token(Node) p_node, const ComponentType& p_intial_value)
	{
		Token(NodeComponentHeader) l_added_component = this->tree.add_node_component(p_node, ComponentType::Type, cast(const char*, &p_intial_value));
		this->component_events.push_back_element(ComponentEvent{ ComponentEvent::State::ADDED, p_node, l_added_component });
		return l_added_component;
	};

	template<class ComponentType>
	inline ComponentType* Scene::get_node_component_typed(const Token(Node) p_node)
	{
		NodeComponentHeader* l_component_header;

#if SCENE_BOUND_TEST
		if (!this->tree.get_node_component_by_type(p_node, ComponentType::Type, &l_component_header))
		{
			abort();
		};
#else
		this->tree.get_node_component_by_type(p_node, ComponentType::Type, &l_component_header);
#endif

		return l_component_header->cast_to_object<ComponentType>();
	};

	template<class ComponentType>
	inline ComponentType* Scene::get_component_from_token_typed(const Token(NodeComponentHeader) p_component)
	{
		return this->tree.heap.get_component(p_component)->cast_to_object<ComponentType>();
	};

	template<class ComponentType>
	inline void Scene::remove_node_component_typed(const Token(Node) p_node)
	{
		Token(NodeComponentHeader) l_detached_component;
		if (!this->tree.detach_node_component_by_type(p_node, ComponentType::Type, &l_detached_component))
		{
			abort();
		};
		this->component_events.push_back_element(ComponentEvent{ ComponentEvent::State::REMOVED, p_node, l_detached_component });
	};

	inline void Scene::remove_node_component(const Token(Node) p_node, const Token(NodeComponentHeader) p_component)
	{
		// this->tree.detach_node_component_by_type()
	};


	inline void Scene::step_destroy_resource_only()
	{
		this->destroy_orphan_nodes();
		this->destroy_component_events();
	};

	inline void Scene::destroy_orphan_nodes()
	{
		for (vector_loop(&this->orphan_nodes, i))
		{
			this->tree.remove_node(this->tree.get_node(this->orphan_nodes.get(i)));
		}
		this->orphan_nodes.clear();
	};

	inline void Scene::destroy_component_events()
	{
		for (vector_loop(&this->component_events, i))
		{
			ComponentEvent& l_component_event = this->component_events.get(i);
			if (l_component_event.state == ComponentEvent::State::REMOVED)
			{
				this->tree.free_component(l_component_event.component);
			};
		}
		this->component_events.clear();
	};
}