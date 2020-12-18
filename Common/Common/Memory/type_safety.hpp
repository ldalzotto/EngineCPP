
#pragma once

struct SizetType
{
	size_t val;

	inline SizetType() : val{ (size_t)-1 } {};
	inline SizetType(const size_t& p_index) : val{ p_index } {};

	inline void reset() { this->val = -1; };
};