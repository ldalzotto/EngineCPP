#include "AssetServer\asset_server.hpp"
#include "asset_query.cpp"

struct AssetServer
{
	inline static const char* DatabasePath = "database/assets.db";

	AssetDatabaseConnection connection;
	AssetPath asset_path;

	GenericAssetQuery resource_query;

	inline void allocate(const std::string& p_executable_path)
	{
		this->asset_path.initialize(p_executable_path);
		if (this->connection.allocate((this->asset_path.asset_folder_path + AssetServer::DatabasePath).c_str()) == AssetDatabaseConnection::Allocate_Step::FILE_CREATED)
		{
			AssetQuery l_table_initialization_query;
			l_table_initialization_query.allocate(this->connection, "CREATE TABLE resource(id integer not null primary key, path text not null, data blob)");
			l_table_initialization_query.insert(this->connection);
			l_table_initialization_query.free(this->connection);
		}
		this->resource_query.allocate(this->asset_path, this->connection);
	}

	inline void free()
	{
		this->resource_query.free();
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

std::string AssetServerHandle::get_asset_basepath() const
{
	return ((AssetServer*)this->handle)->asset_path.asset_folder_path;
};

com::Vector<char> AssetServerHandle::get_resource(const std::string& p_id) const
{
	com::Vector<char> l_resource;
	((AssetServer*)this->handle)->resource_query.request(p_id, l_resource);
	return l_resource;
};

com::Vector<char> AssetServerHandle::get_resource(const size_t p_id) const
{
	com::Vector<char> l_resource;
	((AssetServer*)this->handle)->resource_query.request(p_id, l_resource);
	return l_resource;
};

String<> AssetServerHandle::get_path_from_resourcehash(const size_t p_id) const
{
	return ((AssetServer*)this->handle)->resource_query.get_path_from_resourcehash(p_id);
};

void AssetServerHandle::insert_or_update_resource_fromfile(const std::string& p_path)
{
	((AssetServer*)this->handle)->resource_query.insert_or_update_fromfile(p_path);
};

void AssetServerHandle::insert_or_update_resource(const std::string& p_path, const com::Vector<char>& p_data)
{
	((AssetServer*)this->handle)->resource_query.insert_or_update(p_path, p_data);
};