#include "Scene/Scene.cpp"

#include "Scene/component_def.hpp"
#include "Common/Container/resource_map.hpp"
#include "Common/Container/vector.hpp"
#include "Common/Serialization/binary.hpp"
#include "Common/Serialization/json.hpp"
#include "Math/serialization.hpp"
#include "Scene/serialization.hpp"
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

struct EditorScene
{
	Scene* engine_scene;
	Scene proxy_scene;

	inline void allocate(Scene* p_engine_scene)
	{
		this->engine_scene = p_engine_scene;
		this->proxy_scene = SceneKernel::clone(p_engine_scene);
	};

	inline void free()
	{
		SceneKernel::free_scene(&this->proxy_scene);
		this->engine_scene = nullptr;
	};

};

int main()
{

	Scene l_scene;
	SceneKernel::allocate_scene(&l_scene,
		Callback<void, ComponentAddedParameter>(nullptr, component_added_cb),
		Callback<void, ComponentRemovedParameter>(nullptr, component_removed_cb),
		Callback<void, ComponentAssetPushParameter>(nullptr, component_asset_push_cb)
	);

	SceneAsset l_scene_asset;
	l_scene_asset.allocate();
	l_scene_asset.component_asset_heap.allocate(200);
	NodeAsset l_node;
	l_node.parent = -1;
	l_node.components_begin = 0;
	l_node.components_end = 1;
	l_node.local_position = Math::vec3f(1.0f, 2.0f, 3.0f);
	l_node.local_rotation = Math::quat(4.0f, 5.0f, 6.0f, 7.0f);
	l_node.local_scale = Math::vec3f(1.0f, 2.0f, 3.0f);
	l_scene_asset.nodes.push_back(l_node);

	com::TPoolToken<GeneralPurposeHeapMemoryChunk> l_allocated_component;
	l_scene_asset.component_asset_heap.allocate_element(sizeof(ComponentTest), &l_allocated_component);
	ComponentAsset l_component_asset;
	l_component_asset.id = ComponentTest::Id;
	l_component_asset.componentasset_heap_index = l_allocated_component.Index;

	l_scene_asset.components.push_back(l_component_asset);

	{
		SceneKernel::feed_with_asset(&l_scene, l_scene_asset);
		l_scene_asset.free();
		
		EditorScene l_editor_scene;
		l_editor_scene.allocate(&l_scene);



		// l_editor_scene.free();
	}

	SceneKernel::free_scene(&l_scene);

}