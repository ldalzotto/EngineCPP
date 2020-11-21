#pragma once

#include "Common/Serialization/binary.hpp"
#include "Common/Serialization/json.hpp"
#include "Math/vector_def.hpp"
#include "Math/serialization.hpp"
#include "Math/quaternion_def.hpp"
#include "Common/Functional/Hash.hpp"

struct ComponentAsset
{
	com::PoolToken componentasset_heap_index;
	size_t id;
};

struct NodeAsset
{
	int parent;
	Math::vec3f local_position;
	Math::quat local_rotation;
	Math::vec3f local_scale;

	size_t components_begin;
	size_t components_end;
};

template<>
struct JSONDeserializer<NodeAsset>
{
	template<class ComponentAssetSerializer>
	static NodeAsset deserialize(Serialization::JSON::JSONObjectIterator& p_iterator, com::Vector<ComponentAsset>& p_component_assets, GeneralPurposeHeap<>& p_compoent_asset_heap)
	{
		NodeAsset l_asset;

		Serialization::JSON::JSONObjectIterator l_object_iterator;
		p_iterator.next_field("parent");
		l_asset.parent = JSONDeserializer<int>::deserialize(p_iterator);

		p_iterator.next_object("local_position", &l_object_iterator);
		l_asset.local_position = JSONDeserializer<Math::vec3f>::deserialize(l_object_iterator);

		p_iterator.next_object("local_rotation", &l_object_iterator);
		l_asset.local_rotation = JSONDeserializer<Math::quat>::deserialize(l_object_iterator);

		p_iterator.next_object("local_scale", &l_object_iterator);
		l_asset.local_scale = JSONDeserializer<Math::vec3f>::deserialize(l_object_iterator);

		p_iterator.next_array("components", &l_object_iterator);
		Serialization::JSON::JSONObjectIterator l_component_iterator;
		size_t l_componentasset_count = 0;
		while (l_object_iterator.next_array_object(&l_component_iterator))
		{
			l_component_iterator.next_field("type");
			StringSlice l_component_type = l_component_iterator.get_currentfield().value;

			Serialization::JSON::JSONObjectIterator l_component_object_iterator;
			l_component_iterator.next_object("object", &l_component_object_iterator);

			ComponentAsset l_component_asset;
			if (ComponentAssetSerializer::deserialize(l_component_type, l_component_object_iterator, p_component_assets, p_compoent_asset_heap, &l_component_asset))
			{
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
	template<class ComponentAssetSerializer>
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
			NodeAsset l_node = JSONDeserializer<NodeAsset>::deserialize<ComponentAssetSerializer>(l_node_iterator, l_asset.components, l_asset.component_asset_heap);
			l_asset.nodes.push_back(l_node);
		}

		p_iterator.free();

		return l_asset;
	};
};


struct SceneSerializer
{
	template<class ComponentAssetAllocator>
	inline static com::Vector<char> serialize_from_json_to_binary(const com::Vector<char>& p_json_scene)
	{
		String<> p_json_scene_as_string;
		p_json_scene_as_string.Memory = p_json_scene;
		Serialization::JSON::JSONObjectIterator l_scene_root_json = Serialization::JSON::StartDeserialization(p_json_scene_as_string);
		SceneAsset l_scene_asset = JSONDeserializer<SceneAsset>::deserialize<ComponentAssetAllocator>(l_scene_root_json);

		com::Vector<char> l_binary_scene;
		l_scene_asset.serialize(l_binary_scene);

		l_scene_asset.free();

		return l_binary_scene;
	};

	inline static SceneAsset deserialize_from_binary(const com::Vector<char>& p_binary_scene)
	{
		size_t l_cursor = 0;
		return SceneAsset::deserialize(l_cursor, p_binary_scene.Memory);
	};

};
