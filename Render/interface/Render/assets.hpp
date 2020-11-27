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

struct ShaderBlendFactor
{
	enum class Type
	{
		Invalid = 0,
		One = 1,
		Zero = 2,
		SrcColor = 3,
		SrcAlpha = 4,
		DstColor = 5,
		DstAlpha = 6,
		OneMinusSrcColor = 7,
		OneMinusSrcAlpha = 8,
		OneMinusDstColor = 9,
		OneMinusDstAlpha = 10
	};

	inline static const size_t One_Hash = 193466759;	// Hash<StringSlice>::hash(StringSlice("One"));
	inline static const size_t Zero_Hash = 6384789093;	// Hash<StringSlice>::hash(StringSlice("Zero"));
	inline static const size_t SrcColor_Hash = 7571573290841388;	// Hash<StringSlice>::hash(StringSlice("SrcColor"));
	inline static const size_t SrcAlpha_Hash = 7571573288365843;	// Hash<StringSlice>::hash(StringSlice("SrcAlpha"));
	inline static const size_t DstColor_Hash = 7570935970966383;	// Hash<StringSlice>::hash(StringSlice("DstColor"));
	inline static const size_t DstAlpha_Hash = 7570935968490838;	// Hash<StringSlice>::hash(StringSlice("DstAlpha"));
	inline static const size_t OneMinusSrcColor_Hash = 3381930011083874842;	// Hash<StringSlice>::hash(StringSlice("OneMinusSrcColor"));
	inline static const size_t OneMinusSrcAlpha_Hash = 3381930011081399297;	// Hash<StringSlice>::hash(StringSlice("OneMinusSrcAlpha"));
	inline static const size_t OneMinusDstColor_Hash = 3381929373763999837;	// Hash<StringSlice>::hash(StringSlice("OneMinusDstColor"));
	inline static const size_t OneMinusDstAlpha_Hash = 3381929373761524292;	// Hash<StringSlice>::hash(StringSlice("OneMinusDstAlpha"));

	inline static ShaderBlendFactor::Type from_string(StringSlice& p_str)
	{
		size_t p_str_hash = Hash<StringSlice>::hash(p_str);
		switch (p_str_hash)
		{
		case ShaderBlendFactor::One_Hash: return ShaderBlendFactor::Type::One;
		case ShaderBlendFactor::Zero_Hash: return ShaderBlendFactor::Type::Zero;
		case ShaderBlendFactor::SrcColor_Hash: return ShaderBlendFactor::Type::SrcColor;
		case ShaderBlendFactor::SrcAlpha_Hash: return ShaderBlendFactor::Type::SrcAlpha;
		case ShaderBlendFactor::DstColor_Hash: return ShaderBlendFactor::Type::DstColor;
		case ShaderBlendFactor::DstAlpha_Hash: return ShaderBlendFactor::Type::DstAlpha;
		case ShaderBlendFactor::OneMinusSrcColor_Hash: return ShaderBlendFactor::Type::OneMinusSrcColor;
		case ShaderBlendFactor::OneMinusSrcAlpha_Hash: return ShaderBlendFactor::Type::OneMinusSrcAlpha;
		case ShaderBlendFactor::OneMinusDstColor_Hash: return ShaderBlendFactor::Type::OneMinusDstColor;
		case ShaderBlendFactor::OneMinusDstAlpha_Hash: return ShaderBlendFactor::Type::OneMinusDstAlpha;
		default: return ShaderBlendFactor::Type::Invalid;
		}
	};

};

struct ShaderBlendOp
{
	enum class Type
	{
		Invalid = 0,
		Add = 1,
		Substract = 2,
		ReverseSubstract = 3,
		Min = 4,
		Max = 5
	};

	inline static const size_t Add_Hash = 193451182;	// Hash<StringSlice>::hash(StringSlice("Add"));
	inline static const size_t Substract_Hash = 249862047046255232;	// Hash<StringSlice>::hash(StringSlice("Substract"));
	inline static const size_t ReverseSubstract_Hash = 15133734447464593820;	// Hash<StringSlice>::hash(StringSlice("ReverseSubstract"));
	inline static const size_t Min_Hash = 193464425;	// Hash<StringSlice>::hash(StringSlice("Min"));
	inline static const size_t Max_Hash = 193464171;	// Hash<StringSlice>::hash(StringSlice("Max"));


