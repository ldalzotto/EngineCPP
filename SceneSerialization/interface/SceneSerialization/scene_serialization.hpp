#pragma once

#include "SceneComponents/components.hpp"
#include "Scene/assets.hpp"
#include "Common/Serialization/json.hpp"
#include "Common/Functional/Hash.hpp"
#include "Common/Serialization/binary.hpp"
#include "Math/vector_def.hpp"
#include "Math/serialization.hpp"
#include "Math/quaternion_def.hpp"
#include "AssetServer/asset_server.hpp"

template<>
struct JSONDeserializer<MeshRendererAsset>
{
	inline static MeshRendererAsset deserialize(Deserialization::JSON::JSONObjectIterator& p_iterator)
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
	};
};

template<>
struct JSONSerializer<MeshRendererAsset>
{
	inline static void serialize(Serialization::JSON::Deserializer& p_serializer, const MeshRendererAsset& p_object, AssetServerHandle& p_asset_server)
	{
		String<> l_path_tmp;
		l_path_tmp = p_asset_server.get_path_from_resourcehash(p_object.mesh);
		p_serializer.push_field("mesh", l_path_tmp.toSlice());
		l_path_tmp.free();
		l_path_tmp = p_asset_server.get_path_from_resourcehash(p_object.material);
		p_serializer.push_field("material", l_path_tmp.toSlice());
		l_path_tmp.free();
	};
};

template<>
struct JSONDeserializer<CameraAsset>
{
	inline static CameraAsset deserialize(Deserialization::JSON::JSONObjectIterator& p_iterator)
	{
		CameraAsset l_asset;
		p_iterator.next_field("fov", &l_asset.fov);
		p_iterator.next_field("near", &l_asset.near_);
		p_iterator.next_field("far", &l_asset.far_);

		p_iterator.free();
		return l_asset;
	};
};

template<>
struct JSONSerializer<CameraAsset>
{
	inline static void serialize(Serialization::JSON::Deserializer& p_serializer, const CameraAsset& p_object)
	{
		p_serializer.push_field("fov", p_object.fov);
		p_serializer.push_field("near", p_object.near_);
		p_serializer.push_field("far", p_object.far_);
	};
};





template<>
struct JSONDeserializer<NodeAsset>
{
	template<class ComponentAssetSerializer>
	inline static void deserialize(Deserialization::JSON::JSONObjectIterator& p_iterator, size_t p_parent_nodeasset_index, com::Vector<NodeAsset>& p_node_assets,
		com::Vector<ComponentAsset>& p_component_assets, GeneralPurposeHeap<>& p_compoent_asset_heap)
	{
		NodeAsset l_asset;
		l_asset.parent = p_parent_nodeasset_index;

		Deserialization::JSON::JSONObjectIterator l_object_iterator;

		p_iterator.next_object("local_position", &l_object_iterator);
		l_asset.local_position = JSONDeserializer<Math::vec3f>::deserialize(l_object_iterator);

		p_iterator.next_object("local_rotation", &l_object_iterator);
		l_asset.local_rotation = JSONDeserializer<Math::quat>::deserialize(l_object_iterator);

		p_iterator.next_object("local_scale", &l_object_iterator);
		l_asset.local_scale = JSONDeserializer<Math::vec3f>::deserialize(l_object_iterator);

		p_iterator.next_array("components", &l_object_iterator);
		Deserialization::JSON::JSONObjectIterator l_component_iterator;
		size_t l_componentasset_count = 0;
		while (l_object_iterator.next_array_object(&l_component_iterator))
		{
			l_component_iterator.next_field("type");
			StringSlice l_component_type = l_component_iterator.get_currentfield().value;

			Deserialization::JSON::JSONObjectIterator l_component_object_iterator;
			l_component_iterator.next_object("object", &l_component_object_iterator);

			ComponentAsset l_component_asset;
			if (ComponentAssetSerializer::deserializeJSON(l_component_type, l_component_object_iterator, p_component_assets, p_compoent_asset_heap, &l_component_asset))
			{
				p_component_assets.push_back(l_component_asset);
				l_componentasset_count += 1;
			}

			l_component_object_iterator.free();
		};

		l_asset.components_begin = p_component_assets.Size - l_componentasset_count;
		l_asset.components_end = p_component_assets.Size;

		//childs
		p_node_assets.push_back(l_asset);
		size_t l_node_insert_index = p_node_assets.Size - 1;

		if (p_iterator.next_array("childs", &l_object_iterator))
		{
			Deserialization::JSON::JSONObjectIterator l_childs_iterator;
			while (l_object_iterator.next_array_object(&l_childs_iterator))
			{
				deserialize<ComponentAssetSerializer>(l_childs_iterator, l_node_insert_index, p_node_assets, p_component_assets, p_compoent_asset_heap);
			}
		}

		p_node_assets[l_node_insert_index].childs_begin = l_node_insert_index + 1;
		p_node_assets[l_node_insert_index].childs_end = p_node_assets.Size;

		p_iterator.free();
	};
};

