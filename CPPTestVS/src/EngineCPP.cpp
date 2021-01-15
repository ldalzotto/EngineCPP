// CPPTestVS.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include "Math/math.hpp"
#include "Common/Container/vector.hpp"
#include "Common/Container/pool.hpp"
#include "Engine/engine.hpp"
#include "SceneComponents/components.hpp"
#include "Math/serialization.hpp"
#include "Input/input.hpp"
#include "Middleware/render_middleware.hpp"
#include "Scene/kernel/scene.hpp"
#include "SceneSerialization/scene_serialization.hpp"
#include "Common/Clock/clock.hpp"

const SceneNodeComponent_TypeInfo test_component_info = SceneNodeComponent_TypeInfo(3, 28);

using namespace Math;

//scenes/test_scene.json
#if 0

struct TestContext
{
	com::PoolToken center_node;
	com::PoolToken moving_node;
	com::Vector<SceneNodeToken> moving_nodes;
	size_t framecount = 0;
} testContext;

void update(void* p_engine, float p_delta)
{
	EngineHandle* l_engine = (EngineHandle*)p_engine;
	Scene* l_scenehandle = engine_scene(*l_engine);
	InputHandle l_input = engine_input(*l_engine);


#if 1
	if (testContext.framecount == 0)
	{
		com::Vector<char> l_scene_binary = engine_assetserver(*l_engine).get_resource("scenes/test_scene.json");
		SceneAsset l_scene_asset = SceneSerializer2::Binary_to_SceneAsset(l_scene_binary);
		SceneKernel::feed_with_asset(l_scenehandle, l_scene_asset);
		// 		*l_scenehandle = SceneKernel::clone(l_scenehandle);

		l_scene_binary.free();

		com::Vector<NTreeResolve<SceneNode>> l_nodes = SceneKernel::get_nodes_with_component<MeshRenderer>(l_scenehandle);
		{
			for (size_t i = 0; i < l_nodes.Size; i++)
			{
				testContext.moving_nodes.push_back(SceneNodeToken(l_nodes[i].node->index.val));
			}
		}
		l_nodes.free();
	}
	/*
	else if ((testContext.framecount % 2) == 0)
	{
		testContext.center_node = com::PoolToken(9);
		SceneKernel::remove_component(l_scenehandle, SceneNodeToken(testContext.center_node.val), test_component_info);
	}
	else
	{

		testContext.center_node = com::PoolToken(9);
		SceneKernel::add_component(l_scenehandle, SceneNodeToken(testContext.center_node.val), test_component_info, (void*)&test_component_info);
	}
	*/

	if (l_input.get_state(InputKey::InputKey_B, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
	{
		com::Vector<NTreeResolve<SceneNode>> l_nodes = SceneKernel::get_nodes_with_component<MeshRenderer>(l_scenehandle);
		MeshRenderer& l_mesh_renderer = SceneKernel::get_component<MeshRenderer>(l_scenehandle, SceneNodeToken(l_nodes[0].node->index.val));
		engine_render_middleware(*l_engine)->set_material(l_mesh_renderer, Hash<StringSlice>::hash(StringSlice("materials/editor_selected.json")));
	}
	else if (l_input.get_state(InputKey::InputKey_V, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
	{
		com::Vector<NTreeResolve<SceneNode>> l_nodes = SceneKernel::get_nodes_with_component<MeshRenderer>(l_scenehandle);
		MeshRenderer& l_mesh_renderer = SceneKernel::get_component<MeshRenderer>(l_scenehandle, SceneNodeToken(l_nodes[0].node->index.val));
		engine_render_middleware(*l_engine)->set_material(l_mesh_renderer, Hash<StringSlice>::hash(StringSlice("materials/test.json")));
		engine_render_middleware(*l_engine)->set_mesh(l_mesh_renderer, Hash<StringSlice>::hash(StringSlice("models/16.09.obj")));
	}
	else if (l_input.get_state(InputKey::InputKey_C, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
	{
		com::Vector<NTreeResolve<SceneNode>> l_nodes = SceneKernel::get_nodes_with_component<MeshRenderer>(l_scenehandle);
		SceneKernel::remove_node(l_scenehandle, l_nodes[0].node->index.to_pooltoken());
	}

	for (size_t i = 0; i < testContext.moving_nodes.Size; i++)
	{
		SceneNode* l_node = SceneKernel::resolve_node(l_scenehandle, testContext.moving_nodes[i]).element;
		SceneKernel::set_localrotation(l_node, l_scenehandle, mul(SceneKernel::get_localrotation(l_node), rotateAround(vec3f(0.0f, 1.0f, 0.0f), p_delta)));
	}


#endif

	testContext.framecount += 1;
};

#endif

#if 0

#include <stdlib.h>

struct TestContext
{
	bool rotation_enabled = false;
	bool rotation_global_enabled = false;
	size_t framecount = 0;
} testContext;

void update(void* p_engine, float p_delta)
{
	EngineHandle* l_engine = (EngineHandle*)p_engine;
	Scene* l_scenehandle = engine_scene(*l_engine);
	InputHandle l_input = engine_input(*l_engine);
	Clock* l_clock = engine_clock(*l_engine);


	if (testContext.framecount == 0)
	{
		com::Vector<char> l_scene_binary = engine_assetserver(*l_engine).get_resource("scenes/test_scene.json");
		SceneAsset l_scene_asset = SceneSerializer2::Binary_to_SceneAsset(l_scene_binary);
		SceneKernel::feed_with_asset(l_scenehandle, l_scene_asset);
		l_scene_binary.free();
	}
	/*
	else if ((testContext.framecount % 2) == 0)
	{
		testContext.center_node = com::PoolToken(9);
		SceneKernel::remove_component(l_scenehandle, SceneNodeToken(testContext.center_node.val), test_component_info);
	}
	else
	{

		testContext.center_node = com::PoolToken(9);
		SceneKernel::add_component(l_scenehandle, SceneNodeToken(testContext.center_node.val), test_component_info, (void*)&test_component_info);
	}
	*/

	if (l_input.get_state(InputKey::InputKey_R, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
	{
		testContext.rotation_enabled = !testContext.rotation_enabled;
	}
	if (l_input.get_state(InputKey::InputKey_G, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
	{
		testContext.rotation_global_enabled = !testContext.rotation_global_enabled;
	}
	if (l_input.get_state(InputKey::InputKey_T, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
	{
		com::Vector<NTreeResolve<SceneNode>> l_nodes = SceneKernel::get_nodes_with_component<MeshRenderer>(l_scenehandle);
		for (size_t i = 0; i < l_nodes.Size; i++)
		{
			MeshRenderer& l_mesh_renderer = SceneKernel::get_component<MeshRenderer>(l_scenehandle, SceneNodeToken(l_nodes[i].node->index.val));
			{
				String<> l_material; l_material.allocate(0);
				l_material.append("0.0.1/block_1x1_ALT_");
				l_material.append((rand() % 3) + 1);
				l_material.append(".mat.json");
				engine_render_middleware(*l_engine)->set_material(l_mesh_renderer, Hash<StringSlice>::hash(l_material.toSlice()));
				l_material.free();
			}
		}
		l_nodes.free();
	}
	if (l_input.get_state(InputKey::InputKey_M, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
	{
		com::Vector<NTreeResolve<SceneNode>> l_nodes = SceneKernel::get_nodes_with_component<MeshRenderer>(l_scenehandle);
		for (size_t i = 0; i < l_nodes.Size; i++)
		{
			MeshRenderer& l_mesh_renderer = SceneKernel::get_component<MeshRenderer>(l_scenehandle, SceneNodeToken(l_nodes[i].node->index.val));
			{
				String<> l_mesh; l_mesh.allocate(0);
				l_mesh.append("0.0.1/block_1x1_ALT_");
				l_mesh.append((rand() % 3) + 1);
				l_mesh.append(".obj");
				engine_render_middleware(*l_engine)->set_mesh(l_mesh_renderer, Hash<StringSlice>::hash(l_mesh.toSlice()));
				l_mesh.free();
			}
		}
		l_nodes.free();
	}

	if (testContext.rotation_enabled)
	{
		com::Vector<NTreeResolve<SceneNode>> l_nodes = SceneKernel::get_nodes_with_component<MeshRenderer>(l_scenehandle);
		for (size_t i = 0; i < l_nodes.Size; i++)
		{
			SceneKernel::set_localrotation(l_nodes[i].element, l_scenehandle, Math::mul(SceneKernel::get_localrotation(l_nodes[i]), Math::rotateAround(Math::VecConst<float>::UP, l_clock->deltatime)));
		}
		l_nodes.free();
	}
	if (testContext.rotation_global_enabled)
	{
		com::Vector<NTreeResolve<SceneNode>> l_nodes = SceneKernel::get_nodes_with_component<MeshRenderer>(l_scenehandle);

		NTreeResolve<SceneNode> l_root = SceneKernel::resolve_node(l_scenehandle, SceneNodeToken(l_nodes[0].node->parent.val));

		SceneKernel::set_localrotation(l_root.element, l_scenehandle, Math::mul(SceneKernel::get_localrotation(l_root), Math::rotateAround(Math::VecConst<float>::UP, l_clock->deltatime)));

		l_nodes.free();
	}



	testContext.framecount += 1;
};

#endif

#if 1
struct TestContext
{
	SceneNodeToken moving_node;
	SceneNodeToken other_node;
	ColliderDetectorHandle collider_detector_0;
	ColliderDetectorHandle collider_detector_1;
	size_t framecount = 0;
} testContext;

void update(void* p_engine, float p_delta)
{
	EngineHandle* l_engine = (EngineHandle*)p_engine;
	Scene* l_scenehandle = engine_scene(*l_engine);
	InputHandle l_input = engine_input(*l_engine);
	CollisionMiddlewareHandle l_collider_middleware = engine_collider_middleware(*l_engine);

	if (testContext.framecount == 0)
	{
		com::Vector<char> l_scene_binary = engine_assetserver(*l_engine).get_resource("scenes/box_collider_scene.json");
		SceneAsset l_scene_asset = SceneSerializer2::Binary_to_SceneAsset(l_scene_binary);
		SceneKernel::feed_with_asset(l_scenehandle, l_scene_asset);
		l_scene_binary.free();

		com::Vector<NTreeResolve<SceneNode>> l_nodes = SceneKernel::get_nodes_with_component<BoxCollider>(l_scenehandle);
		if (l_nodes.Size > 0)
		{
			// BoxCollider& l_collider = SceneKernel::get_component<BoxCollider>(l_scenehandle, l_nodes[0]);
			// BoxColliderHandle l_box_collider = l_collider_middleware->get_collider(l_nodes[0].element->scenetree_entry);
			testContext.moving_node = SceneNodeToken(l_nodes[0].node->parent.val);
			testContext.other_node = SceneNodeToken(l_nodes[1].node->parent.val);
			testContext.collider_detector_0 = l_collider_middleware->attach_collider_detector(l_nodes[0].element->scenetree_entry);
			testContext.collider_detector_1 = l_collider_middleware->attach_collider_detector(l_nodes[1].element->scenetree_entry);
		}

	}

	/*
	SceneNode* l_node = SceneKernel::resolve_node(l_scenehandle, testContext.moving_node).element;
	SceneKernel::set_localposition(l_node, l_scenehandle, Math::vec3f(-0.2f, 0.0f, 0.0f));
	SceneKernel::set_worldrotation(l_node, l_scenehandle,
		Math::mul(SceneKernel::get_localrotation(l_node), Math::rotateAround(Math::VecConst<float>::UP, p_delta)));
		*/
	
	else if (testContext.framecount == 20 || testContext.framecount == 40 || testContext.framecount == 60 || testContext.framecount == 80 || testContext.framecount == 100)
	{
		SceneKernel::set_localposition(testContext.moving_node, l_scenehandle, SceneKernel::get_localposition(testContext.moving_node, l_scenehandle) + Math::vec3f(1.0f, 0.0f, 0.0f));
	}
	
	if (testContext.framecount == 80)
	{
		//TODO
		// SceneKernel::remove_node(l_scenehandle, testContext.other_node);
		// SceneKernel::remove_node(l_scenehandle, testContext.moving_node);
		// l_collider_middleware->remove_collider_detector(testContext.collider_detector_1);
	}

	/*
	if (testContext.framecount == 40)
	{
		l_collider_middleware->remove_collider_detector(testContext.collider_detector);
	}
	*/


	printf("Trigger events : ");
	Slice<Trigger::Event> l_0_trigger_events = testContext.collider_detector_0.get_collision_events(l_collider_middleware->collision);
	if (l_0_trigger_events.Size > 0)
	{
		printf("collider : %lld", testContext.collider_detector_0.collider.handle);
		for (size_t i = 0; i < l_0_trigger_events.Size; i++)
		{
			printf(" to : %lld, ", l_0_trigger_events.get(i).other.handle);
			printf("%ld;", l_0_trigger_events.get(i).state);
		}
	};

	Slice<Trigger::Event> l_1_trigger_events = testContext.collider_detector_1.get_collision_events(l_collider_middleware->collision);
	if (l_1_trigger_events.Size > 0)
	{
		printf("collider : %lld", testContext.collider_detector_1.collider.handle);
		for (size_t i = 0; i < l_1_trigger_events.Size; i++)
		{
			printf(" to : %lld, ", l_1_trigger_events.get(i).other.handle);
			printf("%ld;", l_1_trigger_events.get(i).state);
		}
		printf("\n");
	};

	testContext.framecount += 1;
}
#endif

int main(int argc, char** argv)
{
	EngineHandle l_engine;
	ExternalHooks l_external_hooks;
	l_external_hooks.ext_update = update;
	l_external_hooks.closure = &l_engine;
	l_engine = engine_create(argv[0], l_external_hooks);
	engine_mainloop(l_engine);
	engine_exit(l_engine);

}