#pragma once

namespace v2
{
	/*
		A String is a vector of char that always have a NULL character at it's last element.
	*/
	struct String
	{
		Vector<char> Memory;

		inline static String allocate(const size_t p_initial_capacity)
		{
			char l_null_char = (char)NULL;
			return String{ Vector<char>::allocate_capacity_elements(p_initial_capacity + 1, Slice<char>::build_aschar_memory_elementnb(&l_null_char, 1)) };
		};

		inline static String allocate_elements(const Slice<char>& p_initial_elements)
		{
			String l_string = String{ Vector<char>::allocate_capacity_elements(p_initial_elements.Size + 1, p_initial_elements) };
			l_string.Memory.push_back_element((char)NULL);
			return l_string;
		};

		inline void free()
		{
			this->Memory.free();
		};

		inline void append(const Slice<char>& p_elements)
		{
			this->Memory.insert_array_at(p_elements, this->Memory.Size - 1);
		};

		inline void insert_array_at(const Slice<char>& p_elements, const size_t p_index)
		{
			// The insert_array_at will fail if p_index == this->get_size();
			this->Memory.insert_array_at(p_elements, p_index);
		};

		inline char& get(const size_t p_index)
		{
			return this->Memory.get(p_index);
		};

		inline const char& get(const size_t p_index) const
		{
			return ((String*)this)->Memory.get(p_index);
		};

		inline char* get_memory()
		{
			return this->Memory.get_memory();
		};

		inline size_t get_size() const
		{
			return this->Memory.Size;
		};

		inline size_t get_char_nb() const
		{
			return this->Memory.Size - 1;
		};

		inline void clear()
		{
			this->Memory.clear();
			this->Memory.push_back_element((char)NULL);
		};

		inline Slice<char> to_slice()
		{
			return Slice<char>::build_memory_elementnb(this->Memory.Memory.Memory, this->Memory.Size - 1);
		};

		inline void remove_chars(const char p_char)
		{
			for (size_t i = this->Memory.Size - 1; i != -1; --i)
			{
				if (this->Memory.get(i) == p_char)
				{
					this->Memory.erase_element_at(i);
				}
			}
		}
	};
}