template<>
struct JSONSerializer<NodeAsset>
{
	template<class ComponentAssetSerializer>
	inline static void serialize(Serialization::JSON::Deserializer& p_json_deserializer, NodeAsset& p_node_asset, SceneAsset& p_scene_asset, AssetServerHandle p_asset_server)
	{
		p_json_deserializer.start_object("local_position");
		JSONSerializer<Math::vec3f>::serialize(p_json_deserializer, p_node_asset.local_position);
		p_json_deserializer.end_object();
		p_json_deserializer.start_object("local_rotation");
		JSONSerializer<Math::quat>::serialize(p_json_deserializer, p_node_asset.local_rotation);
		p_json_deserializer.end_object();
		p_json_deserializer.start_object("local_scale");
		JSONSerializer<Math::vec3f>::serialize(p_json_deserializer, p_node_asset.local_scale);
		p_json_deserializer.end_object();
			
		p_json_deserializer.start_array("components");
		for (size_t l_component_index = p_node_asset.components_begin; l_component_index < p_node_asset.components_end; l_component_index++)
		{
			p_json_deserializer.start_object();
			ComponentAssetSerializer::serializeJSON(p_scene_asset.components[l_component_index], p_json_deserializer, p_scene_asset.component_asset_heap, p_asset_server);
			p_json_deserializer.end_object();
		}
		p_json_deserializer.end_array();

		p_json_deserializer.start_array("childs");
		for (size_t l_child_index = p_node_asset.childs_begin; l_child_index < p_node_asset.childs_end; l_child_index++)
		{
			p_json_deserializer.start_object();
			serialize<ComponentAssetSerializer>(p_json_deserializer, p_scene_asset.nodes[l_child_index], p_scene_asset, p_asset_server);
			p_json_deserializer.end_object();
		}
		p_json_deserializer.end_array();
	}
};

template<>
struct JSONDeserializer<SceneAsset>
{
	template<class ComponentAssetSerializer>
	inline static SceneAsset deserialize(Deserialization::JSON::JSONObjectIterator& p_iterator)
	{
		SceneAsset l_asset;
		l_asset.allocate();

		p_iterator.next_field("type");

		Deserialization::JSON::JSONObjectIterator l_node_array_iterator;
		Deserialization::JSON::JSONObjectIterator l_node_iterator;
		p_iterator.next_array("nodes", &l_node_array_iterator);
		while (l_node_array_iterator.next_array_object(&l_node_iterator))
		{
			JSONDeserializer<NodeAsset>::deserialize<ComponentAssetSerializer>(l_node_iterator, -1, l_asset.nodes, l_asset.components, l_asset.component_asset_heap);
			// l_asset.nodes.push_back(l_node);
		}

		p_iterator.free();

		return l_asset;
	};
};


template<>
struct JSONSerializer<SceneAsset>
{
	template<class ComponentAssetSerializer>
	inline static void serialize(Serialization::JSON::Deserializer& p_json_deserializer, SceneAsset& p_scene_asset, AssetServerHandle p_asset_server)
	{
		p_json_deserializer.start();
		p_json_deserializer.push_field("type", StringSlice("scene"));
		p_json_deserializer.start_array("nodes");
		
		for (size_t i = 0; i < p_scene_asset.nodes.Size; i++)
		{
			if (p_scene_asset.nodes[i].parent == -1)
			{
				p_json_deserializer.start_object();
				JSONSerializer<NodeAsset>::serialize<ComponentAssetSerializer>(p_json_deserializer, p_scene_asset.nodes[i], p_scene_asset, p_asset_server);
				p_json_deserializer.end_object();
			}
		}

		p_json_deserializer.end_array();
		p_json_deserializer.end();
	};
};

struct SceneSerializer
{
	template<class ComponentAssetSerializer>
	inline static com::Vector<char> serialize_from_json_to_binary(const com::Vector<char>& p_json_scene)
	{
		String<> p_json_scene_as_string;
		p_json_scene_as_string.Memory = p_json_scene;
		Deserialization::JSON::JSONObjectIterator l_scene_root_json = Deserialization::JSON::StartDeserialization(p_json_scene_as_string);
		SceneAsset l_scene_asset = JSONDeserializer<SceneAsset>::deserialize<ComponentAssetSerializer>(l_scene_root_json);

		com::Vector<char> l_binary_scene;
		l_scene_asset.serialize(l_binary_scene);

		l_scene_asset.free();

		return l_binary_scene;
	};

	template<class ComponentAssetSerializer>
	inline static com::Vector<char> serialize_to_json(SceneAsset& p_scene_asset, AssetServerHandle p_asset_server)
	{
		Serialization::JSON::Deserializer l_json_seralizer;
		l_json_seralizer.allocate();
		JSONSerializer<SceneAsset>::serialize<ComponentAssetSerializer>(l_json_seralizer, p_scene_asset, p_asset_server);
		return l_json_seralizer.output.Memory;
	};

	inline static SceneAsset deserialize_from_binary(const com::Vector<char>& p_binary_scene)
	{
		size_t l_cursor = 0;
		return SceneAsset::deserialize(l_cursor, p_binary_scene.Memory);
	};

};
