#pragma once

#include "./component2.hpp"

#include "Math2/math.hpp"

namespace v2
{
	struct Scene
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

		/* The header of every components allocated on the heap. */
		struct NodeComponentHeader
		{
			const SceneNodeComponentType* type = nullptr;

			template<class ComponentType>
			ComponentType* cast_to_object();

			char* get_component_object();
		};

		struct Heap
		{
			HeapMemory component_heap;

			static Heap allocate_default();
			void free();

			Token<NodeComponentHeader> allocate_component(const Token<Node> p_node, const SceneNodeComponentType& p_component_type, const char* p_initial_value);
			NodeComponentHeader& get_component(const Token<NodeComponentHeader> p_component_header);
			void free_component(const Token<NodeComponentHeader> p_component);
		};

		Heap heap;

		NTree<Node> node_tree;
		PoolOfVector<Token<NodeComponentHeader>> node_to_components;

		//TODO -> event vectors

		static Scene allocate_default();
		void free();

		Token<Node> add_node(const transform& p_initial_local_transform);
		NTree<Node>::Resolve get_node_resolve(const Token<Node> p_node);



	private:
		Token<Node> allocate_node(const transform& p_initial_local_transform, const Token<Node> p_parent);
		void add_child(const NTree<Node>::Resolve& p_parent, NTree<Node>::Resolve& p_child);
		void mark_node_for_recalculation_tree(const NTree<Node>::Resolve& p_node);
		
		// Slice<Token<NodeComponentHeader>> get_node_components_token(const Token<Node> p_node);
		void free_node(const Token<Node> p_node);

	};

}