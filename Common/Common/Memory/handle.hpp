#pragma once

struct Handle
{
	size_t handle = -1;
	Handle(){}
	inline Handle(const size_t p_handle) { this->handle = p_handle; }
	inline void reset() { this->handle = -1; }
	inline bool isvalid() { return this->handle != -1; }
};