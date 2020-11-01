#pragma once

template <class TYPE>
struct Optional
{
	bool hasValue;
	TYPE value;

	Optional(): hasValue(false) {}

	inline Optional(const TYPE& p_value)
	{
		this->hasValue = true;
		this->value = p_value;
	}

	inline void clear()
	{
		this->hasValue = false;
	}
};