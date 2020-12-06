#pragma once

#include <string>
#include "Common/Container/string_def.hpp"

// http://www.cse.yorku.ca/~oz/hash.html
inline constexpr size_t HashFunctionRaw(const char* p_value, const size_t p_size)
{
	size_t hash = 5381;

	for (size_t i = 0; i < p_size; i++)
	{
		char c = *(p_value + i);
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash;
}

template<class TYPE>
inline size_t HashFunction(const TYPE& p_value)
{
	return HashFunctionRaw((char*)&p_value, sizeof(TYPE));
};

template<class TYPE>
inline size_t HashCombineFunction(const size_t p_hash, const TYPE& p_value)
{
	return p_hash ^ (HashFunction<TYPE>(p_value) + 0x9e3779b9 + (p_hash << 6) + (p_hash >> 2));
};


template<class Key>
struct Hash
{
	static size_t hash(const Key& p_key);
};

template<>
struct Hash<std::string>
{
	inline static size_t hash(const std::string& p_key)
	{
		return HashFunctionRaw(p_key.c_str(), p_key.length());
	};
};

template<>
struct Hash<StringSlice>
{
	inline static size_t hash(const StringSlice& p_key)
	{
		return HashFunctionRaw(p_key.Memory + p_key.Begin, p_key.End - p_key.Begin);
	};
};

template<>
struct Hash<size_t>
{
	inline static size_t hash(const size_t p_key)
	{
		return p_key;
	};
};

template<>
struct Hash<ConstString>
{
	inline static constexpr const size_t hash(const char* p_memory)
	{
		return HashFunctionRaw(p_memory, ConstString::size(p_memory) - 1);
		// return HashFunctionRaw(p_str, p_str.Size);
	};
};
