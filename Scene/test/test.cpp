#include "../Scene/Scene.cpp"

#include "Scene/component_def.hpp"
#include "Scene/kernel/scene.hpp"
#include "Common/Container/resource_map.hpp"
#include "Common/Container/vector.hpp"
#include "Common/Serialization/binary.hpp"
#include "Common/Serialization/json.hpp"
#include "Math/serialization.hpp"
#include "Scene/assets.hpp"
#include "Scene/kernel/scene.hpp"
#include "SceneComponents/components.hpp"

using namespace Math;

struct ComponentTest
{
	static const size_t Id;
	static const SceneNodeComponent_TypeInfo Type;
	int a = 1;
	int b = 2;
};

size_t const ComponentTest::Id = 10;
SceneNodeComponent_TypeInfo const ComponentTest::Type = SceneNodeComponent_TypeInfo(ComponentTest::Id, sizeof(ComponentTest));



inline void component_added_cb(void* p_clos, ComponentAddedParameter* p_par)
{

};

inline void component_removed_cb(void* p_clos, ComponentRemovedParameter* p_par)
{

};

inline void component_asset_push_cb(void* p_clos, ComponentAssetPushParameter* p_par)
{
	if (p_par->component_asset->id == ComponentTest::Id)
	{
		SceneKernel::add_component<ComponentTest>((Scene*)p_par->scene, p_par->node, *(ComponentTest*)p_par->component_asset_object);
	}

};

#include <cstdlib>
#include "Common/Clock/clock.hpp"
int main()
{
	
	Scene l_scene;
	SceneKernel::allocate_scene(&l_scene,
		Callback<void, ComponentAddedParameter>(nullptr, component_added_cb),
		Callback<void, ComponentRemovedParameter>(nullptr, component_removed_cb),
		Callback<void, ComponentAssetPushParameter>(nullptr, component_asset_push_cb)
	);

	SceneNodeToken l_node = SceneKernel::add_node(&l_scene, Math::Transform());
	
	for (size_t i = 0; i < 100; i++)
	{
		SceneKernel::add_component<ComponentTest>(&l_scene, l_node, ComponentTest());
		SceneKernel::add_component<ComponentTest>(&l_scene, l_node, ComponentTest());
		SceneKernel::add_component<ComponentTest>(&l_scene, l_node, ComponentTest());
		SceneKernel::add_component<ComponentTest>(&l_scene, l_node, ComponentTest());
		SceneKernel::add_component<ComponentTest>(&l_scene, l_node, ComponentTest());
		SceneKernel::add_component<ComponentTest>(&l_scene, l_node, ComponentTest());
		SceneKernel::add_component<ComponentTest>(&l_scene, l_node, ComponentTest());
		SceneKernel::add_component<ComponentTest>(&l_scene, l_node, ComponentTest());
		SceneKernel::add_component<ComponentTest>(&l_scene, l_node, ComponentTest());

		SceneKernel::remove_component<ComponentTest>(&l_scene, l_node);
		SceneKernel::remove_component<ComponentTest>(&l_scene, l_node);
		SceneKernel::remove_component<ComponentTest>(&l_scene, l_node);
		SceneKernel::remove_component<ComponentTest>(&l_scene, l_node);
		SceneKernel::remove_component<ComponentTest>(&l_scene, l_node);
		SceneKernel::remove_component<ComponentTest>(&l_scene, l_node);
		SceneKernel::remove_component<ComponentTest>(&l_scene, l_node);
		SceneKernel::remove_component<ComponentTest>(&l_scene, l_node);
		SceneKernel::remove_component<ComponentTest>(&l_scene, l_node);
	}
	
	SceneKernel::free_scene(&l_scene);
}