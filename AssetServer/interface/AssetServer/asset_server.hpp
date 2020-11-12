#pragma once

#include <string>
#include "Common/Container/vector_def.hpp"

struct AssetServerHandle
{
	void* handle = nullptr;
	void allocate(const std::string& p_executable_path);
	void free();

	std::string get_asset_basepath() const;
	com::Vector<char> get_resource(const std::string& p_id) const;
	com::Vector<char> get_resource(const size_t p_id) const;
	void insert_or_update_resource(const std::string& p_id);
};