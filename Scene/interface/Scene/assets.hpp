#pragma once

#include "Common/Container/pool.hpp"
#include "Common/Serialization/binary.hpp"
#include "Math/vector_def.hpp"
#include "Math/quaternion_def.hpp"

struct ComponentAsset
{
	com::TPoolToken<GeneralPurposeHeapMemoryChunk> componentasset_heap_index;
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

	size_t childs_begin;
	size_t childs_end;
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