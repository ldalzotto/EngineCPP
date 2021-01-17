#pragma once

#include "./scene2_header.hpp"

namespace v2
{
	inline SceneTree::Node::State SceneTree::Node::State::build(const char p_matrices_must_be_recalculated, const char p_haschanged_thisframe)
	{
		return State{ p_matrices_must_be_recalculated, p_haschanged_thisframe };
	};

	inline SceneTree::Node SceneTree::Node::build_default()
	{
		Node l_node;
		l_node.state = State{ true, false };
		return l_node;
	};

	inline SceneTree::Node SceneTree::Node::build(const State& p_state, const transform& p_local_transform)
	{
		return SceneTree::Node{
			p_state,
			p_local_transform
		};
	};

	inline void SceneTree::Node::mark_for_recaluclation()
	{
		this->state = State{ true, true };
	};


	template<class ComponentType>
	inline ComponentType* SceneTree::NodeComponentHeader::cast_to_object()
	{
		return cast(ComponentType*, this->get_component_object());
	};

	inline char* SceneTree::NodeComponentHeader::get_component_object()
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

	inline Token(SceneTree::NodeComponentHeader) SceneTree::Heap::allocate_component(const Token(Node) p_node, const SceneNodeComponentType& p_component_type, const char* p_initial_value)
	{
		return tk_bf(NodeComponentHeader,
			this->component_heap.allocate_element(Slice<char>::build_memory_elementnb(cast(char*, p_initial_value), p_component_type.size))
		);
	};

	inline SceneTree::NodeComponentHeader& SceneTree::Heap::get_component(const Token(NodeComponentHeader) p_component_header)
	{
		return (SceneTree::NodeComponentHeader&)*this->component_heap.get(tk_bf(SliceIndex, p_component_header)).Begin;
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

	inline Token(SceneTree::Node) SceneTree::add_node(const transform& p_initial_local_transform, const Token(Node) p_parent)
	{
		return this->allocate_node(p_initial_local_transform, p_parent);
	};

	inline SceneTree::NodeEntry SceneTree::get_node(const Token(Node) p_node)
	{
		return this->node_tree.get(p_node);
	};

	inline SceneTree::NodeEntry SceneTree::get_node_parent(const NodeEntry& p_node)
	{
		return this->get_node(tk_bf(Node, p_node.Node->parent));
	};

	inline Slice<Token(SceneTree::Node)> SceneTree::get_node_childs(const NodeEntry& p_node)
	{
		Slice<Token(NTreeNode)> l_childs = this->node_tree.get_childs(p_node.Node->childs);
		return sliceoftoken_cast(SceneTree::Node, l_childs);	
	};

	inline void SceneTree::remove_node(const NodeEntry& p_node)
	{
		this->free_node_recurvise(p_node);
	};



	inline Token(SceneTree::NodeComponentHeader) SceneTree::add_node_component(const Token(Node) p_node, const SceneNodeComponentType& p_type, const char* p_initial_value)
	{
		Token(NodeComponentHeader) l_component_header = this->heap.allocate_component(p_node, p_type, p_initial_value);
		// We push to it's corresponding array
		this->node_to_components.element_push_back_element(tk_bf(Slice<Token(NodeComponentHeader)>, p_node), l_component_header);
		return l_component_header;
	};

	inline char SceneTree::get_node_component(const Token(Node) p_node, const Token(NodeComponentHeader) p_component, NodeComponentHeader** out_component)
	{
		Slice<Token(NodeComponentHeader)> l_components = this->get_node_components_token(p_node);
		for (loop(i, 0, l_components.Size))
		{
			Token(NodeComponentHeader)& l_component = l_components.get(i);
			if (tk_eq(p_component, l_component))
			{
				*out_component = &this->heap.get_component(l_component);
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
			NodeComponentHeader& l_component = this->heap.get_component(l_components.get(i));
			if (l_component.type->id == p_type.id)
			{
				*out_component = &l_component;
				return 1;
			}
		}

		*out_component = NULL;
		return 0;
	};

	inline void SceneTree::remove_node_component(const Token(Node) p_node, const Token(NodeComponentHeader) p_component)
	{
		Slice<Token(NodeComponentHeader)> l_components = this->get_node_components_token(p_node);
		for (loop(i, 0, l_components.Size))
		{
			if (tk_eq(p_component, l_components.get(i)))
			{
				this->node_to_components.element_erase_element_at_always(tk_bf(Slice<Token(NodeComponentHeader)>, p_node), i);
				break;
			}
		}
	};

	inline void SceneTree::remove_node_component_by_type(const Token(Node) p_node, const SceneNodeComponentType& p_type)
	{
		Slice<Token(NodeComponentHeader)> l_components = this->get_node_components_token(p_node);
		for (loop(i, 0, l_components.Size))
		{
			NodeComponentHeader& l_component = this->heap.get_component(l_components.get(i));
			if (l_component.type->id == p_type.id)
			{
				this->node_to_components.element_erase_element_at_always(tk_bf(Slice<Token(NodeComponentHeader)>, p_node), i);
				break;
			}
		}
	};

	inline Token(SceneTree::Node) SceneTree::allocate_node(const transform& p_initial_local_transform, const Token(Node) p_parent)
	{
		Token(Node) l_node = this->node_tree.push_value(
			Node::build(Node::State::build(1, 1), p_initial_local_transform),
			p_parent
		);
		this->node_to_components.alloc_vector();
		return l_node;
	};

	inline Token(SceneTree::Node) SceneTree::allocate_root_node()
	{
		Token(Node) l_node = this->node_tree.push_root_value(
			Node::build(Node::State::build(1, 1), transform_const::ORIGIN)
		);
		this->node_to_components.alloc_vector();
		return l_node;
	};

	inline void SceneTree::add_child(const NodeEntry& p_parent, NodeEntry& p_child)
	{
		if (this->node_tree.add_child(p_parent, p_child))
		{
			this->mark_node_for_recalculation_recursive(p_child);
		}
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

		//TODO -> foreach nodes
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
		Slice<Token(SceneTree::NodeComponentHeader)> l_node_components = this->get_node_components_token(tk_bf(SceneTree::Node, p_node.Node->index));
		for (loop(i, 0, l_node_components.Size))
		{
			this->heap.free_component(l_node_components.get(i));
		}
		this->node_to_components.release_vector(tk_bf(Slice<Token(NodeComponentHeader)>, p_node.Node->index));
	};

	inline Slice<Token(SceneTree::NodeComponentHeader)> SceneTree::get_node_components_token(const Token(Node) p_node)
	{
		return this->node_to_components.get_vector(tk_bf(Slice<Token(NodeComponentHeader)>, p_node));
	};


}