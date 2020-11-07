#pragma once

#include "Common/Memory/handle.hpp"

struct SceneHandle
{
	void* handle;
	void allocate();
	void free();
};