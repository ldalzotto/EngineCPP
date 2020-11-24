#pragma once

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
			size_t start_index = -1;
		};

		struct ArrayNode
		{

		};

		struct JSONObjectIterator
		{
			String<> source;

			ObjectNode object;
			com::Vector<FieldNode> stack_fields;

			size_t current_field = -1;

			inline void free()
			{
				this->stack_fields.free();
				this->current_field = -1;
			};

			inline bool next_field(const char* p_field_name)
			{
				bool l_field_found = false;
				size_t l_lastfieldname_index = this->get_current_pointer();

				if (l_lastfieldname_index != -1)
				{
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

								l_field_found = true;
							}
						}
					}

					l_field_name_json.free();
				}

				return l_field_found;
			};

			inline bool next_object(const char* p_field_name, JSONObjectIterator* out_object_iterator)
			{
				bool l_field_found = false;
				size_t l_lastfieldname_index = this->get_current_pointer();

				if (l_lastfieldname_index != -1)
				{
					String<> l_field_name_json; l_field_name_json.allocate(strlen(p_field_name) + 2);
					l_field_name_json.append("\"");
					l_field_name_json.append(p_field_name);
					l_field_name_json.append("\":");

					if (l_field_name_json.equals(StringSlice(this->source.Memory.Memory, l_lastfieldname_index, l_lastfieldname_index + l_field_name_json.Memory.Size - 1)))
					{
						size_t l_currentfieldvalue_index = l_lastfieldname_index + l_field_name_json.Memory.Size - 1;

						if (is_value_an_object(l_currentfieldvalue_index))
						{
							out_object_iterator->source = this->source;


							size_t l_openedbrace_count = 1;
							size_t l_object_string_iterator = l_currentfieldvalue_index + 1;
							while (l_openedbrace_count != 0)
							{
								if (this->source.Memory[l_object_string_iterator] == '{')
								{
									l_openedbrace_count += 1;
								}
								else if (this->source.Memory[l_object_string_iterator] == '}')
								{
									l_openedbrace_count -= 1;
								}

								l_object_string_iterator += 1;
							}

							FieldNode l_field_node;

							size_t l_fieldvalue_start = l_currentfieldvalue_index;
							l_field_node.value = StringSlice(this->source.Memory.Memory, l_fieldvalue_start, l_object_string_iterator - 1);
							l_field_node.whole_field = StringSlice(this->source.Memory.Memory, l_lastfieldname_index, l_object_string_iterator);

							out_object_iterator->object.start_index = l_currentfieldvalue_index + 1;

							this->stack_fields.push_back(l_field_node);
							this->current_field = this->stack_fields.Size - 1;

							l_field_found = true;
						}
					}
					l_field_name_json.free();
				}

				return l_field_found;
			};

			inline bool next_array_object(JSONObjectIterator* out_object_iterator)
			{
				out_object_iterator->free();

				bool l_field_found = false;
				size_t l_lastfieldname_index = this->get_current_pointer();

				if (l_lastfieldname_index != -1)
				{
					if (!is_value_an_array_end(l_lastfieldname_index))
					{
						if (is_value_an_object(l_lastfieldname_index))
						{
							size_t l_currentfieldvalue_index = l_lastfieldname_index + 1;


							size_t l_openedbrace_count = 1;
							size_t l_object_string_iterator = l_currentfieldvalue_index + 1;
							while (l_openedbrace_count != 0)
							{
								if (this->source.Memory[l_object_string_iterator] == '{')
								{
									l_openedbrace_count += 1;
								}
								else if (this->source.Memory[l_object_string_iterator] == '}')
								{
									l_openedbrace_count -= 1;
								}

								l_object_string_iterator += 1;
							}

							FieldNode l_field_node;

							size_t l_fieldvalue_start = l_currentfieldvalue_index;
							l_field_node.value = StringSlice(this->source.Memory.Memory, l_fieldvalue_start, l_object_string_iterator - 1);
							l_field_node.whole_field = StringSlice(this->source.Memory.Memory, l_lastfieldname_index, l_object_string_iterator);

							out_object_iterator->source = this->source;
							out_object_iterator->object.start_index = l_currentfieldvalue_index;

							this->stack_fields.push_back(l_field_node);
							this->current_field = this->stack_fields.Size - 1;

							l_field_found = true;
						}
					}
				}

				return l_field_found;
			}

			inline bool next_array(const char* p_field_name, JSONObjectIterator* out_object_iterator)
			{
				out_object_iterator->free();

				bool l_field_found = false;
				size_t l_lastfieldname_index = this->get_current_pointer();

				if (l_lastfieldname_index != -1)
				{
					String<> l_field_name_json; l_field_name_json.allocate(strlen(p_field_name) + 2);
					l_field_name_json.append("\"");
					l_field_name_json.append(p_field_name);
					l_field_name_json.append("\":");

					if (l_field_name_json.equals(StringSlice(this->source.Memory.Memory, l_lastfieldname_index, l_lastfieldname_index + l_field_name_json.Memory.Size - 1)))
					{
						size_t l_currentfieldvalue_index = l_lastfieldname_index + l_field_name_json.Memory.Size - 1;

						if (is_value_an_array(l_currentfieldvalue_index))
						{

							size_t l_openedbrace_count = 1;
							size_t l_object_string_iterator = l_currentfieldvalue_index + 1;
							while (l_openedbrace_count != 0)
							{
								if (this->source.Memory[l_object_string_iterator] == '[')
								{
									l_openedbrace_count += 1;
								}
								else if (this->source.Memory[l_object_string_iterator] == ']')
								{
									l_openedbrace_count -= 1;
								}

								l_object_string_iterator += 1;
							}

							FieldNode l_field_node;

							size_t l_fieldvalue_start = l_currentfieldvalue_index + 1;
							l_field_node.value = StringSlice(this->source.Memory.Memory, l_fieldvalue_start, l_object_string_iterator - 1);
							l_field_node.whole_field = StringSlice(this->source.Memory.Memory, l_lastfieldname_index, l_object_string_iterator);


							out_object_iterator->source = this->source;
							out_object_iterator->object.start_index = l_currentfieldvalue_index + 1;

							this->stack_fields.push_back(l_field_node);
							this->current_field = this->stack_fields.Size - 1;

							l_field_found = true;
						}
					}

					l_field_name_json.free();
				}

				return l_field_found;
			};

			inline FieldNode& get_currentfield()
			{
				return this->stack_fields[this->current_field];
			}

		private:
			inline size_t get_current_pointer()
			{
				size_t l_lastfieldname_index = this->object.start_index;

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
						return -1;
					}
				}

				return l_lastfieldname_index;

			}

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
				return this->source.Memory[p_start_index] == '[';
			};

			inline bool is_value_an_array_end(const size_t p_start_index)
			{
				return this->source.Memory[p_start_index] == ']';
			};
		};

		inline static JSONObjectIterator StartDeserialization(const String<>& p_source)
		{
			JSONObjectIterator l_object_iterator;

			p_source.find("{", 0, &l_object_iterator.object.start_index);
			l_object_iterator.object.start_index += 1;

			l_object_iterator.source = p_source;

			return l_object_iterator;
		};
	};
}

