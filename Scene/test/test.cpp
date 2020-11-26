#include "Scene/Scene.cpp"

#include "Scene/component_def.hpp"
#include "Common/Container/resource_map.hpp"
#include "Common/Container/vector.hpp"
#include "Common/Serialization/binary.hpp"
#include "Common/Serialization/json.hpp"
#include "Math/serialization.hpp"
#include "Scene/serialization.hpp"
#include "SceneComponents/components.hpp"

using namespace Math;

struct ComponentTest
{
	static const size_t Id;
	static const SceneNodeComponent_TypeInfo Type;
	int zd;
};

size_t const ComponentTest::Id = 10;
SceneNodeComponent_TypeInfo const ComponentTest::Type = SceneNodeComponent_TypeInfo(ComponentTest::Id, sizeof(ComponentTest));



void component_added_cb(void* p_clos, ComponentAddedParameter* p_par)
{

};

void component_removed_cb(void* p_clos, ComponentRemovedParameter* p_par)
{

};

#if 0
struct ComponentAsset
{
	com::PoolToken componentasset_heap_index;
	size_t type;
};

struct NodeAsset
{
	int parent;
	vec3f local_position;
	quat local_rotation;
	vec3f local_scale;

	size_t components_begin;
	size_t components_end;
};

struct MeshRendererAsset
{
	size_t mesh;
	size_t material;
};

template<>
struct JSONDeserializer<MeshRendererAsset>
{
	static MeshRendererAsset deserialize(Serialization::JSON::JSONObjectIterator& p_iterator)
	{
		MeshRendererAsset l_asset;
		p_iterator.next_field("mesh");
		String<> l_mesh_str = JSONDeserializer<String<>>::deserialize(p_iterator);
		l_asset.mesh = Hash<StringSlice>::hash(l_mesh_str.toSlice());
		l_mesh_str.free();

		p_iterator.next_field("material");
		String<> l_material_str = JSONDeserializer<String<>>::deserialize(p_iterator);
		l_asset.material = Hash<StringSlice>::hash(l_material_str.toSlice());
		l_material_str.free();

		return l_asset;
	}
};

template<>
struct JSONDeserializer<NodeAsset>
{
	static NodeAsset deserialize(Serialization::JSON::JSONObjectIterator& p_iterator, com::Vector<ComponentAsset>& p_component_assets, GeneralPurposeHeap<>& p_compoent_asset_heap)
	{
		NodeAsset l_asset;

		Serialization::JSON::JSONObjectIterator l_object_iterator;
		p_iterator.next_field("parent");
		l_asset.parent = JSONDeserializer<int>::deserialize(p_iterator);

		p_iterator.next_object("local_position", &l_object_iterator);
		l_asset.local_position = JSONDeserializer<vec3f>::deserialize(l_object_iterator);

		p_iterator.next_object("local_rotation", &l_object_iterator);
		l_asset.local_rotation = JSONDeserializer<quat>::deserialize(l_object_iterator);

		p_iterator.next_object("local_scale", &l_object_iterator);
		l_asset.local_scale = JSONDeserializer<vec3f>::deserialize(l_object_iterator);

		p_iterator.next_array("components", &l_object_iterator);
		Serialization::JSON::JSONObjectIterator l_component_iterator;
		size_t l_componentasset_count = 0;
		while (l_object_iterator.next_array_object(&l_component_iterator))
		{
			l_component_iterator.next_field("type");
			StringSlice l_component_type = l_component_iterator.get_currentfield().value;

			Serialization::JSON::JSONObjectIterator l_component_object_iterator;
			l_component_iterator.next_object("object", &l_component_object_iterator);

			if (l_component_type.equals("MeshRenderer"))
			{
				ComponentAsset l_component_asset;
				l_component_asset.type = 0;
				//Hash<StringSlice>::hash(l_component_type);

				while (!p_compoent_asset_heap.allocate_element(sizeof(MeshRendererAsset), &l_component_asset.componentasset_heap_index))
				{
					p_compoent_asset_heap.realloc(p_compoent_asset_heap.memory.Capacity == 0 ? 100 : p_compoent_asset_heap.memory.Capacity * 2);
				};

				MeshRendererAsset* l_mesh_renderer_asset = p_compoent_asset_heap.map<MeshRendererAsset>(l_component_asset.componentasset_heap_index);
				*l_mesh_renderer_asset = JSONDeserializer<MeshRendererAsset>::deserialize(l_component_object_iterator);

				p_component_assets.push_back(l_component_asset);
				l_componentasset_count += 1;
			}

			l_component_object_iterator.free();
		};

		l_asset.components_begin = p_component_assets.Size - l_componentasset_count;
		l_asset.components_end = p_component_assets.Size;

		l_component_iterator.free();
		p_iterator.free();

		return l_asset;
	};
};

