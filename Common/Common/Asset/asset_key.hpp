#pragma once

#include "Common/Functional/Hash.hpp"

struct AssetKey
{
	size_t key;
	inline AssetKey() {};
	inline AssetKey(size_t p_key) { this->key = p_key; };
	inline AssetKey(const StringSlice& p_key) { this->key = Hash<StringSlice>::hash(p_key); }
};