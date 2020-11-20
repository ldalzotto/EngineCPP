
#include "Common/Container/vector.hpp"
#include "Common/Container/string.hpp"

namespace Serialization
{
	struct JSON
	{

		struct FieldNode
		{
			StringSlice value;
			StringSlice whole_field;
		};

		struct ObjectNode
		{
			size_t start_index;
		};

		struct ArrayNode
		{

		};

		struct DeserializeIterator
		{
			String<> source;

			com::Vector<ObjectNode> stack_objects;
			com::Vector<ArrayNode> stack_arrays;
			com::Vector<FieldNode> stack_fields;

			size_t current_array = -1;
			size_t current_object = -1;
			size_t current_field = -1;

			inline void start(const String<>& p_source)
			{
				this->source = p_source;

				ObjectNode l_root_object;
				this->source.find("{", 0, &l_root_object.start_index);
				l_root_object.start_index += 1;

				this->stack_objects.push_back(l_root_object);
				this->current_object = this->stack_objects.Size - 1;
			};

			inline void free()
			{
				this->stack_objects.free();
				this->stack_arrays.free();
				this->stack_fields.free();
			};

			inline bool next_field(const char* p_field_name)
			{
				size_t l_lastfieldname_index;

				if (current_field != -1)
				{
					const FieldNode& l_current_field = this->stack_fields[this->current_field];
					//is there is another field
					if (this->source.Memory[l_current_field.whole_field.End] == ',')
					{
						l_lastfieldname_index = l_current_field.whole_field.End + 1;
					}
					else
					{
						return false;
					}
				}
				else if (current_object != -1)
				{
					const ObjectNode& l_current_object = this->stack_objects[this->current_object];
					l_lastfieldname_index = l_current_object.start_index;
				}
				
				String<> l_field_name_json; l_field_name_json.allocate(strlen(p_field_name) + 2);
				l_field_name_json.append("\"");
				l_field_name_json.append(p_field_name);
				l_field_name_json.append("\":");

				if (l_field_name_json.equals(StringSlice(this->source.Memory.Memory, l_lastfieldname_index, l_lastfieldname_index + l_field_name_json.Memory.Size - 1)))
				{
					size_t l_currentfieldvalue_index = l_lastfieldname_index + l_field_name_json.Memory.Size - 1;
					
					if (is_value_a_field(l_currentfieldvalue_index))
					{
						FieldNode l_field_node;

						size_t l_fieldvalue_start = l_currentfieldvalue_index + 1;
						if (this->source.find("\"", l_fieldvalue_start, &l_currentfieldvalue_index))
						{
							l_field_node.value = StringSlice(this->source.Memory.Memory, l_fieldvalue_start, l_currentfieldvalue_index);
							l_field_node.whole_field = StringSlice(this->source.Memory.Memory, l_lastfieldname_index, l_currentfieldvalue_index + 1);

							this->stack_fields.push_back(l_field_node);
							this->current_field = this->stack_fields.Size - 1;


							return true;
						}
					}
					else if (is_value_an_object(l_currentfieldvalue_index))
					{
						//TODO - we skip the object
					}
					else if (is_value_an_array(l_currentfieldvalue_index))
					{
						//TODO - we skip the array
					}

				}

				return false;
			};

		private:
			inline bool is_value_a_field(const size_t p_start_index)
			{
				return this->source.Memory[p_start_index] == '"';
			};

			inline bool is_value_an_object(const size_t p_start_index)
			{
				return this->source.Memory[p_start_index] == '{';
			};

			inline bool is_value_an_array(const size_t p_start_index)
			{
				return this->source.Memory[p_start_index] == '{';
			};
		};
	};
}