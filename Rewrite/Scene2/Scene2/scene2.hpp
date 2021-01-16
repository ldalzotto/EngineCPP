#pragma once

#include "./scene2_header.hpp"

namespace v2
{
	inline Scene::Node::State Scene::Node::State::build(const char p_matrices_must_be_recalculated, const char p_haschanged_thisframe)
	{
		return State{ p_matrices_must_be_recalculated, p_haschanged_thisframe };
	};

	inline Scene::Node Scene::Node::build_default()
	{
		Node l_node;
		l_node.state = State{ true, false };
		return l_node;
	};

	inline Scene::Node Scene::Node::build(const State& p_state, const transform& p_local_transform)
	{
		return Scene::Node{
			p_state,
			p_local_transform
		};
	};

	inline void Scene::Node::mark_for_recaluclation()
	{
		this->state = State{ true, true };
	};


	template<class ComponentType>
	inline ComponentType* Scene::NodeComponentHeader::cast_to_object()
	{
		return cast(ComponentType*, this->get_component_object());
	};

	inline char* Scene::NodeComponentHeader::get_component_object()
	{
		return cast(char*, this) + sizeof(NodeComponentHeader);
	};


	inline Scene::Heap Scene::Heap::allocate_default()
	{
		return Heap{
			HeapMemory::allocate_default()
		};
	};

	inline void Scene::Heap::free()
	{
		this->component_heap.free();
	};

	inline Token(Scene::NodeComponentHeader) Scene::Heap::allocate_component(const Token(Node) p_node, const SceneNodeComponentType& p_component_type, const char* p_initial_value)
	{
		return tk_bf(NodeComponentHeader,
			this->component_heap.allocate_element(Slice<char>::build_memory_elementnb(cast(char*, p_initial_value), p_component_type.size))
		);
	};

	inline Scene::NodeComponentHeader& Scene::Heap::get_component(const Token(NodeComponentHeader) p_component_header)
	{
		return (Scene::NodeComponentHeader&)*this->component_heap.get(tk_bf(SliceIndex, p_component_header)).Begin;
	};


	inline void Scene::Heap::free_component(const Token(NodeComponentHeader) p_component)
	{
		this->component_heap.release_element(tk_bf(SliceIndex, p_component));
	};



	inline Scene Scene::allocate_default()
	{
		Scene l_scene = Scene{
			Heap::allocate_default()
		};

		l_scene.add_node(transform_const::ORIGIN);

		return l_scene;
	};

	inline void free();


	inline Token(Scene::Node) Scene::add_node(const transform& p_initial_local_transform)
	{
		this->allocate_node(p_initial_local_transform, tk_bd(Node));
	};

	inline NTree<Scene::Node>::Resolve Scene::get_node_resolve(const Token(Node) p_node)
	{
		return this->node_tree.get(p_node);
	};


	inline Token(Scene::Node) Scene::allocate_node(const transform& p_initial_local_transform, const Token(Node) p_parent)
	{
		Token(Node) l_node = this->node_tree.push_value(
			Node::build(Node::State::build(1, 1), p_initial_local_transform),
			p_parent
		);
		this->node_to_components.alloc_vector();
		return l_node;
	};

	inline void Scene::add_child(const NTree<Node>::Resolve& p_parent, NTree<Node>::Resolve& p_child)
	{
		if (this->node_tree.add_child(p_parent, p_child))
		{
			this->mark_node_for_recalculation_tree(p_child);
		}
	};

	inline void Scene::mark_node_for_recalculation_tree(const NTree<Node>::Resolve& p_node)
	{
		tree_traverse2_begin(Node, Foreach)
			p_node.Element->mark_for_recaluclation();
		tree_traverse2_end(Node, &this->node_tree, p_node.Node->index, Foreach)
	};


	/*
	inline Slice<Token<Scene::NodeComponentHeader>> Scene::get_node_components_token(const Token<Node> p_node)
	{
		return this->node_to_components.get_vector(PoolOfVectorToken<Token<NodeComponentHeader>>{p_node.tok});
	};
	*/
	/*
	inline void Scene::free_node(const Token<Node> p_node)
	{
		Slice<Token<Scene::NodeComponentHeader>> l_node_components = this->get_components_token(p_node);
		for (loop(i, 0, l_node_components.Size))
		{
			this->free_component(l_node_components.get(i));
		}
		this->node_to_components.release_vector(PoolOfVectorToken<Token<NodeComponentHeader>>{p_node.tok});
		this->node_tree.remove_node(token_cast_v(NTreeNode, p_node));
	};
	*/
}