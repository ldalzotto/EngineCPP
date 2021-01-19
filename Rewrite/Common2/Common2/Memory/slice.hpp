#pragma once

/*
	A Slice is an encapsulated C style array.
*/
template<class ElementType>
struct Slice
{
	uimax Size;
	ElementType* Begin;

	inline static Slice<ElementType> build_default()
	{
		return Slice<ElementType>{0, NULL};
	};

	inline static Slice<ElementType> build(ElementType* p_memory, const uimax p_begin, const uimax p_end)
	{
		return Slice<ElementType>{p_end - p_begin, p_memory + p_begin};
	};

	inline static Slice<ElementType> build_memory_elementnb(ElementType* p_memory, const uimax p_element_nb)
	{
		return Slice<ElementType>{p_element_nb, p_memory};
	};

	inline static Slice<ElementType> build_memory_offset_elementnb(ElementType* p_memory, const uimax p_offset, const uimax p_element_nb)
	{
		return Slice<ElementType>{p_element_nb, p_memory + p_offset};
	};

	inline static Slice<int8> build_asint8(ElementType* p_memory, const uimax p_begin, const uimax p_end)
	{
		return Slice<int8>{sizeof(ElementType)* (p_end - p_begin), cast(int8*, (p_memory + p_begin))};
	};

	inline Slice<int8> build_asint8() const
	{
		return Slice<int8>{sizeof(ElementType)* this->Size, cast(int8*, this->Begin)};
	};

	inline static Slice<int8> build_asint8_memory_elementnb(const ElementType* p_memory, const uimax p_element_nb)
	{
		return Slice<int8>{sizeof(ElementType)* p_element_nb, cast(int8*, p_memory)};
	};

	inline static Slice<int8> build_asint8_memory_singleelement(const ElementType* p_memory)
	{
		return Slice<int8>{sizeof(ElementType), cast(int8*, p_memory)};
	};

	inline ElementType& get(const uimax p_index)
	{
#if CONTAINER_BOUND_TEST
		if (p_index >= this->Size)
		{
			abort();
		}
#endif
		return this->Begin[p_index];
	};

	inline const ElementType& get(const uimax p_index) const
	{
		return ((Slice<ElementType>*)this)->get(p_index);
	};

	inline void slide(const uimax p_offset_index)
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

	inline Slice<ElementType> slide_rv(const uimax p_offset_index) const
	{
		Slice<ElementType> l_return = *this;
		l_return.slide(p_offset_index);
		return l_return;
	};

	inline int8 compare(const Slice<ElementType>& p_other) const
	{
		return slice_memcompare_element(*this, p_other);
	};

	inline int8 find(const Slice<ElementType>& p_other, uimax* out_index) const
	{
		return slice_memfind(*this, p_other, out_index);
	};
};

inline Slice<int8> slice_int8_build_rawstr(const int8* p_str)
{
	return Slice<int8>::build_memory_elementnb((int8*)p_str, strlen(p_str));
};


template<class CastedType>
inline Slice<CastedType> slice_cast(const Slice<int8>& p_slice)
{
#if CONTAINER_BOUND_TEST
	if ((p_slice.Size % sizeof(CastedType)) != 0)
	{
		abort();
	}
#endif

	return Slice<CastedType>{ cast(uimax, p_slice.Size / sizeof(CastedType)), cast(CastedType*, p_slice.Begin) };
};


template<class CastedType>
inline CastedType* slice_cast_singleelement(const Slice<int8>& p_slice)
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
inline Slice<CastedType> slice_cast_fixedelementnb(const Slice<int8>& p_slice, const uimax p_element_nb)
{
#if CONTAINER_BOUND_TEST
	if (p_slice->Size < (sizeof(CastedType) * p_element_nb))
	{
		abort();
	}
#endif

	return slice_build_memory_elementnb(cast(CastedType*, p_slice.Begin), p_element_nb);
};

#if TOKEN_TYPE_SAFETY
#define sliceoftoken_cast(CastedType, SourceSlice) Slice<Token(CastedType)>{(SourceSlice).Size, (Token(CastedType)*)(SourceSlice).Begin}
#else
#define sliceoftoken_cast(CastedType, SourceSlice) SourceSlice
#endif





template<class ElementType>
inline int8* slice_memmove(const Slice<ElementType>& p_target, const Slice<ElementType>& p_source)
{
#if STANDARD_ALLOCATION_BOUND_TEST
	return memory_move_safe(cast(int8*, p_target.Begin), p_target.Size * sizeof(ElementType), cast(int8*, p_source.Begin), p_source.Size * sizeof(ElementType));
#else
	return memory_move((int8*)p_target.Begin, (int8*)p_source.Begin, p_source.Size * sizeof(ElementType));
#endif
};

template<class ElementType>
inline int8* slice_memcpy(const Slice<ElementType>& p_target, const Slice<ElementType>& p_source)
{
#if STANDARD_ALLOCATION_BOUND_TEST
	return memory_cpy_safe(cast(int8*, p_target.Begin), p_target.Size * sizeof(ElementType), cast(int8*, p_source.Begin), p_source.Size * sizeof(ElementType));
#else
	return memory_cpy((int8*)p_target.Begin, (int8*)p_source.Begin, p_source.Size * sizeof(ElementType));
#endif
};

template<class ElementType>
inline int8 slice_memcompare_element(const Slice<ElementType>& p_target, const Slice<ElementType>& p_compared)
{
	return memory_compare(cast(int8*, p_target.Begin), cast(int8*, p_compared.Begin), p_compared.Size);
};

template<class ElementType>
inline int8 slice_memfind(const Slice<ElementType>& p_target, const Slice<ElementType>& p_compared, uimax* out_index)
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

	for (uimax i = 1; i < p_target.Size - p_compared.Size + 1; i++)
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
	A SliceIndex is just a begin and end uimax
*/
struct SliceIndex
{
	uimax Begin;
	uimax Size;

	inline static SliceIndex build(const uimax p_begin, const uimax p_size)
	{
		return SliceIndex{ p_begin, p_size };
	};

	inline static SliceIndex build_default()
	{
		return build(0, 0);
	};

	inline void slice_two(const uimax p_break_point, SliceIndex* out_left, SliceIndex* out_right) const
	{
		uimax l_source_initial_size = this->Size;
		*out_left = SliceIndex::build(this->Begin, p_break_point - this->Begin);
		*out_right = SliceIndex::build(p_break_point, l_source_initial_size - out_left->Size);
	};
};