	inline static ShaderBlendOp::Type from_string(StringSlice& p_str)
	{
		size_t p_str_hash = Hash<StringSlice>::hash(p_str);
		switch (p_str_hash)
		{
		case ShaderBlendOp::Add_Hash: return ShaderBlendOp::Type::Add;
		case ShaderBlendOp::Substract_Hash: return ShaderBlendOp::Type::Substract;
		case ShaderBlendOp::ReverseSubstract_Hash: return ShaderBlendOp::Type::ReverseSubstract;
		case ShaderBlendOp::Min_Hash: return ShaderBlendOp::Type::Min;
		case ShaderBlendOp::Max_Hash: return ShaderBlendOp::Type::Max;
		default: return ShaderBlendOp::Type::Invalid;
		}
	};

};

struct ShaderAsset
{
	short execution_order;
	size_t vertex;
	size_t fragment;

	struct Config
	{
		ShaderCompareOp::Type ztest;
		bool zwrite;

		/*
		ShaderBlendFactor::Type srcColorBlendFactor;
		ShaderBlendFactor::Type dstColorBlendFactor;
		ShaderBlendOp::Type colorBlendOp;

		ShaderBlendFactor::Type srcAlphaBlendFactor;
		ShaderBlendFactor::Type dstAlphaBlendFactor;
		ShaderBlendOp::Type alphaBlendOp;
		*/

	} config;

	inline void serialize(com::Vector<char>& out_target)
	{
		Serialization::Binary::serialize_field<short>(&this->execution_order, out_target);
		Serialization::Binary::serialize_field<size_t>(&this->vertex, out_target);
		Serialization::Binary::serialize_field<size_t>(&this->fragment, out_target);
		Serialization::Binary::serialize_field<ShaderCompareOp::Type>(&this->config.ztest, out_target);
		Serialization::Binary::serialize_field<bool>(&this->config.zwrite, out_target);

		/*
		Serialization::Binary::serialize_field<ShaderBlendFactor::Type>(&this->config.srcColorBlendFactor, out_target);
		Serialization::Binary::serialize_field<ShaderBlendFactor::Type>(&this->config.dstColorBlendFactor, out_target);
		Serialization::Binary::serialize_field<ShaderBlendOp::Type>(&this->config.colorBlendOp, out_target);

		Serialization::Binary::serialize_field<ShaderBlendFactor::Type>(&this->config.srcAlphaBlendFactor, out_target);
		Serialization::Binary::serialize_field<ShaderBlendFactor::Type>(&this->config.dstAlphaBlendFactor, out_target);
		Serialization::Binary::serialize_field<ShaderBlendOp::Type>(&this->config.alphaBlendOp, out_target);
		*/
	};

