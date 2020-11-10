#pragma once

#include "asset_query.hpp"

struct AssetServerHandle
{
	void* handle;
	void allocate(const std::string& p_executable_path);
	void free();
	const AssetDatabaseConnection& get_connection() const;
	const AssetPath& get_assetpath()  const;
};