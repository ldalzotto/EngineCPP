#pragma once

/*
	A Slice is an encapsulated C style array.
*/
template<class ElementType>
struct Slice
{
	size_t Size;
	ElementType* Begin;

	inline static Slice<ElementType> build_default()
	{
		return Slice<ElementType>{0, NULL};
	};

	inline static Slice<ElementType> build(ElementType* p_memory, const size_t p_begin, const size_t p_end)
	{
		return Slice<ElementType>{p_end - p_begin, p_memory + p_begin};
	};

	inline static Slice<ElementType> build_memory_elementnb(ElementType* p_memory, const size_t p_element_nb)
	{
		return Slice<ElementType>{p_element_nb, p_memory};
	};

	inline static Slice<ElementType> build_memory_offset_elementnb(ElementType* p_memory, const size_t p_offset, const size_t p_element_nb)
	{
		return Slice<ElementType>{p_element_nb, p_memory + p_offset};
	};

	inline static Slice<char> build_aschar(ElementType* p_memory, const size_t p_begin, const size_t p_end)
	{
		return Slice<char>{sizeof(ElementType)* (p_end - p_begin), cast(char*, (p_memory + p_begin))};
	};

	inline Slice<char> build_aschar() const
	{
		return Slice<char>{sizeof(ElementType)* this->Size, cast(char*, this->Begin)};
	};

	inline static Slice<char> build_aschar_memory_elementnb(const ElementType* p_memory, const size_t p_element_nb)
	{
		return Slice<char>{sizeof(ElementType)* p_element_nb, cast(char*, p_memory)};
	};

	inline static Slice<char> build_aschar_memory_singleelement(const ElementType* p_memory)
	{
		return Slice<char>{sizeof(ElementType), cast(char*, p_memory)};
	};

	inline ElementType& get(const size_t p_index)
	{
#if CONTAINER_BOUND_TEST
		if(p_index >= this->Size)
		{
			abort();
		}
#endif
		return this->Begin[p_index];
	};

	inline const ElementType& get(const size_t p_index) const
	{
		return ((Slice<ElementType>*)this)->get(p_index);
	};

	inline void slide(const size_t p_offset_index)
	{
#if CONTAINER_BOUND_TEST
		if (p_offset_index > this->Size)
		{
			abort();
		};
#endif

		this->Begin = this->Begin + p_offset_index;
		this->Size -= p_offset_index;
	};

	inline Slice<ElementType> slide_rv(const size_t p_offset_index) const
	{
		Slice<ElementType> l_return = *this;
		l_return.slide(p_offset_index);
		return l_return;
	};

	inline char compare(const Slice<ElementType>& p_other) const
	{
		return slice_memcompare_element(*this, p_other);
	};

	inline char find(const Slice<ElementType>& p_other, size_t* out_index) const
	{
		return slice_memfind(*this, p_other, out_index);
	};
};

inline Slice<char> slice_char_build_rawstr(const char* p_str)
{
	return Slice<char>::build_memory_elementnb((char*)p_str, strlen(p_str));
};


template<class CastedType>
inline Slice<CastedType> slice_cast(const Slice<char>& p_slice)
{
#if CONTAINER_BOUND_TEST
	if ((p_slice.Size % sizeof(CastedType)) != 0)
	{
		abort();
	}
#endif

	return Slice<CastedType>{ cast(size_t, p_slice.Size / sizeof(CastedType)), cast(CastedType*, p_slice.Begin) };
};


template<class CastedType>
inline CastedType* slice_cast_singleelement(const Slice<char>& p_slice)
{
#if CONTAINER_BOUND_TEST
	if (p_slice.Size < sizeof(CastedType))
	{
		abort();
	}
#endif
	return cast(CastedType*, p_slice.Begin);
};

template<class CastedType>
inline Slice<CastedType> slice_cast_fixedelementnb(const Slice<char>& p_slice, const size_t p_element_nb)
{
#if CONTAINER_BOUND_TEST
	if (p_slice->Size < (sizeof(CastedType) * p_element_nb))
	{
		abort();
	}
#endif

	return slice_build_memory_elementnb(cast(CastedType*, p_slice->Begin), p_element_nb);
};





template<class ElementType>
inline char* slice_memmove(const Slice<ElementType>& p_target, const Slice<ElementType>& p_source)
{
#if STANDARD_ALLOCATION_BOUND_TEST
	return memory_move_safe(cast(char*, p_target.Begin), p_target.Size * sizeof(ElementType), cast(char*, p_source.Begin), p_source.Size * sizeof(ElementType));
#else
	return memory_move((char*)p_target.Begin, (char*)p_source.Begin, p_source.Size * sizeof(ElementType));
#endif
};

template<class ElementType>
inline char* slice_memcpy(const Slice<ElementType>& p_target, const Slice<ElementType>& p_source)
{
#if STANDARD_ALLOCATION_BOUND_TEST
	return memory_cpy_safe(cast(char*, p_target.Begin), p_target.Size *sizeof(ElementType), cast(char*, p_source.Begin), p_source.Size * sizeof(ElementType));
#else
	return memory_cpy((char*)p_target.Begin, (char*)p_source.Begin, p_source.Size * sizeof(ElementType));
#endif
};

template<class ElementType>
inline char slice_memcompare_element(const Slice<ElementType>& p_target, const Slice<ElementType>& p_compared)
{
	return memory_compare(cast(char*, p_target.Begin), cast(char*, p_compared.Begin), p_compared.Size);
};

template<class ElementType>
inline char slice_memfind(const Slice<ElementType>& p_target, const Slice<ElementType>& p_compared, size_t* out_index)
{
#if CONTAINER_BOUND_TEST
	if (p_compared.Size > p_target.Size)
	{
		abort();
	}
#endif

	Slice<ElementType> l_target_slice = p_target;
	if (slice_memcompare_element(l_target_slice, p_compared))
	{
		*out_index = 0;
		return 1;
	};

	for (size_t i = 1; i < p_target.Size - p_compared.Size; i++)
	{
		l_target_slice.slide(1);
		if (slice_memcompare_element(l_target_slice, p_compared))
		{
			*out_index = i;
			return 1;
		};
	};

	return 0;
};

/*
	A SliceIndex is just a begin and end size_t
*/
struct SliceIndex
{
	size_t Begin;
	size_t Size;

	inline static SliceIndex build(const size_t p_begin, const size_t p_size)
	{
		return SliceIndex{ p_begin, p_size };
	};

	inline static SliceIndex build_default()
	{
		return build(0, 0);
	};

	inline void slice_two(const size_t p_break_point, SliceIndex* out_left, SliceIndex* out_right) const
	{
		size_t l_source_initial_size = this->Size;
		*out_left = SliceIndex::build(this->Begin, p_break_point - this->Begin);
		*out_right = SliceIndex::build(p_break_point, l_source_initial_size - out_left->Size);
	};
};