struct SceneAsset
{
	com::Vector<NodeAsset> nodes;
	com::Vector<ComponentAsset> components;
	GeneralPurposeHeap<> component_asset_heap;

	inline void allocate()
	{
		this->component_asset_heap.allocate(0);
	};

	inline void free()
	{
		this->nodes.free();
		this->components.free();
		this->component_asset_heap.dispose();
	};

	inline void serialize(com::Vector<char>& p_target_data)
	{
		Serialization::Binary::serialize_vector<NodeAsset>(this->nodes, p_target_data);
		Serialization::Binary::serialize_vector<ComponentAsset>(this->components, p_target_data);
		Serialization::Binary::serialize_heap(this->component_asset_heap, p_target_data);
	};

	inline static SceneAsset deserialize(size_t& p_current_pointer, const char* p_source)
	{
		SceneAsset l_scene_asset;
		l_scene_asset.nodes = Serialization::Binary::deserialize_vector<NodeAsset>(p_current_pointer, p_source);
		l_scene_asset.components = Serialization::Binary::deserialize_vector<ComponentAsset>(p_current_pointer, p_source);
		l_scene_asset.component_asset_heap = Serialization::Binary::deserialize_heap(p_current_pointer, p_source);
		return l_scene_asset;
	}
};


template<>
struct JSONDeserializer<SceneAsset>
{
	static SceneAsset deserialize(Serialization::JSON::JSONObjectIterator& p_iterator)
	{
		SceneAsset l_asset;
		l_asset.allocate();

		p_iterator.next_field("type");

		Serialization::JSON::JSONObjectIterator l_node_array_iterator;
		Serialization::JSON::JSONObjectIterator l_node_iterator;
		p_iterator.next_array("nodes", &l_node_array_iterator);
		while (l_node_array_iterator.next_array_object(&l_node_iterator))
		{
			NodeAsset l_node = JSONDeserializer<NodeAsset>::deserialize(l_node_iterator, l_asset.components, l_asset.component_asset_heap);
			l_asset.nodes.push_back(l_node);
		}

		p_iterator.free();

		return l_asset;
	};
};

inline void scene_deserialization_test()
{
	Scene l_scene = Scene();
	l_scene.allocate(Callback<void, ComponentAddedParameter>(nullptr, component_added_cb), Callback<void, ComponentRemovedParameter>(nullptr, component_removed_cb));

	char* l_json_test = "{\"type\":\"scene\",\"nodes\":[{\"parent\":\"-1\",\"local_position\":{\"x\":\"0.1452\",\"y\":\"0.1452\",\"z\":\"0.1452\",},\"local_rotation\":{\"x\":\"0.0\",\"y\":\"0.0\",\"z\":\"0.0\",\"w\":\"1.0\"},\"local_scale\":{\"x\":\"1.0\",\"y\":\"1.0\",\"z\":\"1.0\"},\"components\":[{\"type\":\"MeshRenderer\",\"object\":{\"mesh\":\"model/16.09.obj\",\"material\":\"materials/material_test.json\"}}]},{\"parent\":\"-1\",\"local_position\":{\"x\":\"-0.1452\",\"y\":\"-0.1452\",\"z\":\"-0.1452\",},\"local_rotation\":{\"x\":\"0.0\",\"y\":\"0.0\",\"z\":\"0.0\",\"w\":\"1.0\"},\"local_scale\":{\"x\":\"1.0\",\"y\":\"1.0\",\"z\":\"1.0\"},\"components\":[{\"type\":\"MeshRenderer\",\"object\":{\"mesh\":\"model/16.09.obj\",\"material\":\"materials/material_test.json\"}}]}]}";

	String<> l_source;
	l_source.from_raw(l_json_test);
	Serialization::JSON::JSONObjectIterator l_root = Serialization::JSON::StartDeserialization(l_source);
	SceneAsset l_scene_asset = JSONDeserializer<SceneAsset>::deserialize(l_root);

	

	com::Vector<char> l_asset_binary;
	l_scene_asset.serialize(l_asset_binary);
	size_t l_curesor = 0;
	SceneAsset l_scene_asset_2 = l_scene_asset.deserialize(l_curesor, l_asset_binary.Memory);

	l_scene_asset.free();
	l_scene.free();
};
#endif


