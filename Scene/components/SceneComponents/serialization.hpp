#pragma once

#include "components.hpp"
#include "Common/Serialization/json.hpp"
#include "Common/Functional/Hash.hpp"

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
	};
};


template<>
struct JSONDeserializer<CameraAsset>
{
	static CameraAsset deserialize(Serialization::JSON::JSONObjectIterator& p_iterator)
	{
		CameraAsset l_asset;
		p_iterator.next_field("fov");
		l_asset.fov = JSONDeserializer<float>::deserialize(p_iterator);
		p_iterator.next_field("near");
		l_asset.near_ = JSONDeserializer<float>::deserialize(p_iterator);
		p_iterator.next_field("far");
		l_asset.far_ = JSONDeserializer<float>::deserialize(p_iterator);

		p_iterator.free();
		return l_asset;
	};
};

