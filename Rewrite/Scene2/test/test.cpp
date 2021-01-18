
#include "Scene2/scene2.hpp"

namespace v2
{

	struct ComponentTest
	{
		static const SceneNodeComponentType Type;

		int i0, i1, i2;
	};

	constexpr SceneNodeComponentType ComponentTest::Type = SceneNodeComponentType::build(1, sizeof(ComponentTest));

	struct ComponentTest2
	{
		static const SceneNodeComponentType Type;

		size_t i0, i1, i2;
	};
	constexpr SceneNodeComponentType ComponentTest2::Type = SceneNodeComponentType::build(2, sizeof(ComponentTest2));

	// There is no component removal test here
	inline void add_remove_setparent_node()
	{
		Scene l_scene = Scene::allocate_default();

		transform l_node_1_transform = transform{ v3f_const::FORWARD, quat_const::IDENTITY, v3f_const::ZERO };

		Token(Node) l_node_1 = l_scene.add_node(l_node_1_transform, Scene_const::root_node);
		{
			assert_true(tk_v(l_node_1) == 1);

			NodeEntry l_node_1_value = l_scene.get_node(l_node_1);
			assert_true(l_node_1_value.has_parent() == 1);
			assert_true(l_node_1_value.Element->local_transform == l_node_1_transform);
			assert_true(l_node_1_value.Element->state.haschanged_thisframe == 1);
			assert_true(l_node_1_value.Element->state.matrices_mustBe_recalculated == 1);
		}

		Token(Node) l_node_2 = l_scene.add_node(l_node_1_transform, l_node_1);
		Token(Node) l_node_3 = l_scene.add_node(l_node_1_transform, l_node_1);

		{
			NodeEntry l_node_2_value = l_scene.get_node(l_node_2);
			NodeEntry l_node_3_value = l_scene.get_node(l_node_3);
			assert_true(tk_eq(l_scene.get_node_parent(l_node_2_value).Node->index, l_node_1));
			assert_true(tk_eq(l_scene.get_node_parent(l_node_3_value).Node->index, l_node_1));
			assert_true(l_scene.get_node_childs(l_scene.get_node(l_node_1)).Size == 2);
		}


		l_scene.add_node(l_node_1_transform, Scene_const::root_node);
		Token(Node) l_node_5 = l_scene.add_node(l_node_1_transform, l_node_3);

		// set_parent
		l_scene.step();
		assert_true(l_scene.get_node(l_node_3).Element->state.haschanged_thisframe == 0);
		assert_true(l_scene.get_node(l_node_5).Element->state.haschanged_thisframe == 0);

		// When a node parent has changed, the state of the node is set as if it's position has changed
		{
			l_scene.tree.add_child(l_scene.get_node(l_node_1), l_scene.get_node(l_node_3));
			assert_true(l_scene.get_node(l_node_3).Element->state.haschanged_thisframe == 1);
			assert_true(l_scene.get_node(l_node_5).Element->state.haschanged_thisframe == 1);
			assert_true(tk_eq(l_scene.get_node(l_node_3).Node->parent, l_scene.get_node(l_node_1).Node->index));
		}

		// remove node
		{
			l_scene.remove_node(l_scene.get_node(l_node_1));
			assert_true(l_scene.get_node_childs(l_scene.get_node(Scene_const::root_node)).Size == 1);

			// deleted nodes have been pushed to the deleted node stack
			assert_true(l_scene.node_that_will_be_destroyed.Size == 4);
			l_scene.step();
			assert_true(l_scene.node_that_will_be_destroyed.Size == 0);
		}

		l_scene.free();
	};

