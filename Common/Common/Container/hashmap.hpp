#pragma once

#include "hashmap_def.hpp"

template<class Key, class Value, class HashFn, class Allocator>
inline HashMap<Key, Value, HashFn, Allocator>::Entry::Entry(const Key& p_key, const Value& p_value)
{
	this->key = p_key;
	this->value = p_value;
};

template<class Key, class Value, class HashFn, class Allocator>
inline void HashMap<Key, Value, HashFn, Allocator>::allocate(size_t p_initialSize, const Allocator& p_allocator)
{
	this->allocator = p_allocator;
	this->Entries.Memory = this->allocator.calloc(sizeof(HashMap<Key, Value, HashFn, Allocator>::Entry) * p_initialSize);
	this->Entries.Capacity = p_initialSize;
};

template<class Key, class Value, class HashFn, class Allocator>
inline void HashMap<Key, Value, HashFn, Allocator>::free()
{
	this->allocator.free(this->Entries);
	this->Entries = nullptr;
	this->Capacity = 0;
};


inline size_t hashmap_calculateindex_from_hash(const size_t& p_hash, const size_t p_maxvalue_excluded)
{
	return p_hash % p_maxvalue_excluded;
};

template<class Key, class HashFn>
inline size_t hashmap_calculateindex_from_key(const Key& p_key, const size_t p_maxvalue_excluded)
{
	return hashmap_calculateindex_from_hash(HashFn::hash(p_key), p_maxvalue_excluded);
};


template<class Key, class Value, class HashFn, class Allocator>
inline void hashmap_reallocate_entries(HashMap<Key, Value, HashFn, Allocator>& p_hashmap,const size_t p_newCapacity)
{
	if (p_newCapacity > p_hashmap.Entries.Capacity)
	{
		HashMap<Key, Value, HashFn, Allocator>::Entry* l_new_entries = (HashMap<Key, Value, HashFn, Allocator>::Entry*)p_hashmap.allocator.calloc(sizeof(HashMap<Key, Value, HashFn, Allocator>::Entry) * p_newCapacity);

		for (size_t i = 0; i < p_hashmap.Entries.Capacity; i++)
		{
			HashMap<Key, Value, HashFn, Allocator>::Entry& l_entry = p_hashmap.Entries.Memory[i];
			if (l_entry.isOccupied)
			{
				size_t l_rehashedkey = hashmap_calculateindex_from_key<Key, HashFn>(l_entry.key, p_newCapacity);
				l_new_entries[l_rehashedkey] = l_entry;
			}
		}

		p_hashmap.allocator.free((void*)p_hashmap.Entries.Memory);
		p_hashmap.Entries.Memory = l_new_entries;
		p_hashmap.Entries.Capacity = p_newCapacity;
	}
};

template<class Key, class Value, class HashFn, class Allocator>
inline Value& HashMap<Key, Value, HashFn, Allocator>::operator[](const Key& p_key)
{
	size_t l_inputkey_hash = HashFn::hash(p_key);
	Entry& l_entry = this->Entries.Memory[hashmap_calculateindex_from_hash(l_inputkey_hash, this->Entries.Capacity)] ;
	if (l_entry.isOccupied)
	{
		if (HashFn::hash(l_entry.key) == l_inputkey_hash)
		{
			return l_entry.value;
		}
	}
	// return (Value&)(void*)0;
}

template<class Key, class Value, class HashFn, class Allocator>
inline void HashMap<Key, Value, HashFn, Allocator>::push_entry(const Entry& p_entry)
{
	if (this->Entries.Capacity == 0)
	{
		hashmap_reallocate_entries(*this, this->Entries.Capacity == 0 ? 1 : (this->Entries.Capacity * 2));
		this->push_entry(p_entry);
	}
	else
	{
		size_t l_inputkey_hash = HashFn::hash(p_entry.key);
		Entry& l_target_entry = this->Entries.Memory[hashmap_calculateindex_from_hash(l_inputkey_hash, this->Entries.Capacity)];
		if (l_target_entry.isOccupied)
		{
			if(hashmap_calculateindex_from_key<Key, HashFn>(l_target_entry.key, this->Entries.Capacity) == l_inputkey_hash)
			{
				//TODO -> ERROR we are trying to insert the same key
				return;
			}
			else
			{
				hashmap_reallocate_entries(*this, this->Entries.Capacity == 0 ? 1 : (this->Entries.Capacity * 2));
				this->push_entry(p_entry);
			}
		}
		else
		{
			l_target_entry = p_entry;
			l_target_entry.isOccupied = true;
		}
	}
}
