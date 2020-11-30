#pragma once

#include "Common/Container/vector.hpp"
#include "Common/Container/string.hpp"

namespace Deserialization
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

			inline JSONObjectIterator clone()
			{
				JSONObjectIterator l_return;
				l_return.source = this->source;
				l_return.object = this->object;
				l_return.stack_fields = this->stack_fields.clone();
				l_return.current_field = this->current_field;
				return l_return;
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

			template<class ElementType>
			inline bool next_field(const char* p_field_name, ElementType* out_value)
			{
				if (this->next_field(p_field_name))
				{
					*out_value = JSONDeserializer<ElementType>::deserialize(*this);
					return true;
				}
				return false;
			}

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

namespace Serialization
{
	struct JSON
	{
		struct Deserializer
		{
			String<> output;
			size_t current_indentation = 0;

			inline void allocate() 
			{
				this->output.allocate(0);
				this->current_indentation = 0;
			}

			inline void free() 
			{
				this->output.free();
				this->current_indentation = 0;
			}

			inline void start() 
			{
				this->output.append("{\n");
				this->current_indentation += 1;
			};

			inline void end()
			{
				this->remove_last_coma();
				this->output.append("}");
				this->current_indentation -= 1;
			};

			inline void push_field(const StringSlice& p_name,const StringSlice& p_value)
			{
				this->push_indentation();
				this->output.append("\"");
				this->output.append(p_name);
				this->output.append("\": \"");
				this->output.append(p_value);
				this->output.append("\",\n");
			};

			template<class ElementType>
			inline void push_field(const StringSlice& p_name, const ElementType& p_value)
			{
				String<> l_str = ToString<ElementType>::to_str(p_value);
				this->push_field(p_name, l_str.toSlice());
				l_str.free();
			};

			inline void start_object(const StringSlice& p_name)
			{
				this->push_indentation();
				this->output.append("\"");
				this->output.append(p_name);
				this->output.append("\": {\n");
				this->current_indentation += 1;
			};

			inline void start_object()
			{
				this->push_indentation();
				this->output.append("{\n");
				this->current_indentation += 1;
			};

			inline void end_object()
			{
				this->remove_last_coma();
				this->current_indentation -= 1;
				this->push_indentation();
				this->output.append("},\n");
			};

			inline void start_array(const StringSlice& p_name)
			{
				this->push_indentation();
				this->output.append("\"");
				this->output.append(p_name);
				this->output.append("\": [\n");
				this->current_indentation += 1;
			};

			inline void end_array()
			{
				this->remove_last_coma();
				this->current_indentation -= 1;
				this->push_indentation();
				this->output.append("],\n");
			};

		private:
			void push_indentation()
			{
				String<> l_indentation;
				l_indentation.allocate(this->current_indentation);
				for (size_t i = 0; i < this->current_indentation; i++)
				{
					l_indentation.append(" ");
				}
				this->output.append(l_indentation);
				l_indentation.free();
			};

			void remove_last_coma()
			{
				if (this->output.Memory[this->output.Memory.Size - 1 - 2] == ',')
				{
					this->output.remove(this->output.Memory.Size - 1 - 2, this->output.Memory.Size - 1 - 1);
				}
			};
		};

	};
}

template<class ElementType>
struct JSONDeserializer
{
	static ElementType deserialize(Deserialization::JSON::JSONObjectIterator& p_iterator);
};

#define DECLARE_JSON_DESERIALIAZER(ElementType) \
template<> \
struct JSONDeserializer<ElementType> \
{ \
	inline static ElementType deserialize(Deserialization::JSON::JSONObjectIterator& p_iterator) \
	{ \
		return FromString<ElementType>::from_str(p_iterator.get_currentfield().value); \
	}; \
};

DECLARE_JSON_DESERIALIAZER(size_t);
DECLARE_JSON_DESERIALIAZER(int);
DECLARE_JSON_DESERIALIAZER(short);
DECLARE_JSON_DESERIALIAZER(bool);
DECLARE_JSON_DESERIALIAZER(float);
DECLARE_JSON_DESERIALIAZER(String<>);

template<class ElementType>
struct JSONSerializer
{
	static void serialize(Serialization::JSON::Deserializer& p_serializer, const ElementType& p_object);
};

#define DECLARE_JSON_SERIALIZER(ElementType) \
template<> \
struct JSONSerializer<ElementType> \
{ \
	inline static void serialize(Serialization::JSON::Deserializer& p_serializer, const StringSlice& p_name, const ElementType& p_object) \
	{ \
		String<> l_object_str = ToString<ElementType>::to_str(p_object); \
		p_serializer.push_field(p_name, l_object_str.toSlice()); \
		l_object_str.free(); \
	}; \
};

DECLARE_JSON_SERIALIZER(int);
DECLARE_JSON_SERIALIZER(short);
DECLARE_JSON_SERIALIZER(float);