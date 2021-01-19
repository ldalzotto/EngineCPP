#pragma once

#include "./scene2_header.hpp"

namespace v2
{
	inline Node::State Node::State::build(const int8 p_matrices_must_be_recalculated, const int8 p_haschanged_thisframe)
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

	inline int8* NodeComponentHeader::get_component_object()
	{
		return cast(int8*, this) + sizeof(NodeComponentHeader);
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

	inline Token(NodeComponentHeader) SceneTree::Heap::allocate_component(const Token(Node) p_node, const SceneNodeComponentType& p_component_type, const int8* p_initial_value)
	{
		Slice<int8> l_allocated_chunk;
		Token(SliceIndex) l_chunk_token = this->component_heap.allocate_empty_element_return_chunk(sizeof(NodeComponentHeader) + p_component_type.size, &l_allocated_chunk);
		((NodeComponentHeader*)l_allocated_chunk.Begin)->type = &p_component_type;
		slice_memcpy(l_allocated_chunk.slide_rv(sizeof(NodeComponentHeader)), Slice<int8>::build_memory_elementnb((int8*)p_initial_value, p_component_type.size));
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



	inline Token(NodeComponentHeader) SceneTree::add_node_component(const Token(Node) p_node, const SceneNodeComponentType& p_type, const int8* p_initial_value)
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

	inline int8 SceneTree::get_node_component(const Token(Node) p_node, const Token(NodeComponentHeader) p_component, NodeComponentHeader** out_component)
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

	inline int8 SceneTree::get_node_component_by_type(const Token(Node) p_node, const SceneNodeComponentType& p_type, NodeComponentHeader** out_component)
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
	inline int8 SceneTree::remove_node_component(const Token(Node) p_node, const Token(NodeComponentHeader) p_component)
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

	inline int8 SceneTree::remove_node_component_by_type(const Token(Node) p_node, const SceneNodeComponentType& p_type)
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

	inline int8 SceneTree::detach_node_component_by_type(const Token(Node) p_node, const SceneNodeComponentType& p_type, Token(NodeComponentHeader)* out_component)
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


	inline v3f& SceneTree::get_localposition(const NodeEntry& p_node)
	{
		return p_node.Element->local_transform.position;
	};

	inline quat& SceneTree::get_localrotation(const NodeEntry& p_node)
	{
		return p_node.Element->local_transform.rotation;
	};

	inline v3f& SceneTree::get_localscale(const NodeEntry& p_node)
	{
		return p_node.Element->local_transform.scale;
	};

	inline void SceneTree::set_localposition(const NodeEntry& p_node, const v3f& p_local_position)
	{
		if (p_node.Element->local_transform.position != p_local_position)
		{
			this->mark_node_for_recalculation_recursive(p_node);
			p_node.Element->local_transform.position = p_local_position;
		}
	};

	inline void SceneTree::set_localrotation(const NodeEntry& p_node, const quat& p_local_rotation)
	{
		if (p_node.Element->local_transform.rotation != p_local_rotation)
		{
			this->mark_node_for_recalculation_recursive(p_node);
			p_node.Element->local_transform.rotation = p_local_rotation;
		}
	};

	inline void SceneTree::set_localscale(const NodeEntry& p_node, const v3f& p_local_scale)
	{
		if (p_node.Element->local_transform.scale != p_local_scale)
		{
			this->mark_node_for_recalculation_recursive(p_node);
			p_node.Element->local_transform.scale = p_local_scale;
		}
	};

	inline void SceneTree::set_worldposition(const NodeEntry& p_node, const v3f& p_world_position)
	{
		if (!p_node.has_parent())
		{
			this->set_localposition(p_node, p_world_position);
		}
		else
		{
			NodeEntry l_parent = this->get_node(tk_bf(Node, p_node.Node->parent));
			this->set_localposition(p_node, (this->get_worldtolocal(l_parent) * v4f::build_v3f_s(p_world_position, 1.0f)).Vec3);
		}
	};

	inline void SceneTree::set_worldrotation(const NodeEntry& p_node, const quat& p_world_rotation)
	{
		if (!p_node.has_parent())
		{
			this->set_localrotation(p_node, p_world_rotation);
		}
		else
		{
			NodeEntry l_parent = this->get_node(tk_bf(Node, p_node.Node->parent));
			this->set_localrotation(p_node, this->get_worldrotation(l_parent).inv() * p_world_rotation);
		}
	};

	inline void SceneTree::set_worldscale(const NodeEntry& p_node, const v3f& p_world_scale)
	{
		if (!p_node.has_parent())
		{
			set_localscale(p_node, p_world_scale);
		}
		else
		{
			NodeEntry l_parent = this->get_node(tk_bf(Node, p_node.Node->parent));
			set_localscale(p_node, p_world_scale * this->get_worldscalefactor(l_parent).inv());
		}
	};

	inline v3f SceneTree::get_worldposition(const NodeEntry& p_node)
	{
		return this->get_localtoworld(p_node).get_translation();
	};

	inline quat SceneTree::get_worldrotation(const NodeEntry& p_node)
	{
		if (!p_node.has_parent())
		{
			return p_node.Element->local_transform.rotation;
		}
		else
		{
			NodeEntry l_parent = this->get_node(tk_bf(Node, p_node.Node->parent));
			return this->get_worldrotation(l_parent) * p_node.Element->local_transform.rotation;
		}
	};

	inline v3f SceneTree::get_worldscalefactor(const NodeEntry& p_node)
	{
		if (!p_node.has_parent())
		{
			return p_node.Element->local_transform.scale;
		}
		else
		{
			NodeEntry l_parent = this->get_node(tk_bf(Node, p_node.Node->parent));
			return this->get_worldscalefactor(l_parent) * p_node.Element->local_transform.scale;
		}
	};

	inline m44f& SceneTree::get_localtoworld(const NodeEntry& p_node)
	{
		this->updatematrices_if_necessary(p_node);
		return p_node.Element->localtoworld;
	};

	inline m44f SceneTree::get_worldtolocal(const NodeEntry& p_node)
	{
		return this->get_localtoworld(p_node).inv();
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

	inline void SceneTree::updatematrices_if_necessary(const NodeEntry& p_node)
	{
		if (p_node.Element->state.matrices_mustBe_recalculated)
		{
			p_node.Element->localtoworld = m44f::trs(p_node.Element->local_transform.position, p_node.Element->local_transform.rotation.to_axis(), p_node.Element->local_transform.scale);

			if (p_node.has_parent())
			{
				NodeEntry l_parent = this->get_node(tk_bf(Node, p_node.Node->parent));
				p_node.Element->localtoworld = this->get_localtoworld(l_parent) * p_node.Element->localtoworld;
			}
			p_node.Element->state.matrices_mustBe_recalculated = false;
		}
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
			Vector<Token(Node)>::allocate(0),
			Vector<ComponentEvent>::allocate(0)
		};
	};

	inline void Scene::free()
	{
		this->step_destroy_resource_only();

		this->tree.free();
		this->orphan_nodes.free();
		this->node_that_will_be_destroyed.free();
		this->component_events.free();
	};

	template<class ComponentEventCallbackFunc>
	inline void Scene::consume_component_events()
	{
		for (vector_loop(&this->component_events, i))
		{
			ComponentEvent& l_component_event = this->component_events.get(i);

			if (l_component_event.state == ComponentEvent::State::REMOVED)
			{
				ComponentEventCallbackFunc::on_component_removed(this, this->get_node(l_component_event.node), this->tree.heap.get_component(l_component_event.component));
				this->tree.free_component(l_component_event.component);
			}
			else
			{
				NodeComponentHeader* l_node_component;
				this->tree.get_node_component(l_component_event.node, l_component_event.component, &l_node_component);
				ComponentEventCallbackFunc::on_component_added(this, this->get_node(l_component_event.node), l_node_component);
			};
		};
		this->component_events.clear();
	};

	template<class ComponentEventCallbackObj>
	inline void Scene::consume_component_events_stateful(ComponentEventCallbackObj& p_closure)
	{
		for (vector_loop(&this->component_events, i))
		{
			ComponentEvent& l_component_event = this->component_events.get(i);

			if (l_component_event.state == ComponentEvent::State::REMOVED)
			{
				p_closure.on_component_removed(this, this->get_node(l_component_event.node), this->tree.heap.get_component(l_component_event.component));
				this->tree.free_component(l_component_event.component);
			}
			else
			{
				NodeComponentHeader* l_node_component;
				this->tree.get_node_component(l_component_event.node, l_component_event.component, &l_node_component);
				p_closure.on_component_added(this, this->get_node(l_component_event.node), l_node_component);
			};
		};
		this->component_events.clear();
	};

	inline void Scene::step()
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

		tree_traverse2_stateful_begin(Node, Vector<Token(Node)>*p_node_that_will_be_destroyed, GetAllNodes)
			p_node_that_will_be_destroyed->push_back_element(tk_bf(Node, p_node.Node->index));
		tree_traverse2_stateful_end(Node, &this->tree.node_tree, p_node.Node->index, &this->node_that_will_be_destroyed, GetAllNodes)
	};

	template<class ComponentType>
	inline Token(NodeComponentHeader) Scene::add_node_component_typed(const Token(Node) p_node, const ComponentType& p_intial_value)
	{
		Token(NodeComponentHeader) l_added_component = this->tree.add_node_component(p_node, ComponentType::Type, cast(const int8*, &p_intial_value));
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
		this->node_that_will_be_destroyed.clear();
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