#pragma once

#include "Common/Container/vector.hpp"
#include "Common/Functional/hash.hpp"
#include "Common/Serialization/json.hpp"
#include "Common/Serialization/binary.hpp"

struct MaterialAsset
{
	struct Shader
	{
		size_t vertex;
		size_t fragment;
	} shader;

	size_t texture;

	inline void serialize(com::Vector<char>& out_target)
	{
		Serialization::Binary::serialize_field<size_t>(&this->shader.vertex, out_target);
		Serialization::Binary::serialize_field<size_t>(&this->shader.fragment, out_target);
		Serialization::Binary::serialize_field<size_t>(&this->texture, out_target);
	};

	inline static MaterialAsset deserialize(const char* p_source)
	{
		MaterialAsset l_resource;
		size_t l_current_pointer = 0;
		l_resource.shader.vertex = *Serialization::Binary::deserialize_field<size_t>(l_current_pointer, p_source);
		l_resource.shader.fragment = *Serialization::Binary::deserialize_field<size_t>(l_current_pointer, p_source);
		l_resource.texture = *Serialization::Binary::deserialize_field<size_t>(l_current_pointer, p_source);
		return l_resource;
	};
};

template<>
struct JSONDeserializer<MaterialAsset>
{
	inline static MaterialAsset deserialize(Serialization::JSON::JSONObjectIterator& p_iterator)
	{
		MaterialAsset l_asset;

		p_iterator.next_field("type");
		p_iterator.next_field("vertex");
		l_asset.shader.vertex = Hash<StringSlice>::hash(p_iterator.get_currentfield().value);
		p_iterator.next_field("fragment");
		l_asset.shader.fragment = Hash<StringSlice>::hash(p_iterator.get_currentfield().value);
		p_iterator.next_field("texture");
		l_asset.texture = Hash<StringSlice>::hash(p_iterator.get_currentfield().value);

		p_iterator.free();

		return l_asset;
	}
};
