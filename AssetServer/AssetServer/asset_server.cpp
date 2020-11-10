#include "AssetServer\asset_server.hpp"

struct AssetServer
{
	inline static const char* DatabasePath = "database/assets.db";

	AssetDatabaseConnection connection;
	AssetPath asset_path;

	inline void allocate(const std::string& p_executable_path)
	{
		this->asset_path.initialize(p_executable_path);
		this->connection.allocate((this->asset_path.asset_folder_path + AssetServer::DatabasePath).c_str());
	}

	inline void free()
	{
		this->connection.free();
	}
};

void AssetServerHandle::allocate(const std::string& p_executable_path)
{
	AssetServer* l_asset_server = new AssetServer();
	l_asset_server->allocate(p_executable_path);
	this->handle = (void*)l_asset_server;
}

void AssetServerHandle::free()
{
	((AssetServer*)this->handle)->free();
}

const AssetDatabaseConnection& AssetServerHandle::get_connection()  const
{
	return 	((AssetServer*)this->handle)->connection;
};

const AssetPath& AssetServerHandle::get_assetpath() const
{
	return 	((AssetServer*)this->handle)->asset_path;
};
