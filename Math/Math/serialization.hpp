#pragma once

#include "Common/Serialization/json.hpp"
#include "vector_def.hpp"
#include "quaternion_def.hpp"

template<>
struct JSONDeserializer<Math::vec3f>
{
	inline static Math::vec3f deserialize(Deserialization::JSON::JSONObjectIterator& p_iterator)
	{
		Math::vec3f l_return;

		p_iterator.next_field("x", &l_return.x);
		p_iterator.next_field("y", &l_return.y);
		p_iterator.next_field("z", &l_return.z);
		p_iterator.free();

		return l_return;
	};
};

template<>
struct JSONSerializer<Math::vec3f>
{
	inline static void serialize(Serialization::JSON::Deserializer& p_serializer, const Math::vec3f& p_object)
	{
		p_serializer.push_field("x", p_object.x);
		p_serializer.push_field("y", p_object.y);
		p_serializer.push_field("z", p_object.z);
	};
};

template<>
struct JSONDeserializer<Math::vec4f>
{
	inline static Math::vec4f deserialize(Deserialization::JSON::JSONObjectIterator& p_iterator)
	{
		Math::vec4f l_return;

		p_iterator.next_field("x", &l_return.x);
		p_iterator.next_field("y", &l_return.y);
		p_iterator.next_field("z", &l_return.z);
		p_iterator.next_field("w", &l_return.w);

		p_iterator.free();

		return l_return;
	};
};

template<>
struct JSONSerializer<Math::vec4f>
{
	inline static void serialize(Serialization::JSON::Deserializer& p_serializer, const Math::vec4f& p_object)
	{
		p_serializer.push_field("x", p_object.x);
		p_serializer.push_field("y", p_object.y);
		p_serializer.push_field("z", p_object.z);
		p_serializer.push_field("w", p_object.w);
	};
};

template<>
struct JSONDeserializer<Math::quat>
{
	inline static Math::quat deserialize(Deserialization::JSON::JSONObjectIterator& p_iterator)
	{
		Math::quat l_return;

		p_iterator.next_field("x", &l_return.x);
		p_iterator.next_field("y", &l_return.y);
		p_iterator.next_field("z", &l_return.z);
		p_iterator.next_field("w", &l_return.w);

		p_iterator.free();

		return l_return;
	};
};


template<>
struct JSONSerializer<Math::quat>
{
	inline static void serialize(Serialization::JSON::Deserializer& p_serializer, const Math::quat& p_object)
	{
		p_serializer.push_field("x", p_object.x);
		p_serializer.push_field("y", p_object.y);
		p_serializer.push_field("z", p_object.z);
		p_serializer.push_field("w", p_object.w);
	};
};