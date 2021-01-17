#pragma once

#include "./component2.hpp"

#include "Math2/math.hpp"

namespace v2
{
	struct Node
	{
		struct State
		{
			char matrices_mustBe_recalculated; /* = true;*/
			char haschanged_thisframe; /* = false; */

			static State build(const char p_matrices_must_be_recalculated, const char p_haschanged_thisframe);

		} state;

		//transform
		transform local_transform;

		/** This matrix will always be relative to the root Node (a Node without parent). */
		m44f localtoworld;

		static Node build_default();
		static Node build(const State& p_state, const transform& p_local_transform);

		void mark_for_recaluclation();
	};

	using NodeEntry = NTree<Node>::Resolve;

	/* The header of every components allocated on the heap. */
	struct NodeComponentHeader
	{
		const SceneNodeComponentType* type = nullptr;

		template<class ComponentType>
		ComponentType* cast_to_object();

		char* get_component_object();
	};

	struct SceneTree
	{
		struct Heap
		{
			HeapMemory component_heap;

			static Heap allocate_default();
			void free();

			Token(NodeComponentHeader) allocate_component(const Token(Node) p_node, const SceneNodeComponentType& p_component_type, const char* p_initial_value);
			NodeComponentHeader* get_component(const Token(NodeComponentHeader) p_component_header);
			void free_component(const Token(NodeComponentHeader) p_component);
		};

		Heap heap;
		NTree<Node> node_tree;
		PoolOfVector<Token(NodeComponentHeader)> node_to_components;

		static SceneTree allocate_default();
		void free();

		Token(Node) add_node(const transform& p_initial_local_transform, const Token(Node) p_parent);
		NodeEntry get_node(const Token(Node) p_node);
		NodeEntry get_node_parent(const NodeEntry& p_node);
		
		Slice<Token(Node)> get_node_childs(const NodeEntry& p_node);
		void add_child(const NodeEntry& p_parent, const  NodeEntry& p_child);
		
		void remove_node(const NodeEntry& p_node);

		Token(NodeComponentHeader) add_node_component(const Token(Node) p_node, const SceneNodeComponentType& p_type, const char* p_initial_value);
		Slice<Token(NodeComponentHeader)> get_node_components_token(const Token(Node) p_node);
		char get_node_component(const Token(Node) p_node, const Token(NodeComponentHeader) p_component, NodeComponentHeader** out_component);
		char get_node_component_by_type(const Token(Node) p_node, const SceneNodeComponentType& p_type, NodeComponentHeader** out_component);
		// char remove_node_component(const Token(Node) p_node, const Token(NodeComponentHeader) p_component);
		// char remove_node_component_by_type(const Token(Node) p_node, const SceneNodeComponentType& p_type);
		void detach_all_node_components(const Token(Node) p_node);
		char detach_node_component_by_type(const Token(Node) p_node, const SceneNodeComponentType& p_type, Token(NodeComponentHeader)* out_component);
		void free_component(const Token(NodeComponentHeader) p_component);

		void clear_nodes_state();

	private:
		Token(Node) allocate_node(const transform& p_initial_local_transform, const Token(Node) p_parent);
		Token(Node) allocate_root_node();
		void mark_node_for_recalculation_recursive(const NodeEntry& p_node);
		
		void free_node_recurvise(const NodeEntry& p_node);
		void free_node_single(const NodeEntry& p_node);
	};

	struct Scene_const
	{
		inline static Token(Node) root_node = tk_b(Node, 0);
	};

	struct Scene
	{
		struct ComponentEvent
		{
			enum class State : char {ADDED = 0, REMOVED = 1} state;
			Token(Node) node;
			Token(NodeComponentHeader) component;
		};

		SceneTree tree;
		Vector<Token(Node)> orphan_nodes;
		Vector<ComponentEvent> component_events;

		static Scene allocate_default();
		void free();

		void step_begin();
		void step_end();

		Token(Node) add_node(const transform& p_initial_local_transform, const Token(Node) p_parent);
		NodeEntry get_node(const Token(Node) p_node);
		NodeEntry get_node_parent(const NodeEntry& p_node);
		Slice<Token(Node)> get_node_childs(const NodeEntry& p_node);
		void remove_node(const NodeEntry& p_node);


		// Token(NodeComponentHeader) add_node_component(const Token(Node) p_node, const SceneNodeComponentType& p_type, const char* p_initial_value);
		template<class ComponentType>
		Token(NodeComponentHeader) add_node_component_typed(const Token(Node) p_node, const ComponentType& p_intial_value);

		template<class ComponentType>
		ComponentType* get_node_component_typed(const Token(Node) p_node);

		template<class ComponentType>
		ComponentType* get_component_from_token_typed(const Token(NodeComponentHeader) p_component);

		template<class ComponentType>
		void remove_node_component_typed(const Token(Node) p_node);

	private:

		void remove_node_component(const Token(Node) p_node, const Token(NodeComponentHeader) p_component);

		void step_destroy_resource_only();
		void destroy_orphan_nodes();
		void destroy_component_events();
	};

}