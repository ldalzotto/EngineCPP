
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
		l_scene.step_end();
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

			l_scene.step_end();

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

			l_scene.step_end();

			assert_true(l_scene.component_events.Size == 0);

			l_scene.remove_node(l_scene.get_node(l_node_1));

			assert_true(l_scene.component_events.Size == 2);
			assert_true(tk_eq(l_scene.component_events.get(0).component, l_node_1_component_1));
			assert_true(l_scene.component_events.get(0).state == Scene::ComponentEvent::State::REMOVED);
			assert_true(tk_eq(l_scene.component_events.get(0).node, l_node_1));
			assert_true(tk_eq(l_scene.component_events.get(1).component, l_node_1_component_2));
			assert_true(l_scene.component_events.get(1).state == Scene::ComponentEvent::State::REMOVED);
			assert_true(tk_eq(l_scene.component_events.get(1).node, l_node_1));


			// We can still retrieve the removed component value before step_end is called
			ComponentTest* l_detached_component = l_scene.get_component_from_token_typed<ComponentTest>(l_node_1_component_1);
			assert_true(l_detached_component->i0 == 0);
			assert_true(l_detached_component->i1 == 1);
			assert_true(l_detached_component->i2 == 2);

			l_scene.step_end();
		}

		l_scene.free();
	};
}

int main()
{
	v2::add_remove_setparent_node();
	v2::add_remove_component();
};