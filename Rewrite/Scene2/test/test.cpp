
#include "Scene2/scene2.hpp"

namespace v2
{
	inline void add_remove_setparent_node()
	{
		SceneTree l_scene_tree = SceneTree::allocate_default();

		transform l_node_1_transform = transform{ v3f_const::FORWARD, quat_const::IDENTITY, v3f_const::ZERO };

		Token(SceneTree::Node) l_node_1 = l_scene_tree.add_node(l_node_1_transform, Scene_const::root_node);
		{
			assert_true(tk_v(l_node_1) == 1);

			SceneTree::NodeEntry l_node_1_value = l_scene_tree.get_node(l_node_1);
			assert_true(l_node_1_value.has_parent() == 1);
			assert_true(l_node_1_value.Element->local_transform == l_node_1_transform);
			assert_true(l_node_1_value.Element->state.haschanged_thisframe == 1);
			assert_true(l_node_1_value.Element->state.matrices_mustBe_recalculated == 1);
		}

		Token(SceneTree::Node) l_node_2 = l_scene_tree.add_node(l_node_1_transform, l_node_1);
		Token(SceneTree::Node) l_node_3 = l_scene_tree.add_node(l_node_1_transform, l_node_1);

		{
			SceneTree::NodeEntry l_node_2_value = l_scene_tree.get_node(l_node_2);
			SceneTree::NodeEntry l_node_3_value = l_scene_tree.get_node(l_node_3);
			assert_true(tk_eq(l_scene_tree.get_node_parent(l_node_2_value).Node->index, l_node_1));
			assert_true(tk_eq(l_scene_tree.get_node_parent(l_node_3_value).Node->index, l_node_1));
			assert_true(l_scene_tree.get_node_childs(l_scene_tree.get_node(l_node_1)).Size == 2);
		}


		l_scene_tree.add_node(l_node_1_transform, Scene_const::root_node);

		// set_parent
		{
			//TODO with the separation of SceneTree (Tree memory management) and Scene (event buffers and handling)
		}

		// remove node
		{
			l_scene_tree.remove_node(l_scene_tree.get_node(l_node_1));
			assert_true(l_scene_tree.get_node_childs(l_scene_tree.get_node(Scene_const::root_node)).Size == 1);
		}

		l_scene_tree.free();
	};

	// 
	inline void add_remove_component()
	{

	};
}

int main()
{
	v2::add_remove_setparent_node();
	v2::add_remove_component();
};