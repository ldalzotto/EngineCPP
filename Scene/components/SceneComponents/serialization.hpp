#pragma once

#include "components.hpp"
#include "Common/Serialization/json.hpp"
#include "Common/Functional/Hash.hpp"

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

/*
template<>
struct JSONSerializer<MeshRendererAsset>
{
	inline static void serialize(Serialization::JSON::Deserializer& p_serializer, const MeshRendererAsset& p_object)
	{
		p_serializer.push_field("mesh", p_object.fov);
		p_serializer.push_field("near", p_object.near_);
		p_serializer.push_field("far", p_object.far_);
	};
};
*/

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
