#pragma once

#include "Common/Memory/allocators.hpp"
#include "Common/Container/array_def.hpp"

template<class Key>
struct Hash
{
	size_t hash(const Key& p_key);
};

template<class Key, class Value, class HashFn = Hash<Key>, class Allocator = HeapAllocator>
struct HashMap
{
	struct Entry
	{
		Key key;
		Value value;
		bool isOccupied = false;

		Entry() {};
		Entry(const Key& p_key, const Value& p_value);
	};

	Allocator allocator;
	Array<Entry> Entries;

	void allocate(size_t p_initialSize, const Allocator& p_allocator = Allocator());
	void free();
	Value& operator[](const Key& p_key);
	void push_entry(const Entry& p_entry);
};


