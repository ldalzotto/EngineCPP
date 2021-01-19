#pragma once

namespace v2
{
	/*
		A String is a vector of int8 that always have a NULL int8acter at it's last element.
	*/
	struct String
	{
		Vector<int8> Memory;

		inline static String allocate(const uimax p_initial_capacity)
		{
			int8 l_null_int8 = (int8)NULL;
			return String{ Vector<int8>::allocate_capacity_elements(p_initial_capacity + 1, Slice<int8>::build_asint8_memory_elementnb(&l_null_int8, 1)) };
		};

		inline static String allocate_elements(const Slice<int8>& p_initial_elements)
		{
			String l_string = String{ Vector<int8>::allocate_capacity_elements(p_initial_elements.Size + 1, p_initial_elements) };
			l_string.Memory.push_back_element((int8)NULL);
			return l_string;
		};

		inline void free()
		{
			this->Memory.free();
		};

		inline void append(const Slice<int8>& p_elements)
		{
			this->Memory.insert_array_at(p_elements, this->Memory.Size - 1);
		};

		inline void insert_array_at(const Slice<int8>& p_elements, const uimax p_index)
		{
			// The insert_array_at will fail if p_index == this->get_size();
			this->Memory.insert_array_at(p_elements, p_index);
		};

		inline int8& get(const uimax p_index)
		{
			return this->Memory.get(p_index);
		};

		inline const int8& get(const uimax p_index) const
		{
			return ((String*)this)->Memory.get(p_index);
		};

		inline int8* get_memory()
		{
			return this->Memory.get_memory();
		};

		inline uimax get_size() const
		{
			return this->Memory.Size;
		};

		inline uimax get_int8_nb() const
		{
			return this->Memory.Size - 1;
		};

		inline void clear()
		{
			this->Memory.clear();
			this->Memory.push_back_element((int8)NULL);
		};

		inline Slice<int8> to_slice()
		{
			return Slice<int8>::build_memory_elementnb(this->Memory.Memory.Memory, this->Memory.Size - 1);
		};

		inline void remove_int8s(const int8 p_int8)
		{
			for (vector_loop_reverse(&this->Memory, i))
			{
				if (this->Memory.get(i) == p_int8)
				{
					this->Memory.erase_element_at(i);
				}
			}
		}
	};
}