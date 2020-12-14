#pragma once

#include "hashmap_def.hpp"

template<class Value>
struct CountedResource
{
	Value value;
	size_t usage = 0;
};

struct ResourceMapEnum
{
	enum Step
	{
		UNDEFINED = 0,
		RESOURCE_ALLOCATED = 1,
		RESOURCE_INCREMENTED = 2,
		RESOURCE_DEALLOCATED = 3,
		RESOURCE_DECREMENTED = 4
	};
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

	struct DefaultResourceCallback
	{
		inline void execute() {};
	};

	template<class OnResourceAllocatedFn = DefaultResourceCallback, class OnResourceIncrementedFn = DefaultResourceCallback>
	inline Value Tallocate_resource(const Key& p_key, OnResourceAllocatedFn& p_on_resource_allocated_fn = DefaultResourceCallback(),
		OnResourceIncrementedFn& p_on_resource_incremented_fn = DefaultResourceCallback())
	{
		CountedResource<Value>* l_existing_resourece;
		if (this->map.get(p_key, &l_existing_resourece))
		{
			l_existing_resourece->usage += 1;
			p_on_resource_incremented_fn.execute();
			return l_existing_resourece->value;
		}
		else
		{
			CountedResource<Value> l_counted_resource;
			l_counted_resource.value = this->resource_allocation.allocate(p_key);
			l_counted_resource.usage = 1;
			this->map.push_entry(HashMap<Key, CountedResource<Value>>::Entry(p_key, l_counted_resource));
			p_on_resource_allocated_fn.execute();
			return l_counted_resource.value;
		}
	}
	
	inline Value allocate_resource(const Key& p_key)
	{
		return this->Tallocate_resource(p_key);
	};

	inline ResourceMapEnum::Step allocate_resource(const Key& p_key, Value* out_value)
	{
		struct OnResourceAllocated { 
			ResourceMapEnum::Step* step;
			inline OnResourceAllocated(ResourceMapEnum::Step* p_step) { this->step = p_step; }
			inline void execute() { *this->step = ResourceMapEnum::Step::RESOURCE_ALLOCATED; };
		};
		struct OnResourceIncremented { 
			ResourceMapEnum::Step* step;
			inline OnResourceIncremented(ResourceMapEnum::Step* p_step) { this->step = p_step; }
			inline void execute() { *this->step = ResourceMapEnum::Step::RESOURCE_INCREMENTED; };
		};

		ResourceMapEnum::Step l_execution_step = ResourceMapEnum::Step::UNDEFINED;
		*out_value = this->Tallocate_resource(p_key, OnResourceAllocated(&l_execution_step), OnResourceIncremented(&l_execution_step));
		return l_execution_step;
	};

	inline void push_resource(const Key& p_key, const Value& p_value)
	{
#if CONTAINER_BOUND_TEST
		if (this->map.conains_key(p_key))
		{
			abort();
		}
#endif
		CountedResource<Value> l_usage;
		l_usage.value = p_value;
		l_usage.usage = 1;
		this->map.push_entry(HashMap<Key, CountedResource<Value>>::Entry(p_key, l_usage));
	};
	
	template<class OnResourceDeallocatedFn = DefaultResourceCallback, class OnResourceDecrementedFn = DefaultResourceCallback>
	inline void Tfree_resource(const Key& p_key, OnResourceDeallocatedFn& p_on_resource_deallocated_fn = DefaultResourceCallback(), 
				OnResourceDecrementedFn& p_on_resource_decremented_fn = DefaultResourceCallback())
	{
		CountedResource<Value>* l_existing_resourece;
		if (this->map.get(p_key, &l_existing_resourece))
		{
			l_existing_resourece->usage -= 1;
			if (l_existing_resourece->usage == 0)
			{
				this->resource_allocation.free(this->map[p_key].value);
				this->map.remove(p_key);
				p_on_resource_deallocated_fn.execute();
			}
			else
			{
				p_on_resource_decremented_fn.execute();
			}
		}
	};

	inline void free_resource(const Key& p_key)
	{
		this->Tfree_resource(p_key);
	};

	inline ResourceMapEnum::Step free_resource_step(const Key& p_key)
	{
		struct OnResourceDeAllocated {
			ResourceMapEnum::Step* step;
			inline OnResourceDeAllocated(ResourceMapEnum::Step* p_step) { this->step = p_step; }
			inline void execute() { *this->step = ResourceMapEnum::Step::RESOURCE_DEALLOCATED; };
		};
		struct OnResourceDecremented {
			ResourceMapEnum::Step* step;
			inline OnResourceDecremented(ResourceMapEnum::Step* p_step) { this->step = p_step; }
			inline void execute() { *this->step = ResourceMapEnum::Step::RESOURCE_DECREMENTED; };
		};

		ResourceMapEnum::Step l_step = ResourceMapEnum::Step::UNDEFINED;
		this->Tfree_resource(p_key, OnResourceDeAllocated(&l_step), OnResourceDecremented(&l_step));
		return l_step;
	};
};

template<class Key, class Value, class Allocator = HeapAllocator>
struct ResourceMap2
{
	HashMap<Key, CountedResource<Value>> map;

	inline void allocate(size_t p_initial_size, Allocator& l_allocator = HeapAllocator())
	{
		this->map.allocate(p_initial_size, l_allocator);
	};

	inline void free()
	{
		this->map.free();
	};

	template<class ResourceAllocation>
	inline ResourceMapEnum::Step allocate_resource(const Key& p_key, Value* out_value, ResourceAllocation& p_resource_allocation)
	{
		CountedResource<Value>* l_existing_resourece;
		if (this->map.get(p_key, &l_existing_resourece))
		{
			l_existing_resourece->usage += 1;
			*out_value = l_existing_resourece->value;
			return ResourceMapEnum::Step::RESOURCE_INCREMENTED;
		}
		else
		{
			CountedResource<Value> l_counted_resource;
			l_counted_resource.value = p_resource_allocation.allocate(p_key);
			l_counted_resource.usage = 1;
			this->map.push_entry(HashMap<Key, CountedResource<Value>>::Entry(p_key, l_counted_resource));
			*out_value = l_counted_resource.value;
			return ResourceMapEnum::Step::RESOURCE_ALLOCATED;
		}
	};

	template<class ResourceDeallocation>
	inline ResourceMapEnum::Step free_resource(const Key& p_key, ResourceDeallocation& p_resrouce_deallocation)
	{
		CountedResource<Value>* l_existing_resourece;
		if (this->map.get(p_key, &l_existing_resourece))
		{
			l_existing_resourece->usage -= 1;
			if (l_existing_resourece->usage == 0)
			{
				p_resrouce_deallocation.free(this->map[p_key].value);
				this->map.remove(p_key);
				return ResourceMapEnum::Step::RESOURCE_DEALLOCATED;
			}
			else
			{
				return ResourceMapEnum::Step::RESOURCE_DECREMENTED;
			}
		}

		return ResourceMapEnum::Step::UNDEFINED;
	};
};