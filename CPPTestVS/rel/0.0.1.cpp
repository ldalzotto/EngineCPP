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

#if 1

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
		com::Vector<char> l_scene_binary = engine_assetserver(*l_engine).get_resource("0.0.1/scene.json");
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