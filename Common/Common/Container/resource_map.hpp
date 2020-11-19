#pragma once

#include "hashmap_def.hpp"

template<class Value>
struct CountedResource
{
	Value value;
	size_t usage = 0;
};

template<class Key, class Value, class ResourceAllocation, class Allocator = HeapAllocator>
struct ResourceMap
{
	ResourceAllocation resource_allocation;
	HashMap<Key, CountedResource<Value>> map;

	inline void allocate(size_t p_initial_size, ResourceAllocation& p_resource_allocation, Allocator& l_allocator = HeapAllocator())
	{
		this->map.allocate(p_initial_size, l_allocator);
		this->resource_allocation = p_resource_allocation;
	};
	
	inline void free()
	{
		for (size_t i = 0; i < this->map.Entries.Capacity; i++)
		{
			HashMap<Key, CountedResource<Value>>::Entry& l_entry = this->map.Entries[i];
			if (l_entry.isOccupied)
			{
				this->resource_allocation.free(l_entry.value.value);
			}
		}

		this->map.free();
	};

	inline Value allocate_resource(const Key& p_key)
	{
		CountedResource<Value>* l_existing_resourece;
		if (this->map.get(p_key, &l_existing_resourece))
		{
			l_existing_resourece->usage += 1;
			return l_existing_resourece->value;
		}
		else
		{
			CountedResource<Value> l_counted_resource;
			l_counted_resource.value = this->resource_allocation.allocate(p_key);
			l_counted_resource.usage = 1;
			this->map.push_entry(HashMap<Key, CountedResource<Value>>::Entry(p_key, l_counted_resource));
			return l_counted_resource.value;
		}
	};
	
	void free_resource(const Key& p_key)
	{
		CountedResource<Value>* l_existing_resourece;
		if (this->map.get(p_key, &l_existing_resourece))
		{
			l_existing_resourece->usage -= 1;
			if (l_existing_resourece->usage == 0)
			{
				this->resource_allocation.free(this->map[p_key].value);
				this->map.remove(p_key);
			}
		}
	};
};
