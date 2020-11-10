
#include <sqlite3.h>
#include <string>
#include <cstdio>
#include <fstream>
#include "AssetServer\asset_server.hpp"
#include "Common/Functional/Hash.hpp"
#include "Common/Container/vector.hpp"
#include "Common/File/file.hpp"

struct AssetPath
{
	std::string asset_folder_path;

	inline void initialize(const std::string& p_executable_path)
	{
#if ASSETPATH_DEV
		this->asset_folder_path = "E:/GameProjects/CPPTestVS/Assets/";
#else
		this->asset_folder_path = p_executable_path.substr(0, p_executable_path.find_last_of("\\")).append("\\Assets\\");
#endif
	};
};

struct AssetDatabaseConnection
{
	sqlite3* connection = nullptr;

	inline int handleSQLiteError(int p_return) const
	{
		if (p_return != 0)
		{
			std::string l_error_message = "SQLITE ERROR : ";
			sqlite3_errstr(p_return);
			if (this->connection)
			{
				l_error_message += " : ";
				l_error_message += sqlite3_errmsg(this->connection);
			}
			printf(l_error_message.c_str());
			abort();
		}
		return p_return;
	};

	inline int handleStepError(int p_step_return) const
	{
		if (p_step_return != SQLITE_BUSY && p_step_return != SQLITE_DONE && p_step_return != SQLITE_ROW)
		{
			std::string l_error_message = "SQLITE ERROR : ";
			sqlite3_errstr(p_step_return);
			if (this->connection)
			{
				l_error_message += " : ";
				l_error_message += sqlite3_errmsg(this->connection);
			}
			printf(l_error_message.c_str());
			abort();
		}

		return p_step_return;
	}

	inline void allocate(const char* p_databasepath)
	{
		this->handleSQLiteError(sqlite3_open(p_databasepath, &this->connection));
	}

	inline void free()
	{
		this->handleSQLiteError(sqlite3_close(this->connection));
	}
};

struct AssetQuery
{
	sqlite3_stmt* statement = nullptr;

	inline void allocate(const AssetDatabaseConnection& p_connection, const std::string& p_query)
	{
		p_connection.handleSQLiteError(sqlite3_prepare_v3(p_connection.connection, p_query.c_str(), p_query.length(), SQLITE_PREPARE_PERSISTENT, &this->statement, nullptr));
	};

	template<class RowFn>
	inline void execute_sync(const AssetDatabaseConnection& p_connection, RowFn& p_rowfn)
	{
		int l_step_status = SQLITE_BUSY;
		while (l_step_status == SQLITE_BUSY)
		{
			l_step_status = p_connection.handleStepError(sqlite3_step(this->statement));
		}

		while (l_step_status == SQLITE_ROW)
		{
			p_rowfn.execute(this->statement);

			l_step_status = SQLITE_BUSY;
			while (l_step_status == SQLITE_BUSY)
			{
				l_step_status = p_connection.handleStepError(sqlite3_step(this->statement));
			}
		}
	};

	template<class RowFn>
	inline void execute_sync_single(const AssetDatabaseConnection& p_connection, RowFn& p_rowfn)
	{
		int l_step_status = SQLITE_BUSY;
		while (l_step_status == SQLITE_BUSY)
		{
			l_step_status = p_connection.handleStepError(sqlite3_step(this->statement));
		}

		if (l_step_status == SQLITE_ROW)
		{
			p_rowfn.execute(this->statement);
		}
	};

	inline void insert(const AssetDatabaseConnection& p_connection)
	{
		int l_step_status = SQLITE_BUSY;
		while (l_step_status == SQLITE_BUSY)
		{
			l_step_status = p_connection.handleStepError(sqlite3_step(this->statement));
		}

		while (l_step_status == SQLITE_ROW)
		{
			l_step_status = p_connection.handleStepError(sqlite3_step(this->statement));
		}
	}

	inline void clear(const AssetDatabaseConnection& p_connection)
	{
		p_connection.handleSQLiteError(sqlite3_clear_bindings(this->statement));
		p_connection.handleSQLiteError(sqlite3_reset(this->statement));
	};

	inline void free(const AssetDatabaseConnection& p_connection)
	{
		p_connection.handleSQLiteError(sqlite3_finalize(this->statement));
		this->statement = nullptr;
	};
};

struct GenericAssetQuery
{
	AssetQuery exists_query;
	AssetQuery request_query;
	AssetQuery insert_query;
	AssetQuery update_query;

