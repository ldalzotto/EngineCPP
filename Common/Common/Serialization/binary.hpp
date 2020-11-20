#pragma once

#include "Common/Container/vector.hpp"

namespace Serialization
{
	struct Binary
	{
		template<class FieldType>
		inline static void serialize_field(size_t& p_current_pointer, const char* p_source, com::Vector<char>& p_target_data)
		{
			p_target_data.insert_at(com::MemorySlice<char>(p_source + p_current_pointer, sizeof(FieldType)), p_target_data.Size);
			p_current_pointer += sizeof(FieldType);
		};

		template<class VectorElementType>
		inline static void serialize_vector(size_t& p_current_pointer, const com::Vector<VectorElementType>& p_source, com::Vector<char>& p_target_data)
		{
			p_target_data.insert_at(com::MemorySlice<char>((const char*)&p_source.Size, sizeof(size_t)), p_target_data.Size);
			p_target_data.insert_at(com::MemorySlice<char>((const char*)p_source.Memory, p_source.size_in_bytes()), p_target_data.Size);
			p_current_pointer += (sizeof(size_t) + sizeof(p_source));
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
	};
};