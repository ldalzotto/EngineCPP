#pragma once

namespace v2
{
	struct JSONDeserializer
	{
		struct FieldNode
		{
			Slice<int8> value;
			Slice<int8> whole_field;
		};

		Slice<int8> source;

		Slice<int8> parent_cursor;
		Vector<FieldNode> stack_fields;
		uimax current_field;


		inline static JSONDeserializer allocate_default()
		{
			return JSONDeserializer{ Slice<int8>::build_default(), Slice<int8>::build_default(), Vector<FieldNode>::allocate(0), (uimax)-1 };
		};

		inline static JSONDeserializer allocate(const Slice<int8>& p_source, const  Slice<int8>& p_parent_cursor)
		{
			return JSONDeserializer{ p_source, p_parent_cursor, Vector<FieldNode>::allocate(0), (uimax)-1 };
		};

		inline static JSONDeserializer start(String& p_source)
		{
			remove_spaces(p_source);
			uimax l_start_index;
			p_source.to_slice().find(slice_int8_build_rawstr("{"), &l_start_index);
			l_start_index += 1;
			return allocate(p_source.to_slice(), p_source.to_slice().slide_rv(l_start_index));
		};

		inline void free()
		{
			this->stack_fields.free();
			this->current_field = -1;
		};


		inline int8 next_field(const int8* p_field_name)
		{
			int8 l_field_found = 0;
			Slice<int8> l_next_field_whole_value;
			if (this->find_next_field_whole_value(&l_next_field_whole_value))
			{
				String l_field_name_json = String::allocate(strlen(p_field_name) + 2);
				l_field_name_json.append(slice_int8_build_rawstr("\""));
				l_field_name_json.append(slice_int8_build_rawstr(p_field_name));
				l_field_name_json.append(slice_int8_build_rawstr("\":"));

				if (l_next_field_whole_value.compare(l_field_name_json.to_slice()))
				{
					Slice<int8> l_next_field_value_with_quotes = l_next_field_whole_value.slide_rv(l_field_name_json.get_int8_nb());

					FieldNode l_field_node;
					uimax l_field_value_delta;
					if (l_next_field_value_with_quotes.slide_rv(1).find(slice_int8_build_rawstr("\""), &l_field_value_delta))
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


		inline int8 next_object(const int8* p_field_name, JSONDeserializer* out_object_iterator)
		{
			int8 l_field_found = 0;

			Slice<int8> l_compared_slice = this->get_current_slice_cursor();

			String l_field_name_json = String::allocate(strlen(p_field_name) + 2);
			l_field_name_json.append(slice_int8_build_rawstr("\""));
			l_field_name_json.append(slice_int8_build_rawstr(p_field_name));
			l_field_name_json.append(slice_int8_build_rawstr("\":"));

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


		inline int8 next_array(const int8* p_field_name, JSONDeserializer* out_object_iterator)
		{
			out_object_iterator->free();

			int8 l_field_found = 0;

			Slice<int8> l_compared_slice = this->get_current_slice_cursor();

			String l_field_name_json = String::allocate(strlen(p_field_name) + 2);
			l_field_name_json.append(slice_int8_build_rawstr("\""));
			l_field_name_json.append(slice_int8_build_rawstr(p_field_name));
			l_field_name_json.append(slice_int8_build_rawstr("\":["));

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

		inline int8 next_array_object(JSONDeserializer* out_object_iterator)
		{

			out_object_iterator->free();

			int8 l_field_found = 0;


			Slice<int8> l_compared_slice = this->get_current_slice_cursor();

			Slice<int8> l_field_name_json_slice = slice_int8_build_rawstr("{");

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
			p_source.remove_int8s(' ');
			p_source.remove_int8s('\n');
			p_source.remove_int8s('\r');
			p_source.remove_int8s('\t');
		};

		inline FieldNode* get_current_field()
		{
			if (this->current_field != -1)
			{
				return &this->stack_fields.get(this->current_field);
			}
			return NULL;
		};

		inline int8 find_next_field_whole_value(Slice<int8>* out_field_whole_value)
		{

			*out_field_whole_value = this->get_current_slice_cursor();

			// If there is an unexpected int8acter, before the field name.
			// This can occur if the field is in the middle of a JSON Object.
			// This if statement is mendatory because if the field is at the first position of the JSON 
			// object, then there is no unexpected int8acter. So we handle both cases;
			if (out_field_whole_value->Size > 0 &&
				(out_field_whole_value->get(0) == ',' || out_field_whole_value->get(0) == '{'))
			{
				out_field_whole_value->slide(1);
			}
			
			// then we get the next field

			uimax l_new_field_index;

			if (out_field_whole_value->find(slice_int8_build_rawstr(","), &l_new_field_index)
				|| out_field_whole_value->find(slice_int8_build_rawstr("}"), &l_new_field_index))
			{
				out_field_whole_value->Size = l_new_field_index;
				return 1;
			}

			out_field_whole_value->Size = 0;
			return 0;
		};

		inline Slice<int8> get_current_slice_cursor()
		{
			Slice<int8> l_compared_slice = this->parent_cursor;
			if (this->current_field != -1)
			{
				for (uimax i = 0; i < this->stack_fields.Size; i++)
				{
					l_compared_slice.slide(this->stack_fields.get(i).whole_field.Size);
					l_compared_slice.slide(1); //for getting after ","
				}
			}
			return l_compared_slice;
		};

		inline static int8 find_next_json_field(const Slice<int8>& p_source, const Slice<int8>& p_field_name,
			const int8 value_begin_delimiter, const int8 value_end_delimiter,
			Slice<int8>* out_object_whole_field, Slice<int8>* out_object_value_only)
		{
			if (p_source.compare(p_field_name))
			{
				Slice<int8> l_object_value = p_source.slide_rv(p_field_name.Size);

				uimax l_openedbrace_count = 1;
				uimax l_object_string_iterator = 1;
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

