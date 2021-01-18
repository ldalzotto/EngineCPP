#pragma once

namespace v2
{
	struct JSONDeserializer
	{
		struct FieldNode
		{
			Slice<char> value;
			Slice<char> whole_field;
		};

		Slice<char> source;

		Slice<char> parent_cursor;
		Vector<FieldNode> stack_fields;
		size_t current_field;


		inline static JSONDeserializer allocate_default()
		{
			return JSONDeserializer{ Slice<char>::build_default(), Slice<char>::build_default(), Vector<FieldNode>::allocate(0), (size_t)-1 };
		};

		inline static JSONDeserializer allocate(const Slice<char>& p_source, const  Slice<char>& p_parent_cursor)
		{
			return JSONDeserializer{ p_source, p_parent_cursor, Vector<FieldNode>::allocate(0), (size_t)-1 };
		};

		inline static JSONDeserializer start(String& p_source)
		{
			remove_spaces(p_source);
			size_t l_start_index;
			p_source.to_slice().find(slice_char_build_rawstr("{"), &l_start_index);
			l_start_index += 1;
			return allocate(p_source.to_slice(), p_source.to_slice().slide_rv(l_start_index));
		};

		inline void free()
		{
			this->stack_fields.free();
			this->current_field = -1;
		};


		inline char next_field(const char* p_field_name)
		{
			char l_field_found = 0;
			Slice<char> l_next_field_whole_value;
			if (this->find_next_field_whole_value(&l_next_field_whole_value))
			{
				String l_field_name_json = String::allocate(strlen(p_field_name) + 2);
				l_field_name_json.append(slice_char_build_rawstr("\""));
				l_field_name_json.append(slice_char_build_rawstr(p_field_name));
				l_field_name_json.append(slice_char_build_rawstr("\":"));

				if (l_next_field_whole_value.compare(l_field_name_json.to_slice()))
				{
					Slice<char> l_next_field_value_with_quotes = l_next_field_whole_value.slide_rv(l_field_name_json.get_char_nb());

					FieldNode l_field_node;
					size_t l_field_value_delta;
					if (l_next_field_value_with_quotes.slide_rv(1).find(slice_char_build_rawstr("\""), &l_field_value_delta))
					{
						l_field_node.value = l_next_field_value_with_quotes.slide_rv(1);
						l_field_node.value.Size = l_field_value_delta;
						l_field_node.whole_field = l_next_field_whole_value;

						this->stack_fields.push_back_element(l_field_node);
						this->current_field = this->stack_fields.Size - 1;

						l_field_found = 1;
					}
				}

				l_field_name_json.free();
			};


			return l_field_found;
		};


		inline char next_object(const char* p_field_name, JSONDeserializer* out_object_iterator)
		{
			char l_field_found = 0;

			Slice<char> l_compared_slice = this->get_current_slice_cursor();

			String l_field_name_json = String::allocate(strlen(p_field_name) + 2);
			l_field_name_json.append(slice_char_build_rawstr("\""));
			l_field_name_json.append(slice_char_build_rawstr(p_field_name));
			l_field_name_json.append(slice_char_build_rawstr("\":"));

			FieldNode l_field_node;
			if (find_next_json_field(l_compared_slice, l_field_name_json.to_slice(), '{', '}', &l_field_node.whole_field, &l_field_node.value))
			{
				*out_object_iterator = JSONDeserializer::allocate(this->source, l_field_node.value);

				this->stack_fields.push_back_element(l_field_node);
				this->current_field = this->stack_fields.Size - 1;
				l_field_found = 1;
			}

			l_field_name_json.free();

			return l_field_found;
		};


		inline char next_array(const char* p_field_name, JSONDeserializer* out_object_iterator)
		{
			out_object_iterator->free();

			char l_field_found = 0;

			Slice<char> l_compared_slice = this->get_current_slice_cursor();

			String l_field_name_json = String::allocate(strlen(p_field_name) + 2);
			l_field_name_json.append(slice_char_build_rawstr("\""));
			l_field_name_json.append(slice_char_build_rawstr(p_field_name));
			l_field_name_json.append(slice_char_build_rawstr("\":["));

			FieldNode l_field_node;
			if (find_next_json_field(l_compared_slice, l_field_name_json.to_slice(), '[', ']', &l_field_node.whole_field, &l_field_node.value))
			{
				*out_object_iterator = JSONDeserializer::allocate(this->source, l_field_node.value);

				this->stack_fields.push_back_element(l_field_node);
				this->current_field = this->stack_fields.Size - 1;
				l_field_found = 1;
			}

			l_field_name_json.free();

			return l_field_found;
		};