	inline void add_remove_component()
	{
		Scene l_scene = Scene::allocate_default();

		// Checking that added components can be retrieved later on
		{
			transform l_node_1_transform = transform{ v3f_const::FORWARD, quat_const::IDENTITY, v3f_const::ZERO };
			Token(Node) l_node_1 = l_scene.add_node(l_node_1_transform, Scene_const::root_node);
			Token(NodeComponentHeader) l_node_1_component_1 = l_scene.add_node_component_typed<ComponentTest>(l_node_1, ComponentTest{ 0,1,2 });

			ComponentTest* l_component_test = l_scene.get_node_component_typed<ComponentTest>(l_node_1);
			assert_true(l_component_test->i0 == 0);
			assert_true(l_component_test->i1 == 1);
			assert_true(l_component_test->i2 == 2);

			Token(NodeComponentHeader) l_node_1_component_2 = l_scene.add_node_component_typed<ComponentTest2>(l_node_1, ComponentTest2{ 0,1,2 });

			// We added two components, so there is two component added events
			assert_true(l_scene.component_events.Size == 2);
			assert_true(tk_eq(l_scene.component_events.get(0).component, l_node_1_component_1));
			assert_true(l_scene.component_events.get(0).state == Scene::ComponentEvent::State::ADDED);
			assert_true(tk_eq(l_scene.component_events.get(0).node, l_node_1));
			assert_true(tk_eq(l_scene.component_events.get(1).component, l_node_1_component_2));
			assert_true(l_scene.component_events.get(1).state == Scene::ComponentEvent::State::ADDED);
			assert_true(tk_eq(l_scene.component_events.get(1).node, l_node_1));

			l_scene.step();

			assert_true(l_scene.component_events.Size == 0);

			l_scene.remove_node_component_typed<ComponentTest>(l_node_1); //ensuring that no error
			assert_true(l_scene.get_node_component_typed<ComponentTest2>(l_node_1) != NULL);

			// One component removed event have been generated
			assert_true(l_scene.component_events.Size == 1);
			assert_true(tk_eq(l_scene.component_events.get(0).component, l_node_1_component_1));
			assert_true(l_scene.component_events.get(0).state == Scene::ComponentEvent::State::REMOVED);
			assert_true(tk_eq(l_scene.component_events.get(0).node, l_node_1));
		}

		l_scene.free();

		l_scene = Scene::allocate_default();

		// deleting a node with components will alse generate events
		{

			transform l_node_1_transform = transform{ v3f_const::FORWARD, quat_const::IDENTITY, v3f_const::ZERO };
			Token(Node) l_node_1 = l_scene.add_node(l_node_1_transform, Scene_const::root_node);
			Token(NodeComponentHeader) l_node_1_component_1 = l_scene.add_node_component_typed<ComponentTest>(l_node_1, ComponentTest{ 0,1,2 });
			Token(NodeComponentHeader) l_node_1_component_2 = l_scene.add_node_component_typed<ComponentTest2>(l_node_1, ComponentTest2{ 0,1,2 });

			l_scene.step();

			assert_true(l_scene.component_events.Size == 0);

			l_scene.remove_node(l_scene.get_node(l_node_1));

			assert_true(l_scene.component_events.Size == 2);
			assert_true(tk_eq(l_scene.component_events.get(0).component, l_node_1_component_1));
			assert_true(l_scene.component_events.get(0).state == Scene::ComponentEvent::State::REMOVED);
			assert_true(tk_eq(l_scene.component_events.get(0).node, l_node_1));
			assert_true(tk_eq(l_scene.component_events.get(1).component, l_node_1_component_2));
			assert_true(l_scene.component_events.get(1).state == Scene::ComponentEvent::State::REMOVED);
			assert_true(tk_eq(l_scene.component_events.get(1).node, l_node_1));


			// We can still retrieve the removed component value before step is called
			ComponentTest* l_detached_component = l_scene.get_component_from_token_typed<ComponentTest>(l_node_1_component_1);
			assert_true(l_detached_component->i0 == 0);
			assert_true(l_detached_component->i1 == 1);
			assert_true(l_detached_component->i2 == 2);

			l_scene.step();
		}

		l_scene.free();
	};


