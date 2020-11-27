#pragma once

#include "Common/Serialization/json.hpp"
#include "vector_def.hpp"
#include "quaternion_def.hpp"

template<>
struct JSONDeserializer<Math::vec3f>
{
	static Math::vec3f deserialize(Serialization::JSON::JSONObjectIterator& p_iterator)
	{
		Math::vec3f l_return;

		String<> l_float_str; l_float_str.allocate(0);

		p_iterator.next_field("x");
		l_float_str.append(p_iterator.get_currentfield().value);
		l_return.x = (float)atof(l_float_str.c_str());
		l_float_str.clear();

		p_iterator.next_field("y");
		l_float_str.append(p_iterator.get_currentfield().value);
		l_return.y = (float)atof(l_float_str.c_str());
		l_float_str.clear();

		p_iterator.next_field("z");
		l_float_str.append(p_iterator.get_currentfield().value);
		l_return.z = (float)atof(l_float_str.c_str());
		l_float_str.clear();

		l_float_str.free();

		p_iterator.free();

		return l_return;
	};
};

template<>
struct JSONDeserializer<Math::vec4f>
{
	static Math::vec4f deserialize(Serialization::JSON::JSONObjectIterator& p_iterator)
	{
		Math::vec4f l_return;

		String<> l_float_str; l_float_str.allocate(0);

		p_iterator.next_field("x");
		l_float_str.append(p_iterator.get_currentfield().value);
		l_return.x = (float)atof(l_float_str.c_str());
		l_float_str.clear();

		p_iterator.next_field("y");
		l_float_str.append(p_iterator.get_currentfield().value);
		l_return.y = (float)atof(l_float_str.c_str());
		l_float_str.clear();

		p_iterator.next_field("z");
		l_float_str.append(p_iterator.get_currentfield().value);
		l_return.z = (float)atof(l_float_str.c_str());
		l_float_str.clear();

		p_iterator.next_field("w");
		l_float_str.append(p_iterator.get_currentfield().value);
		l_return.w = (float)atof(l_float_str.c_str());
		l_float_str.clear();

		l_float_str.free();

		p_iterator.free();

		return l_return;
	};
};

template<>
struct JSONDeserializer<Math::quat>
{
	static Math::quat deserialize(Serialization::JSON::JSONObjectIterator& p_iterator)
	{
		Math::quat l_return;

		String<> l_float_str; l_float_str.allocate(0);

		p_iterator.next_field("x");
		l_float_str.append(p_iterator.get_currentfield().value);
		l_return.x = (float)atof(l_float_str.c_str());
		l_float_str.clear();

		p_iterator.next_field("y");
		l_float_str.append(p_iterator.get_currentfield().value);
		l_return.y = (float)atof(l_float_str.c_str());
		l_float_str.clear();

		p_iterator.next_field("z");
		l_float_str.append(p_iterator.get_currentfield().value);
		l_return.z = (float)atof(l_float_str.c_str());
		l_float_str.clear();

		p_iterator.next_field("w");
		l_float_str.append(p_iterator.get_currentfield().value);
		l_return.w = (float)atof(l_float_str.c_str());
		l_float_str.clear();

		l_float_str.free();
		p_iterator.free();

		return l_return;
	};
};