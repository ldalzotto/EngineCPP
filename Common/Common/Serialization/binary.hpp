#pragma once

#include "Common/Container/vector.hpp"
#include "Common/Memory/heap.hpp"

namespace Serialization
{
	struct Binary
	{
		template<class FieldType>
		inline static void serialize_field(const FieldType* p_source, com::Vector<char>& p_target_data)
		{
			p_target_data.insert_at(com::MemorySlice<char>((const char*)p_source, sizeof(FieldType)), p_target_data.Size);
		};

		template<class VectorElementType>
		inline static void serialize_vector(const com::Vector<VectorElementType>& p_source, com::Vector<char>& p_target_data)
		{
			p_target_data.insert_at(com::MemorySlice<char>((const char*)&p_source.Size, sizeof(size_t)), p_target_data.Size);
			p_target_data.insert_at(com::MemorySlice<char>((const char*)p_source.Memory, p_source.size_in_bytes()), p_target_data.Size);
		};

		template<class PoolElementType>
		inline static void serialize_pool(const com::Pool<PoolElementType>& p_source, com::Vector<char>& p_target_data)
		{
			serialize_vector<PoolElementType>(p_source.Memory, p_target_data);
			serialize_vector<size_t>(p_source.FreeBlocks, p_target_data);
		};

		inline static void serialize_heap(const GeneralPurposeHeap<>& p_source, com::Vector<char>& p_target_data)
		{
			serialize_vector<char>(p_source.memory, p_target_data);
			serialize_pool<GeneralPurposeHeapMemoryChunk>(p_source.allocated_chunks, p_target_data);
			serialize_vector<GeneralPurposeHeapMemoryChunk>(p_source.free_chunks, p_target_data);
		};

		template<class FieldType>
		inline static const FieldType* deserialize_field(size_t& p_current_pointer, const  char* p_source)
		{
			const FieldType* l_field = (const FieldType*)(p_source + p_current_pointer);
			p_current_pointer += sizeof(FieldType);
			return l_field;
		};

		template<class VectorElementType>
		inline static com::Vector<VectorElementType> deserialize_vector(size_t& p_current_pointer, const char* p_source)
		{
			const size_t* l_size = deserialize_field<size_t>(p_current_pointer, p_source);

			com::Vector<VectorElementType> l_return;
			l_return.Memory = (VectorElementType*)(p_source + p_current_pointer);
			l_return.Size = *l_size;
			l_return.Capacity = l_return.Size;

			p_current_pointer += l_return.size_in_bytes();
			
			return l_return;
		};

		template<class PoolElementType>
		inline static com::Pool<PoolElementType> deserialize_pool(size_t& p_current_pointer, const char* p_source)
		{
			com::Pool<PoolElementType> l_pool;
			l_pool.Memory = deserialize_vector<PoolElementType>(p_current_pointer, p_source);
			l_pool.FreeBlocks = deserialize_vector<size_t>(p_current_pointer, p_source);
			return l_pool;
		};

		inline static GeneralPurposeHeap<> deserialize_heap(size_t& p_current_pointer, const char* p_source)
		{
			GeneralPurposeHeap<> l_heap;
			l_heap.memory = deserialize_vector<char>(p_current_pointer, p_source);
			l_heap.allocated_chunks = deserialize_pool<GeneralPurposeHeapMemoryChunk>(p_current_pointer, p_source);
			l_heap.free_chunks = deserialize_vector<GeneralPurposeHeapMemoryChunk>(p_current_pointer, p_source);
			return l_heap;
		};

	};
};