	inline void component_consume()
	{
		struct component_consume_callbacks
		{
			char component_test_1_added_called;
			char component_test_2_added_called;
			char component_test_1_removed_called;
			char component_test_2_removed_called;

			inline static component_consume_callbacks build_default()
			{
				return component_consume_callbacks{ 0,0,0,0 };
			};


			inline void on_component_added(Scene* p_scene, const NodeEntry& p_node, NodeComponentHeader* p_component)
			{
				if (p_component->type->id == ComponentTest::Type.id)
				{
					this->component_test_1_added_called = 1;
				}
				else if (p_component->type->id == ComponentTest2::Type.id)
				{
					this->component_test_2_added_called = 1;
				}
			};

			inline void on_component_removed(Scene* p_scene, const NodeEntry& p_node, NodeComponentHeader* p_component)
			{
				if (p_component->type->id == ComponentTest::Type.id)
				{
					this->component_test_1_removed_called = 1;
				}
				else if (p_component->type->id == ComponentTest2::Type.id)
				{
					this->component_test_2_removed_called = 1;
				}
			};

		};

		Scene l_scene = Scene::allocate_default();

		// Checking that added components can be retrieved later on
		{
			transform l_node_1_transform = transform{ v3f_const::FORWARD, quat_const::IDENTITY, v3f_const::ZERO };
			Token(Node) l_node_1 = l_scene.add_node(l_node_1_transform, Scene_const::root_node);
			l_scene.add_node_component_typed<ComponentTest>(l_node_1, ComponentTest{ 0,1,2 });
			l_scene.add_node_component_typed<ComponentTest2>(l_node_1, ComponentTest2{ 3,4,5 });

			component_consume_callbacks l_callbacks = component_consume_callbacks::build_default();
			l_scene.consume_component_events_stateful(l_callbacks);

			assert_true(l_callbacks.component_test_1_added_called);
			assert_true(l_callbacks.component_test_2_added_called);

			l_scene.remove_node_component_typed<ComponentTest2>(l_node_1);

			l_scene.consume_component_events_stateful(l_callbacks);

			assert_true(l_callbacks.component_test_2_removed_called);
			assert_true(!l_callbacks.component_test_1_removed_called);
		}

		l_scene.free();
	};