template<class ElementType>
struct JSONDeserializer
{
	static ElementType deserialize(Serialization::JSON::JSONObjectIterator& p_iterator);
};

template<>
struct JSONDeserializer<int>
{
	static int deserialize(Serialization::JSON::JSONObjectIterator& p_iterator)
	{
		String<> l_str; l_str.allocate(30);
		l_str.append(p_iterator.get_currentfield().value);
		int l_return = atoi(l_str.c_str());
		l_str.free();
		return l_return;
	};
};

template<>
struct JSONDeserializer<short>
{
	static short deserialize(Serialization::JSON::JSONObjectIterator& p_iterator)
	{
		String<> l_str; l_str.allocate(30);
		l_str.append(p_iterator.get_currentfield().value);
		short l_return = (short)atoi(l_str.c_str());
		l_str.free();
		return l_return;
	};
};

template<>
struct JSONDeserializer<float>
{
	static float deserialize(Serialization::JSON::JSONObjectIterator& p_iterator)
	{
		String<> l_str; l_str.allocate(30);
		l_str.append(p_iterator.get_currentfield().value);
		float l_return = (float)atof(l_str.c_str());
		l_str.free();
		return l_return;
	};
};

template<>
struct JSONDeserializer<String<>>
{
	static String<> deserialize(Serialization::JSON::JSONObjectIterator& p_iterator)
	{
		StringSlice& l_value = p_iterator.get_currentfield().value;
		String<> l_str; l_str.allocate(l_value.size());
		l_str.append(l_value);
		return l_str;
	};
};