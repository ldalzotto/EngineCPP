// CPPTestVS.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include "Math/math.hpp"
#include "Common/Container/vector.hpp"
#include "Common/Container/pool.hpp"
#include <vector>
#include "Engine/engine.hpp"
#include "SceneComponents/components.hpp"
#include "Math/serialization.hpp"

using namespace Math;

struct TestContext
{
	com::PoolToken center_node;
	com::PoolToken moving_node;
	size_t framecount = 0;
} testContext;

void update(void* p_engine, float p_delta)
{
	EngineHandle* l_engine = (EngineHandle*)p_engine;
	SceneHandle l_scenehandle = engine_scene(*l_engine);

#if 1
	if (testContext.framecount == 0)
	{
		com::Vector<char> l_scene_binary = engine_assetserver(*l_engine).get_resource("scenes/test_scene.json");
		SceneAsset l_scene_asset = SceneSerializer::deserialize_from_binary(l_scene_binary);
		l_scenehandle.feed_with_asset(l_scene_asset);
		l_scene_binary.free();
	}
	else if ((testContext.framecount % 101) == 0)
	{
		com::Vector<NTreeResolve<SceneNode>> l_nodes = l_scenehandle.get_nodes_with_component<MeshRenderer>();
		if (l_nodes.Size > 0)
		{
			l_scenehandle.remove_component<MeshRenderer>(l_nodes[0].node->index);
		}
		l_nodes.free();
	}
#endif

#if 0
	if (testContext.framecount == 0)
	{
		{
			auto l_node = l_scenehandle.add_node(l_scenehandle.root(), Math::Transform());

			MeshRenderer l_mesh_renderer;
			l_mesh_renderer.initialize("materials/test.json", "models/16.09.obj");
			l_scenehandle.add_component<MeshRenderer>(l_node, l_mesh_renderer);

			testContext.center_node = l_node;
		}
		{
			auto l_node = l_scenehandle.add_node(testContext.center_node, Math::Transform(vec3f(0.0f, 0.0f, 0.0f), QuatConst::IDENTITY, VecConst<float>::ONE));

			MeshRenderer l_mesh_renderer;
			l_mesh_renderer.initialize("materials/test.json", "models/16.09.obj");
			l_scenehandle.add_component<MeshRenderer>(l_node, l_mesh_renderer);

			testContext.moving_node = l_node;
		}

	}

	if (testContext.framecount == 200)
	{
		l_scenehandle.remove_component<MeshRenderer>(testContext.moving_node);
	}
	else if (testContext.framecount == 300)
	{
		// l_scenehandle.free_node(testContext.center_node);
		// l_scenehandle.remove_component<MeshRenderer>(testContext.center_node);
	}
	// else if()
	{
		SceneNode* l_node = l_scenehandle.resolve_node(testContext.moving_node).element;

		l_node->set_localposition(l_node->get_localposition() + (vec3f(VecConst<float>::FORWARD) * p_delta));
		l_node->set_localrotation(mul(l_node->get_localrotation(), rotateAround(vec3f(0.0f, 1.0f, 0.0f), p_delta)));
	}

	{
		SceneNode* l_node = l_scenehandle.resolve_node(testContext.center_node).element;
		l_node->set_worldposition(l_node->get_worldposition() + (vec3f(VecConst<float>::RIGHT) * p_delta));
		l_node->set_localrotation(mul(l_node->get_localrotation(), rotateAround(vec3f(0.0f, 1.0f, 0.0f), p_delta)));
	}
#endif

	testContext.framecount += 1;
};

int main(int argc, char** argv)
{
	// mat3f l_look_at = lookAtRotation_viewmatrix<3, float>(vec3f(9.0f, 9.0f, 9.0f), vec3f(0.0f, 0.0f, 0.0f), VecConst<float>::UP);
	// quat l_q = fromAxis(l_look_at);
	// 
	// mat3f l_axis_2 = extractAxis<float>(mul(QuatConst::IDENTITY ,l_q));

	EngineHandle l_engine;
	ExternalHooks l_external_hooks;
	l_external_hooks.ext_update = update;
	l_external_hooks.closure = &l_engine;
	l_engine = engine_create(argv[0], l_external_hooks);
	engine_mainloop(l_engine);
	engine_destroy(l_engine);

}