	inline void math_hierarchy()
	{
		Scene l_scene = Scene::allocate_default();

		Token(Node) l_node_1 = l_scene.add_node(transform_const::ORIGIN, tk_b(Node, 0));
		Token(Node) l_node_2 = l_scene.add_node(transform{ v3f_const::ONE, quat_const::IDENTITY, v3f_const::ONE }, l_node_1);
		Token(Node) l_node_3 = l_scene.add_node(transform{ v3f{-1.0f, -1.0f, -1.0f}, quat_const::IDENTITY, v3f_const::ONE }, l_node_1);

		NodeEntry l_node_1_val = l_scene.get_node(l_node_1);
		NodeEntry l_node_2_val = l_scene.get_node(l_node_2);
		NodeEntry l_node_3_val = l_scene.get_node(l_node_3);

		// Position
		{
			l_scene.tree.set_localposition(l_node_1_val, v3f{ 1.0f, 1.0f, 1.0f });
			assert_true(l_scene.tree.get_localposition(l_node_1_val) == v3f{ 1.00000000f, 1.00000000f, 1.00000000f });
			assert_true(l_scene.tree.get_worldposition(l_node_2_val) == v3f{ 2.00000000f, 2.00000000f, 2.00000000f });
			assert_true(l_scene.tree.get_worldposition(l_node_3_val) == v3f{ 0.000000000f, 0.000000000f, 0.000000000f });

			l_scene.tree.set_localposition(l_node_2_val, v3f{ 1.0f, 2.0f, 3.0f });
			assert_true(l_scene.tree.get_localposition(l_node_2_val) == v3f{ 1.00000000f, 2.00000000f, 3.00000000f });
			assert_true(l_scene.tree.get_worldposition(l_node_2_val) == v3f{ 2.00000000f, 3.00000000f, 4.00000000f });

			l_scene.tree.set_worldposition(l_node_3_val, v3f{ -1.0f, -2.0f, 3.0f });
			assert_true(l_scene.tree.get_localposition(l_node_3_val) == v3f{ -2.00000000f, -3.00000000f, 2.00000000f });
			assert_true(l_scene.tree.get_worldposition(l_node_3_val) == v3f{ -1.00000000f, -2.00000000f, 3.00000000f });
		}

		// Rotations
		{
			l_scene.tree.set_localrotation(l_node_1_val, v3f{ 0.32f, 0.9f, 0.7f }.euler_to_quat());
			assert_true(l_scene.tree.get_localrotation(l_node_1_val) == quat{ -0.0124835223f, 0.452567190f, 0.239721030f, 0.858813643f });
			assert_true(l_scene.tree.get_worldrotation(l_node_2_val) == quat{ -0.0124835242f, 0.452567250f, 0.239721060f, 0.858813763f });
			assert_true(l_scene.tree.get_worldrotation(l_node_3_val) == quat{ -0.0124835242f, 0.452567250f, 0.239721060f, 0.858813763f });

			l_scene.tree.set_localrotation(l_node_2_val, v3f{ 0.32f, -0.9f, -0.7f }.euler_to_quat());
			assert_true(l_scene.tree.get_localrotation(l_node_2_val) == quat{ -0.0124835223f, -0.452567190f, -0.239721030f, 0.858813643f });
			assert_true(l_scene.tree.get_worldrotation(l_node_2_val) == quat{ -0.0214420408f, -0.00598512636f, 0.0112992665f, 0.999688387f });

			l_scene.tree.set_worldrotation(l_node_3_val, v3f{ -1.0f, -2.0f, 3.0f }.euler_to_quat());
			assert_true(l_scene.tree.get_localrotation(l_node_3_val) == quat{ 0.346717477f, -0.641801417f, 0.598375380f, 0.331398427f });
			assert_true(l_scene.tree.get_worldrotation(l_node_3_val) == quat{ 0.718287051f, -0.310622454f, 0.444435090f, 0.435952842f });
		}

		// Scale
		{
			l_scene.tree.set_localscale(l_node_1_val, v3f{ 2.0f, 0.5f, 1.0f });
			assert_true(l_scene.tree.get_localscale(l_node_1_val) == v3f{ 2.00000000f, 0.500000000f, 1.00000000f });
			assert_true(l_scene.tree.get_worldscalefactor(l_node_2_val) == v3f{ 2.00000000f, 0.500000000f, 1.00000000f });
			assert_true(l_scene.tree.get_worldscalefactor(l_node_3_val) == v3f{ 2.00000000f, 0.500000000f, 1.00000000f });

			l_scene.tree.set_localscale(l_node_2_val, v3f{ 1.0f, 2.0f, 3.0f });
			assert_true(l_scene.tree.get_localscale(l_node_2_val) == v3f{ 1.00000000f, 2.00000000f, 3.00000000f });
			assert_true(l_scene.tree.get_worldscalefactor(l_node_2_val) == v3f{ 2.00000000f, 1.00000000f, 3.00000000f });


			l_scene.tree.set_worldscale(l_node_3_val, v3f{ -1.0f, -2.0f, 3.0f });
			assert_true(l_scene.tree.get_localscale(l_node_3_val) == v3f{ -0.500000000f, -4.00000000f, 3.00000000f });
			assert_true(l_scene.tree.get_worldscalefactor(l_node_3_val) == v3f{ -1.00000000f, -2.00000000f, 3.00000000f });
		}


		l_scene.free();
	};
}

int main()
{
	v2::add_remove_setparent_node();
	v2::add_remove_component();
	v2::component_consume();
	v2::math_hierarchy();
};