#pragma once

#include "Common/Container/vector_def.hpp"


struct AssetServerHandle
{
	void* handle;
	void allocate(const std::string& p_executable_path);
	void free();
	com::Vector<char> request_shader(const std::string& p_path);
};