#pragma once

#include "Common/Container/vector.hpp"
#include "Common/Functional/hash.hpp"
#include "Common/Serialization/json.hpp"
#include "Common/Serialization/binary.hpp"
#include "Math/serialization.hpp"

struct ShaderCompareOp
{
	enum class Type
	{
		Never = 0,
		Less = 1,
		Equal = 2,
		LessOrEqual = 3,
		Greater = 4,
		NotEqual = 5,
		GreaterOrEqual = 6,
		Always = 7,
		Invalid = 8
	};

	inline static const size_t Never_Hash = 210683813157;	// Hash<StringSlice>::hash(StringSlice("Never"));
	inline static const size_t Less_Hash = 6384286012; // Hash<StringSlice>::hash(StringSlice("Less"));
	inline static const size_t Equal_Hash = 210673569885;// Hash<StringSlice>::hash(StringSlice("Equal"));
	inline static const size_t LessOrEqual_Hash = 13833912425920439605;// Hash<StringSlice>::hash(StringSlice("LessOrEqual"));
	inline static const size_t Greater_Hash = 229426120713519;// Hash<StringSlice>::hash(StringSlice("Greater"));
	inline static const size_t NotEqual_Hash = 7571356991977326;   // Hash<StringSlice>::hash(StringSlice("NotEqual"));
	inline static const size_t GreaterOrEqual_Hash = 13559817491955663304;// Hash<StringSlice>::hash(StringSlice("GreaterOrEqual"));
	inline static const size_t Always_Hash = 6952065407446;  // Hash<StringSlice>::hash(StringSlice("Always"));

	inline static ShaderCompareOp::Type from_string(StringSlice& p_str)
	{
		size_t p_str_hash = Hash<StringSlice>::hash(p_str);
		switch (p_str_hash)
		{
			case ShaderCompareOp::Never_Hash: return ShaderCompareOp::Type::Never;
			case ShaderCompareOp::Less_Hash: return ShaderCompareOp::Type::Less;
			case ShaderCompareOp::Equal_Hash: return ShaderCompareOp::Type::Equal;
			case ShaderCompareOp::LessOrEqual_Hash: return ShaderCompareOp::Type::LessOrEqual;
			case ShaderCompareOp::Greater_Hash: return ShaderCompareOp::Type::Greater;
			case ShaderCompareOp::NotEqual_Hash: return ShaderCompareOp::Type::NotEqual;
			case ShaderCompareOp::GreaterOrEqual_Hash: return ShaderCompareOp::Type::GreaterOrEqual;
			case ShaderCompareOp::Always_Hash: return ShaderCompareOp::Type::Always;
			default: return ShaderCompareOp::Type::Invalid;
		}
	};
};

struct ShaderAsset
{
	short execution_order;
	size_t vertex;
	size_t fragment;
	ShaderCompareOp::Type compare_op;

	inline void serialize(com::Vector<char>& out_target)
	{
		Serialization::Binary::serialize_field<short>(&this->execution_order, out_target);
		Serialization::Binary::serialize_field<size_t>(&this->vertex, out_target);
		Serialization::Binary::serialize_field<size_t>(&this->fragment, out_target);
		Serialization::Binary::serialize_field<ShaderCompareOp::Type>(&this->compare_op, out_target);
	};

	inline static ShaderAsset deserialize(const char* p_source)
	{
		ShaderAsset l_resource;
		size_t l_current_pointer = 0;
		l_resource.execution_order = *Serialization::Binary::deserialize_field<short>(l_current_pointer, p_source);
		l_resource.vertex = *Serialization::Binary::deserialize_field<size_t>(l_current_pointer, p_source);
		l_resource.fragment = *Serialization::Binary::deserialize_field<size_t>(l_current_pointer, p_source);
		l_resource.compare_op = *Serialization::Binary::deserialize_field<ShaderCompareOp::Type>(l_current_pointer, p_source);
		return l_resource;
	};
};


template<>
struct JSONDeserializer<ShaderAsset>
{
	inline static ShaderAsset deserialize(Serialization::JSON::JSONObjectIterator& p_iterator)
	{
		ShaderAsset l_asset;

		p_iterator.next_field("type");
		p_iterator.next_field("execution_order"); l_asset.execution_order = JSONDeserializer<short>::deserialize(p_iterator);
		p_iterator.next_field("vertex"); l_asset.vertex = Hash<StringSlice>::hash(p_iterator.get_currentfield().value);
		p_iterator.next_field("fragment"); l_asset.fragment = Hash<StringSlice>::hash(p_iterator.get_currentfield().value);
		if (p_iterator.next_field("compareOp"))
		{
			l_asset.compare_op = ShaderCompareOp::from_string(p_iterator.get_currentfield().value);
		}
		else
		{
			l_asset.compare_op = ShaderCompareOp::Type::LessOrEqual;
		}
		

		p_iterator.free();
		return l_asset;
	}
};


struct MaterialAsset
{
	size_t shader;
	size_t texture;
	Math::vec3f color;

	inline void serialize(com::Vector<char>& out_target)
	{
		Serialization::Binary::serialize_field<size_t>(&this->shader, out_target);
		Serialization::Binary::serialize_field<size_t>(&this->texture, out_target);
		Serialization::Binary::serialize_field<Math::vec3f>(&this->color, out_target);
	};

	inline static MaterialAsset deserialize(const char* p_source)
	{
		MaterialAsset l_resource;
		size_t l_current_pointer = 0;
		l_resource.shader = *Serialization::Binary::deserialize_field<size_t>(l_current_pointer, p_source);
		l_resource.texture = *Serialization::Binary::deserialize_field<size_t>(l_current_pointer, p_source);
		l_resource.color = *Serialization::Binary::deserialize_field<Math::vec3f>(l_current_pointer, p_source);
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
		p_iterator.next_field("shader");
		l_asset.shader = Hash<StringSlice>::hash(p_iterator.get_currentfield().value);
		p_iterator.next_field("texture");
		l_asset.texture = Hash<StringSlice>::hash(p_iterator.get_currentfield().value);

		Serialization::JSON::JSONObjectIterator l_color_iterator;
		if (p_iterator.next_object("color", &l_color_iterator))
		{
			l_asset.color = JSONDeserializer<Math::vec3f>::deserialize(l_color_iterator);
		}
		else
		{
			l_asset.color = Math::vec3f(1.0f, 1.0f, 1.0f);
		}
		p_iterator.free();

		return l_asset;
	}
};
