#include "Scene/Scene.cpp"

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

struct EditorSceneEventHeader
{
	unsigned int Type;
};

struct EditorSceneEventMoveNode
{
	inline static const unsigned int Type = 1;

	SceneNodeToken scene_node;
	Math::vec3f old_localposition;
	Math::vec3f new_localposition;

	inline EditorSceneEventMoveNode(SceneNodeToken& p_scene_node, Math::vec3f& p_old_local, Math::vec3f& p_new_local)
	{
		this->scene_node = p_scene_node;
		this->old_localposition = p_old_local;
		this->new_localposition = p_new_local;
	};

	inline void _do(Scene* p_scene)
	{
		SceneKernel::set_localposition(SceneKernel::resolve_node(p_scene, this->scene_node).element, p_scene, this->new_localposition);
	};

	inline void _undo(Scene* p_scene)
	{
		SceneKernel::set_localposition(SceneKernel::resolve_node(p_scene, this->scene_node).element, p_scene, this->old_localposition);
	};
};

struct EditorSceneEventRotateNode
{
	inline static const unsigned int Type = EditorSceneEventMoveNode::Type + 1;

	SceneNodeToken scene_node;
	Math::quat old_localrotation;
	Math::quat new_localrotation;

	inline EditorSceneEventRotateNode(SceneNodeToken& p_scene_node, Math::quat& p_old_local, Math::quat& p_new_local)
	{
		this->scene_node = p_scene_node;
		this->old_localrotation = p_old_local;
		this->new_localrotation = p_new_local;
	};

	inline void _do(Scene* p_scene)
	{
		SceneKernel::set_localrotation(SceneKernel::resolve_node(p_scene, this->scene_node).element, p_scene, this->new_localrotation);
	};

	inline void _undo(Scene* p_scene)
	{
		SceneKernel::set_localrotation(SceneKernel::resolve_node(p_scene, this->scene_node).element, p_scene, this->old_localrotation);
	};
};



struct EditorSceneEvent
{
	EditorSceneEventHeader header;
	char* object;

	template<class Event>
	inline void allocate(const Event& p_initial_value = Event())
	{
		this->header.Type = Event::Type;
		this->object = (char*)malloc(sizeof(Event));
		if (this->object)
		{
			memcpy(this->object, &p_initial_value, sizeof(Event));
		}
	};

	inline void free()
	{
		::free(this->object);
	};
};

struct EditorScene
{
	Scene* engine_scene;
	Scene proxy_scene;
	com::Vector<EditorSceneEvent> undo_events;

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

	inline void set_localposition(NTreeResolve<SceneNode>& p_scene_node, Math::vec3f& p_local_position)
	{
		EditorSceneEvent l_event;
		l_event.allocate(EditorSceneEventMoveNode(SceneNodeToken(p_scene_node.node->index), SceneKernel::get_localposition(p_scene_node.element), p_local_position));

		((EditorSceneEventMoveNode*)l_event.object)->_do(this->engine_scene);
		((EditorSceneEventMoveNode*)l_event.object)->_do(&this->proxy_scene);
	};

	inline void set_localrotation(NTreeResolve<SceneNode>& p_scene_node, Math::quat& p_local_rotation)
	{
		EditorSceneEvent l_event;
		l_event.allocate(EditorSceneEventRotateNode(SceneNodeToken(p_scene_node.node->index), SceneKernel::get_localrotation(p_scene_node.element), p_local_rotation));

		((EditorSceneEventRotateNode*)l_event.object)->_do(this->engine_scene);
		((EditorSceneEventRotateNode*)l_event.object)->_do(&this->proxy_scene);
	};
};

int main()
{
	Serialization::JSON::Deserializer l_deserializer;
	l_deserializer.allocate();
	l_deserializer.start();
	l_deserializer.push_field(StringSlice("field1"), StringSlice("value1"));
	l_deserializer.push_field(StringSlice("field1"), StringSlice("value1"));

	l_deserializer.start_object(StringSlice("object1"));
	l_deserializer.push_field(StringSlice("field1"), StringSlice("value1"));
	l_deserializer.end_object();

	l_deserializer.start_array(StringSlice("array1"));
	
	l_deserializer.start_object();
	l_deserializer.push_field(StringSlice("field1"), StringSlice("value1"));
	l_deserializer.end_object();

	l_deserializer.start_object();
	l_deserializer.push_field(StringSlice("field1"), StringSlice("value1"));
	l_deserializer.end_object();
	
	l_deserializer.end_array();

	l_deserializer.end();
	printf(l_deserializer.output.Memory.Memory);
	l_deserializer.free();
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

		l_editor_scene.free();
	}

	SceneKernel::free_scene(&l_scene);

}