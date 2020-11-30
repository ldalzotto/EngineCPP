#pragma once

#include <string>
#include "Common/Container/vector_def.hpp"
#include "Common/Container/string_def.hpp"

struct AssetServerHandle
{
	void* handle = nullptr;
	void allocate(const std::string& p_executable_path);
	void free();

	std::string get_asset_basepath() const;
	com::Vector<char> get_resource(const std::string& p_id) const;
	com::Vector<char> get_resource(const size_t p_id) const;
	String<> get_path_from_resourcehash(const size_t p_id) const;
	void insert_or_update_resource_fromfile(const std::string& p_path);
	void insert_or_update_resource(const std::string& p_path, const com::Vector<char>& p_data);
};