inline void scene_deserialization_test_v2()
{
	Scene l_scene = Scene();
	l_scene.allocate(Callback<void, ComponentAddedParameter>(nullptr, component_added_cb), Callback<void, ComponentRemovedParameter>(nullptr, component_removed_cb));

	char* l_json_test = "{\"type\":\"scene\",\"nodes\":[{\"parent\":\"-1\",\"local_position\":{\"x\":\"0.1452\",\"y\":\"0.1452\",\"z\":\"0.1452\",},\"local_rotation\":{\"x\":\"0.0\",\"y\":\"0.0\",\"z\":\"0.0\",\"w\":\"1.0\"},\"local_scale\":{\"x\":\"1.0\",\"y\":\"1.0\",\"z\":\"1.0\"},\"components\":[{\"type\":\"MeshRenderer\",\"object\":{\"mesh\":\"model/16.09.obj\",\"material\":\"materials/material_test.json\"}}]},{\"parent\":\"-1\",\"local_position\":{\"x\":\"-0.1452\",\"y\":\"-0.1452\",\"z\":\"-0.1452\",},\"local_rotation\":{\"x\":\"0.0\",\"y\":\"0.0\",\"z\":\"0.0\",\"w\":\"1.0\"},\"local_scale\":{\"x\":\"1.0\",\"y\":\"1.0\",\"z\":\"1.0\"},\"components\":[{\"type\":\"MeshRenderer\",\"object\":{\"mesh\":\"model/16.09.obj\",\"material\":\"materials/material_test.json\"}}]}]}";

	String<> l_source;
	l_source.from_raw(l_json_test);

	com::Vector<char> l_asset_binary = SceneSerializer::serialize_from_json_to_binary<ComponentAssetSerializer>(l_source.Memory);
	
	SceneAsset l_scene_asset = SceneSerializer::deserialize_from_binary(l_asset_binary);
	// size_t l_curesor = 0;
	// SceneAsset l_scene_asset_2 = l_scene_asset.deserialize(l_curesor, l_asset_binary.Memory);

	l_scene.feed_with_asset(l_scene_asset);

	// l_scene_asset.free();
	l_asset_binary.free();
	l_scene.free();
};

void main()
{
//	scene_deserialization_test();
	scene_deserialization_test_v2();
}


#if 0
com::PoolToken l_root_token = l_scene.allocate_node(Math::Transform(vec3f(1.0f, 2.0f, 3.0f), quat(1.0f, 2.0f, 3.0f, 4.0f), vec3f(4.0f, 5.0f, 6.0f)));
com::PoolToken l_child_token = l_scene.allocate_node(Math::Transform(vec3f(1.0f, 2.0f, 3.0f), quat(1.0f, 2.0f, 3.0f, 4.0f), vec3f(4.0f, 5.0f, 6.0f)));

NTreeResolve<SceneNode> l_root = l_scene.resolve_node(l_root_token);
NTreeResolve<SceneNode> l_child = l_scene.resolve_node(l_child_token);

l_scene.root().element->addchild(l_root_token);
l_root.element->addchild(l_child_token);
mat4f l_localtoworld = l_root.element->get_localtoworld();
mat4f l_worldtolocal = l_root.element->get_worldtolocal();

mat4f l_child_localtoworld = l_child.element->get_localtoworld();
mat4f l_child_worldtolocal = l_child.element->get_worldtolocal();

vec3f l_child_worldposition = l_child.element->get_worldposition();
quat l_child_worldrotation = l_child.element->get_worldrotation();
vec3f l_child_scalefactor = l_child.element->get_worldscalefactor();

l_child.element->set_worldposition(vec3f(0.0f, 0.0f, 0.0f));
l_child.element->set_worldrotation(quat(0.0f, 0.0f, 0.0f, 1.0f));
l_child.element->set_worldscale(vec3f(1.0f, 1.0f, 1.0f));

l_child_worldposition = l_child.element->get_worldposition();
l_child_worldrotation = l_child.element->get_worldrotation();
l_child_scalefactor = l_child.element->get_worldscalefactor();



SceneHandle l_scene_handle;
l_scene_handle.handle = &l_scene;

com::PoolToken l_comp = l_scene_handle.add_component<ComponentTest>(l_child_token);
ComponentTest* l_c = l_scene_handle.resolve_component(l_comp);
// com::PoolToken<ComponentTest> l_comp = l_scene_handle.allocate_component<ComponentTest>();

#endif