		inline char next_array_object(JSONDeserializer* out_object_iterator)
		{

			out_object_iterator->free();

			char l_field_found = 0;


			Slice<char> l_compared_slice = this->get_current_slice_cursor();

			Slice<char> l_field_name_json_slice = slice_char_build_rawstr("{");

			FieldNode l_field_node;
			if (find_next_json_field(l_compared_slice, l_field_name_json_slice, '{', '}', &l_field_node.whole_field, &l_field_node.value))
			{
				*out_object_iterator = JSONDeserializer::allocate(this->source, l_field_node.value);

				this->stack_fields.push_back_element(l_field_node);
				this->current_field = this->stack_fields.Size - 1;
				l_field_found = 1;
			}
			return l_field_found;
		}


		inline FieldNode& get_currentfield()
		{
			return this->stack_fields.get(this->current_field);
		}

	private:
		inline static void remove_spaces(String& p_source)
		{
			p_source.remove_chars(' ');
			p_source.remove_chars('\n');
			p_source.remove_chars('\r');
			p_source.remove_chars('\t');
		};

		inline FieldNode* get_current_field()
		{
			if (this->current_field != -1)
			{
				return &this->stack_fields.get(this->current_field);
			}
			return NULL;
		};

		inline char find_next_field_whole_value(Slice<char>* out_field_whole_value)
		{

			*out_field_whole_value = this->get_current_slice_cursor();

			// If there is an unexpected character, before the field name.
			// This can occur if the field is in the middle of a JSON Object.
			// This if statement is mendatory because if the field is at the first position of the JSON 
			// object, then there is no unexpected character. So we handle both cases;
			if (out_field_whole_value->Size > 0 &&
				(out_field_whole_value->get(0) == ',' || out_field_whole_value->get(0) == '{'))
			{
				out_field_whole_value->slide(1);
			}
			
			// then we get the next field

			size_t l_new_field_index;

			if (out_field_whole_value->find(slice_char_build_rawstr(","), &l_new_field_index)
				|| out_field_whole_value->find(slice_char_build_rawstr("}"), &l_new_field_index))
			{
				out_field_whole_value->Size = l_new_field_index;
				return 1;
			}

			out_field_whole_value->Size = 0;
			return 0;
		};

		inline Slice<char> get_current_slice_cursor()
		{
			Slice<char> l_compared_slice = this->parent_cursor;
			if (this->current_field != -1)
			{
				for (size_t i = 0; i < this->stack_fields.Size; i++)
				{
					l_compared_slice.slide(this->stack_fields.get(i).whole_field.Size);
					l_compared_slice.slide(1); //for getting after ","
				}
			}
			return l_compared_slice;
		};

		inline static char find_next_json_field(const Slice<char>& p_source, const Slice<char>& p_field_name,
			const char value_begin_delimiter, const char value_end_delimiter,
			Slice<char>* out_object_whole_field, Slice<char>* out_object_value_only)
		{
			if (p_source.compare(p_field_name))
			{
				Slice<char> l_object_value = p_source.slide_rv(p_field_name.Size);

				size_t l_openedbrace_count = 1;
				size_t l_object_string_iterator = 1;
				while (l_openedbrace_count != 0 && l_object_string_iterator < l_object_value.Size)
				{
					if (l_object_value.get(l_object_string_iterator) == value_begin_delimiter)
					{
						l_openedbrace_count += 1;
					}
					else if (l_object_value.get(l_object_string_iterator) == value_end_delimiter)
					{
						l_openedbrace_count -= 1;
					}

					l_object_string_iterator += 1;
				}

				l_object_value.Size = l_object_string_iterator;

				*out_object_value_only = l_object_value;
				// out_object_value_only->Size = l_object_string_iterator - 1;
				*out_object_whole_field = p_source;
				out_object_whole_field->Size = p_field_name.Size + l_object_value.Size;

				return 1;

			}

			return 0;
		};

	};
}

