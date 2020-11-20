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

	inline static MaterialAsset deserializeJSON(char* p_file)
	{
		MaterialAsset l_asset;

		Serialization::JSON::DeserializeIterator l_json_deserializer;
		String<> l_file;
		l_file.from_raw(p_file);
		l_json_deserializer.start(l_file);
		{
			l_json_deserializer.next_field("type");
			l_json_deserializer.next_field("vertex");
			l_asset.shader.vertex = Hash<StringSlice>::hash(l_json_deserializer.stack_fields[l_json_deserializer.current_field].value);
			l_json_deserializer.next_field("fragment");
			l_asset.shader.fragment = Hash<StringSlice>::hash(l_json_deserializer.stack_fields[l_json_deserializer.current_field].value);
			l_json_deserializer.next_field("texture");
			l_asset.texture = Hash<StringSlice>::hash(l_json_deserializer.stack_fields[l_json_deserializer.current_field].value);
		}
		l_json_deserializer.free();

		return l_asset;
	};

	inline void serialize(com::Vector<char>& out_target)
	{
		size_t l_current_pointer = 0;
		Serialization::Binary::serialize_field<size_t>(l_current_pointer, (const char*)this, out_target);
		Serialization::Binary::serialize_field<size_t>(l_current_pointer, (const char*)this, out_target);
		Serialization::Binary::serialize_field<size_t>(l_current_pointer, (const char*)this, out_target);
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