	inline static ShaderAsset deserialize(const char* p_source)
	{
		ShaderAsset l_resource;
		size_t l_current_pointer = 0;
		l_resource.execution_order = *Serialization::Binary::deserialize_field<short>(l_current_pointer, p_source);
		l_resource.vertex = *Serialization::Binary::deserialize_field<size_t>(l_current_pointer, p_source);
		l_resource.fragment = *Serialization::Binary::deserialize_field<size_t>(l_current_pointer, p_source);
		l_resource.config.ztest = *Serialization::Binary::deserialize_field<ShaderCompareOp::Type>(l_current_pointer, p_source);
		l_resource.config.zwrite = *Serialization::Binary::deserialize_field<bool>(l_current_pointer, p_source);

		/*
		l_resource.config.srcColorBlendFactor = *Serialization::Binary::deserialize_field<ShaderBlendFactor::Type>(l_current_pointer, p_source);
		l_resource.config.dstColorBlendFactor = *Serialization::Binary::deserialize_field<ShaderBlendFactor::Type>(l_current_pointer, p_source);
		l_resource.config.colorBlendOp = *Serialization::Binary::deserialize_field<ShaderBlendOp::Type>(l_current_pointer, p_source);

		l_resource.config.srcAlphaBlendFactor = *Serialization::Binary::deserialize_field<ShaderBlendFactor::Type>(l_current_pointer, p_source);
		l_resource.config.dstAlphaBlendFactor = *Serialization::Binary::deserialize_field<ShaderBlendFactor::Type>(l_current_pointer, p_source);
		l_resource.config.alphaBlendOp = *Serialization::Binary::deserialize_field<ShaderBlendOp::Type>(l_current_pointer, p_source);
		*/
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

		if (p_iterator.next_field("ztest"))
		{
			l_asset.config.ztest = ShaderCompareOp::from_string(p_iterator.get_currentfield().value);
		}
		else
		{
			l_asset.config.ztest = ShaderCompareOp::Type::LessOrEqual;
		}

		if (p_iterator.next_field("zwrite")) { l_asset.config.zwrite = JSONDeserializer<bool>::deserialize(p_iterator); }
		else { l_asset.config.zwrite = true; }

		/*
		if (p_iterator.next_field("srcColorBlendFactor")) { l_asset.config.srcColorBlendFactor = ShaderBlendFactor::from_string(p_iterator.get_currentfield().value); }
		else { l_asset.config.srcColorBlendFactor = ShaderBlendFactor::Type::Invalid; }
		if (p_iterator.next_field("dstColorBlendFactor")) { l_asset.config.dstColorBlendFactor = ShaderBlendFactor::from_string(p_iterator.get_currentfield().value); }
		else { l_asset.config.dstColorBlendFactor = ShaderBlendFactor::Type::Invalid; }
		if (p_iterator.next_field("colorBlendOp")) { l_asset.config.colorBlendOp = ShaderBlendOp::from_string(p_iterator.get_currentfield().value); }
		else { l_asset.config.colorBlendOp = ShaderBlendOp::Type::Invalid; }


		if (p_iterator.next_field("srcAlphaBlendFactor")) { l_asset.config.srcAlphaBlendFactor = ShaderBlendFactor::from_string(p_iterator.get_currentfield().value); }
		else { l_asset.config.srcAlphaBlendFactor = ShaderBlendFactor::Type::Invalid; }
		if (p_iterator.next_field("dstAlphaBlendFactor")) { l_asset.config.dstAlphaBlendFactor = ShaderBlendFactor::from_string(p_iterator.get_currentfield().value); }
		else { l_asset.config.dstAlphaBlendFactor = ShaderBlendFactor::Type::Invalid; }
		if (p_iterator.next_field("alphaBlendOp")) { l_asset.config.alphaBlendOp = ShaderBlendOp::from_string(p_iterator.get_currentfield().value); }
		else { l_asset.config.alphaBlendOp = ShaderBlendOp::Type::Invalid; }
		*/
		p_iterator.free();
		return l_asset;
	}
};


struct MaterialAsset
{
	size_t shader;
	size_t texture;
	Math::vec4f color;

	inline void serialize(com::Vector<char>& out_target)
	{
		Serialization::Binary::serialize_field<size_t>(&this->shader, out_target);
		Serialization::Binary::serialize_field<size_t>(&this->texture, out_target);
		Serialization::Binary::serialize_field<Math::vec4f>(&this->color, out_target);
	};

	inline static MaterialAsset deserialize(const char* p_source)
	{
		MaterialAsset l_resource;
		size_t l_current_pointer = 0;
		l_resource.shader = *Serialization::Binary::deserialize_field<size_t>(l_current_pointer, p_source);
		l_resource.texture = *Serialization::Binary::deserialize_field<size_t>(l_current_pointer, p_source);
		l_resource.color = *Serialization::Binary::deserialize_field<Math::vec4f>(l_current_pointer, p_source);
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
			l_asset.color = JSONDeserializer<Math::vec4f>::deserialize(l_color_iterator);
		}
		else
		{
			l_asset.color = Math::vec4f(1.0f, 1.0f, 1.0f, 1.0f);
		}
		p_iterator.free();

		return l_asset;
	}
};
