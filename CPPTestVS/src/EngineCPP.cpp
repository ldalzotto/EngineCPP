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

#if 1
struct TestContext
{
	SceneNodeToken moving_node;
	ColliderDetectorHandle collider_detector;
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
			testContext.collider_detector = l_collider_middleware->attach_collider_detector(l_nodes[0].element->scenetree_entry);
		}

	}
	else if (testContext.framecount == 20 || testContext.framecount == 40 || testContext.framecount == 60 || testContext.framecount == 80 || testContext.framecount == 100)
	{
		SceneKernel::set_localposition(testContext.moving_node, l_scenehandle, SceneKernel::get_localposition(testContext.moving_node, l_scenehandle) + Math::vec3f(1.0f, 0.0f, 0.0f));
	}
	
	/*
	if (testContext.framecount == 40)
	{
		l_collider_middleware->remove_collider_detector(testContext.collider_detector);
	}
	*/

	if (testContext.collider_detector.handle != -1 && testContext.collider_detector.get_collision_events(l_collider_middleware->collision).Size > 0)
	{
		printf("%ld", testContext.collider_detector.get_collision_events(l_collider_middleware->collision)[0].state);
		// printf("YEP\n");
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