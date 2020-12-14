#pragma once

#include "Common/Container/vector.hpp"
#include "Common/Container/varying_vector.hpp"
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

	inline static ShaderCompareOp::Type from_string(StringSlice& p_str)
	{
		size_t p_str_hash = Hash<StringSlice>::hash(p_str);
		switch (p_str_hash)
		{
		case Hash<ConstString>::hash("Never"): return ShaderCompareOp::Type::Never;
		case Hash<ConstString>::hash("Less"): return ShaderCompareOp::Type::Less;
		case Hash<ConstString>::hash("Equal"): return ShaderCompareOp::Type::Equal;
		case Hash<ConstString>::hash("LessOrEqual"): return ShaderCompareOp::Type::LessOrEqual;
		case Hash<ConstString>::hash("Greater"): return ShaderCompareOp::Type::Greater;
		case Hash<ConstString>::hash("NotEqual"): return ShaderCompareOp::Type::NotEqual;
		case Hash<ConstString>::hash("GreaterOrEqual"): return ShaderCompareOp::Type::GreaterOrEqual;
		case Hash<ConstString>::hash("Always"): return ShaderCompareOp::Type::Always;
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

	inline static ShaderBlendFactor::Type from_string(StringSlice& p_str)
	{
		size_t p_str_hash = Hash<StringSlice>::hash(p_str);
		switch (p_str_hash)
		{
		case Hash<ConstString>::hash("One"): return ShaderBlendFactor::Type::One;
		case Hash<ConstString>::hash("Zero"): return ShaderBlendFactor::Type::Zero;
		case Hash<ConstString>::hash("SrcColor"): return ShaderBlendFactor::Type::SrcColor;
		case Hash<ConstString>::hash("SrcAlpha"): return ShaderBlendFactor::Type::SrcAlpha;
		case Hash<ConstString>::hash("DstColor"): return ShaderBlendFactor::Type::DstColor;
		case Hash<ConstString>::hash("DstAlpha"): return ShaderBlendFactor::Type::DstAlpha;
		case Hash<ConstString>::hash("OneMinusSrcColor"): return ShaderBlendFactor::Type::OneMinusSrcColor;
		case Hash<ConstString>::hash("OneMinusSrcAlpha"): return ShaderBlendFactor::Type::OneMinusSrcAlpha;
		case Hash<ConstString>::hash("OneMinusDstColor"): return ShaderBlendFactor::Type::OneMinusDstColor;
		case Hash<ConstString>::hash("OneMinusDstAlpha"): return ShaderBlendFactor::Type::OneMinusDstAlpha;
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

	inline static ShaderBlendOp::Type from_string(StringSlice& p_str)
	{
		size_t p_str_hash = Hash<StringSlice>::hash(p_str);
		switch (p_str_hash)
		{
		case Hash<ConstString>::hash("Add"): return ShaderBlendOp::Type::Add;
		case Hash<ConstString>::hash("Substract"): return ShaderBlendOp::Type::Substract;
		case Hash<ConstString>::hash("ReverseSubstract"): return ShaderBlendOp::Type::ReverseSubstract;
		case Hash<ConstString>::hash("Min"): return ShaderBlendOp::Type::Min;
		case Hash<ConstString>::hash("Max"): return ShaderBlendOp::Type::Max;
		default: return ShaderBlendOp::Type::Invalid;
		}
	};

};

struct ShaderLayoutParameter
{
	enum Type
	{
		UNDEFINED = 0,
		UNIFORM_BUFFER_VERTEX = 1,
		TEXTURE_FRAGMENT = 2,
		UNIFORM_BUFFER_VERTEX_FRAGMENT = 3
	};

	inline static ShaderLayoutParameter::Type from_string(StringSlice& p_str)
	{
		size_t p_str_hash = Hash<StringSlice>::hash(p_str);
		switch (p_str_hash)
		{
		case Hash<ConstString>::hash("UNIFORM_BUFFER_VERTEX"): return ShaderLayoutParameter::Type::UNIFORM_BUFFER_VERTEX;
		case Hash<ConstString>::hash("TEXTURE_FRAGMENT"): return ShaderLayoutParameter::Type::TEXTURE_FRAGMENT;
		case Hash<ConstString>::hash("UNIFORM_BUFFER_VERTEX_FRAGMENT"): return ShaderLayoutParameter::Type::UNIFORM_BUFFER_VERTEX_FRAGMENT;
		default: return ShaderLayoutParameter::Type::UNDEFINED;
		}
	};
};


struct ShaderLayoutAsset
{
	com::Vector<ShaderLayoutParameter::Type> parameters;

	inline void free()
	{
		this->parameters.free();
	};

	inline void serialize(com::Vector<char>& out_target)
	{
		Serialization::Binary::serialize_vector(this->parameters, out_target);
	};

	inline static ShaderLayoutAsset deserialize(const char* p_source)
	{
		ShaderLayoutAsset l_shaderlayout_asset;
		size_t l_current_pointer = 0;
		l_shaderlayout_asset.parameters = Serialization::Binary::deserialize_vector<ShaderLayoutParameter::Type>(l_current_pointer, p_source);
		
		return l_shaderlayout_asset;
	};
};

template<>
struct JSONDeserializer<ShaderLayoutAsset>
{
	inline static ShaderLayoutAsset deserialize(Deserialization::JSON::JSONObjectIterator& p_iterator)
	{
		ShaderLayoutAsset l_asset;

		p_iterator.next_field("type");

		Deserialization::JSON::JSONObjectIterator l_parameters_iterator;
		p_iterator.next_array("parameters", &l_parameters_iterator);
		Deserialization::JSON::JSONObjectIterator l_parameter_iterator;
		while (l_parameters_iterator.next_array_object(&l_parameter_iterator))
		{
			l_parameter_iterator.next_field("type");
			l_asset.parameters.push_back(ShaderLayoutParameter::from_string(l_parameter_iterator.get_currentfield().value));
		}
		l_parameter_iterator.free();
		l_parameters_iterator.free();

		p_iterator.free();
		return l_asset;
	}
};

struct ShaderAsset
{
	short execution_order;
	size_t vertex;
	size_t fragment;
	size_t layout;

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
		Serialization::Binary::serialize_field<size_t>(&this->layout, out_target);
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
		l_resource.layout = *Serialization::Binary::deserialize_field<size_t>(l_current_pointer, p_source);
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
	inline static ShaderAsset deserialize(Deserialization::JSON::JSONObjectIterator& p_iterator)
	{
		ShaderAsset l_asset;

		p_iterator.next_field("type");
		p_iterator.next_field("execution_order"); l_asset.execution_order = JSONDeserializer<short>::deserialize(p_iterator);
		p_iterator.next_field("vertex"); l_asset.vertex = Hash<StringSlice>::hash(p_iterator.get_currentfield().value);
		p_iterator.next_field("fragment"); l_asset.fragment = Hash<StringSlice>::hash(p_iterator.get_currentfield().value);
		p_iterator.next_field("layout"); l_asset.layout = Hash<StringSlice>::hash(p_iterator.get_currentfield().value);

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

enum class MaterialAssetParameterType
{
	UNKNOWN = 0,
	TEXTURE = 1,
	VEC4F = 2
};

struct MaterialAsset
{
	size_t shader;
	VaryingVector<MaterialAssetParameterType> parameters;

	inline void free()
	{
		this->parameters.free();
	}

	inline void serialize(com::Vector<char>& out_target)
	{
		Serialization::Binary::serialize_field<size_t>(&this->shader, out_target);
		Serialization::Binary::serialize_varyingvector(this->parameters, out_target);
	};

	inline static MaterialAsset deserialize(const char* p_source)
	{
		MaterialAsset l_resource;
		size_t l_current_pointer = 0;
		l_resource.shader = *Serialization::Binary::deserialize_field<size_t>(l_current_pointer, p_source);
		l_resource.parameters = Serialization::Binary::deserialize_varyingvector<MaterialAssetParameterType>(l_current_pointer, p_source);
		return l_resource;
	};
};

template<>
struct JSONDeserializer<MaterialAsset>
{
	inline static MaterialAsset deserialize(Deserialization::JSON::JSONObjectIterator& p_iterator)
	{
		MaterialAsset l_asset;

		p_iterator.next_field("type");
		p_iterator.next_field("shader");
		l_asset.shader = Hash<StringSlice>::hash(p_iterator.get_currentfield().value);

		Deserialization::JSON::JSONObjectIterator l_parameters_iterator;
		p_iterator.next_array("parameters", &l_parameters_iterator);
		Deserialization::JSON::JSONObjectIterator l_parameter_iterator;
		
		while (l_parameters_iterator.next_array_object(&l_parameter_iterator))
		{
			l_parameter_iterator.next_field("type");
			if (l_parameter_iterator.get_currentfield().value.equals(StringSlice("TEXTURE")))
			{
				l_parameter_iterator.next_field("object");
				size_t l_texture = Hash<StringSlice>::hash(l_parameter_iterator.get_currentfield().value);
				l_asset.parameters.push_back(MaterialAssetParameterType::TEXTURE, l_texture);
			}
			else if (l_parameter_iterator.get_currentfield().value.equals(StringSlice("VEC4F")))
			{
				Deserialization::JSON::JSONObjectIterator l_value_iterator;
				l_parameter_iterator.next_object("object", &l_value_iterator);
				Math::vec4f l_param = JSONDeserializer<Math::vec4f>::deserialize(l_value_iterator);
				l_asset.parameters.push_back(MaterialAssetParameterType::VEC4F, l_param);
			}
			//TODO -> implementing object as parameter that are a nested list of non obejct material parameter

		}

		l_parameters_iterator.free();
		l_parameter_iterator.free();
		p_iterator.free();

		return l_asset;
	}
};
