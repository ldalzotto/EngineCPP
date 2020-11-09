#pragma once

#include <string>

// http://www.cse.yorku.ca/~oz/hash.html
inline size_t HashFunctionRaw(const char* p_value, size_t p_size)
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
}


template<class Key>
struct Hash
{
	static size_t hash(const Key& p_key);
};

template<>
struct Hash<std::string>
{
	static size_t hash(const std::string& p_key)
	{
		return HashFunctionRaw(p_key.c_str(), p_key.length());
	};
};