	inline void allocate(const AssetDatabaseConnection& p_connection, const std::string& p_resource_name)
	{
		this->exists_query.allocate(p_connection, "select count(*) from " + p_resource_name + " where " + p_resource_name + ".id = ?");
		this->request_query.allocate(p_connection, "select " + p_resource_name + ".data from " + p_resource_name + " where " + p_resource_name + ".id = ?");
		this->insert_query.allocate(p_connection, "insert into " + p_resource_name + " (id, path, data) values (?, ?, ?)");
		this->update_query.allocate(p_connection, "update " + p_resource_name + " set data = ? where id = ?");
	};

	inline void free(const AssetDatabaseConnection& p_connection)
	{
		this->exists_query.free(p_connection);
		this->request_query.free(p_connection);
		this->insert_query.free(p_connection);
		this->update_query.free(p_connection);
	};

	inline void insert(const AssetPath& p_assetpath, const AssetDatabaseConnection& p_connection, const std::string& p_path)
	{
		size_t l_id = Hash<std::string>::hash(p_path);
		p_connection.handleSQLiteError(sqlite3_bind_int64(this->insert_query.statement, 1, l_id));

		com::Vector<char> l_file;
		File::read_bytes(p_assetpath.asset_folder_path + p_path, l_file);
		{
			sqlite3_bind_int64(this->insert_query.statement, 1, l_id);
			sqlite3_bind_text(this->insert_query.statement, 2, p_path.c_str(), p_path.length(), nullptr);
			sqlite3_bind_blob(this->insert_query.statement, 3, l_file.Memory, l_file.Size, nullptr);

			this->insert_query.insert(p_connection);
		}
		l_file.free();

		this->insert_query.clear(p_connection);
	};

	inline void update(const AssetPath& p_assetpath, const AssetDatabaseConnection& p_connection, const std::string& p_path)
	{
		size_t l_id = Hash<std::string>::hash(p_path);
		p_connection.handleSQLiteError(sqlite3_bind_int64(this->update_query.statement, 2, l_id));

		com::Vector<char> l_file;
		File::read_bytes(p_assetpath.asset_folder_path + p_path, l_file);
		{
			p_connection.handleSQLiteError(sqlite3_bind_blob(this->update_query.statement, 1, l_file.Memory, l_file.Size, nullptr));
			this->update_query.insert(p_connection);
		}
		l_file.free();

		this->update_query.clear(p_connection);
	};

	inline void insert_or_update(const AssetPath& p_assetpath, const AssetDatabaseConnection& p_connection, const std::string& p_path)
	{
		size_t l_id = Hash<std::string>::hash(p_path);

		struct CountRowFn
		{
			size_t count = 0;

			inline void execute(sqlite3_stmt* p_statement)
			{
				this->count = sqlite3_column_int64(p_statement, 0);
			};
		};

		p_connection.handleSQLiteError(sqlite3_bind_int64(this->exists_query.statement, 1, l_id));

		CountRowFn l_count;
		this->exists_query.execute_sync_single(p_connection, l_count);
		this->exists_query.clear(p_connection);

		if (l_count.count == 0)
		{
			this->insert(p_assetpath, p_connection, p_path);
		}
		else
		{
			this->update(p_assetpath, p_connection, p_path);
		}
	};

	inline void request(const AssetDatabaseConnection& p_connection, const std::string& p_path, com::Vector<char>& out_bytes)
	{
		size_t l_id = Hash<std::string>::hash(p_path);
		p_connection.handleSQLiteError(sqlite3_bind_int64(this->request_query.statement, 1, l_id));

		struct ShaderRowFn
		{
			com::Vector<char>* output;

			inline ShaderRowFn(com::Vector<char>& p_output)
			{
				this->output = &p_output;
			};

			inline void execute(sqlite3_stmt* p_statement)
			{
				size_t l_size = sqlite3_column_bytes(p_statement, 0);
				this->output->resize(l_size);
				this->output->Size = l_size;
				memcpy(this->output->Memory, sqlite3_column_blob(p_statement, 0), l_size);
			};
		};

		ShaderRowFn l_rowfn = ShaderRowFn(out_bytes);
		this->request_query.execute_sync_single(p_connection, l_rowfn);
		this->request_query.clear(p_connection);
	};
};

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

/*
com::Vector<char> AssetServerHandle::request_shader(const std::string& p_path)
{

};
*/