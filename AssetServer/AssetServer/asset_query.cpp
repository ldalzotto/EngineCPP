
#include <sqlite3.h>
#include <string>
#include <cstdio>
#include <fstream>
#include <optick.h>

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
		p_connection.handleSQLiteError(sqlite3_prepare_v3(p_connection.connection, p_query.c_str(), (int)p_query.length(), SQLITE_PREPARE_PERSISTENT, &this->statement, nullptr));
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

//TODO -> generalize the "from file" feature for insert/update queries to avoid dusplication
struct GenericAssetQuery
{
	AssetQuery exists_query;
	AssetQuery request_query;
	AssetQuery insert_query;
	AssetQuery update_query;

	const AssetPath* asset_path;
	const AssetDatabaseConnection* connection;

	inline void allocate(const AssetPath& p_assetpath, const AssetDatabaseConnection& p_connection)
	{
		this->asset_path = &p_assetpath;
		this->connection = &p_connection;

		this->exists_query.allocate(p_connection, "select count(*) from resource where resource.id = ?");
		this->request_query.allocate(p_connection, "select resource.data from resource where resource.id = ?");
		this->insert_query.allocate(p_connection, "insert into resource (id, path, data) values (?, ?, ?)");
		this->update_query.allocate(p_connection, "update resource set data = ? where id = ?");
	};

	inline void free()
	{
		this->exists_query.free(*connection);
		this->request_query.free(*connection);
		this->insert_query.free(*connection);
		this->update_query.free(*connection);
	};

	inline void insert_fromfile(const std::string& p_path)
	{
		OPTICK_EVENT();

		com::Vector<char> l_file;
		File::read_bytes(asset_path->asset_folder_path + p_path, l_file);
		{
			this->insert(p_path, l_file);
		}
		l_file.free();
	};

	inline void insert(const std::string& p_path, const com::Vector<char>& p_data)
	{
		OPTICK_EVENT();

		size_t l_id = Hash<std::string>::hash(p_path);
		this->connection->handleSQLiteError(sqlite3_bind_int64(this->insert_query.statement, 1, l_id));

		{
			sqlite3_bind_int64(this->insert_query.statement, 1, l_id);
			sqlite3_bind_text(this->insert_query.statement, 2, p_path.c_str(), (int)p_path.length(), nullptr);
			sqlite3_bind_blob(this->insert_query.statement, 3, p_data.Memory, (int)p_data.Size, nullptr);

			this->insert_query.insert(*connection);
		}

		this->insert_query.clear(*connection);
	};

	inline void update_fromfile(const std::string& p_path)
	{
		OPTICK_EVENT();

		com::Vector<char> l_file;
		File::read_bytes(asset_path->asset_folder_path + p_path, l_file);
		{
			this->update(p_path, l_file);
		}
		l_file.free();
	};

	inline void update(const std::string& p_path, const com::Vector<char>& p_data)
	{
		OPTICK_EVENT();

		size_t l_id = Hash<std::string>::hash(p_path);
		this->connection->handleSQLiteError(sqlite3_bind_int64(this->update_query.statement, 2, l_id));

		{
			connection->handleSQLiteError(sqlite3_bind_blob(this->update_query.statement, 1, p_data.Memory, (int)p_data.Size, nullptr));
			this->update_query.insert(*connection);
		}

		this->update_query.clear(*connection);
	};

	inline void insert_or_update_fromfile(const std::string& p_path)
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

		connection->handleSQLiteError(sqlite3_bind_int64(this->exists_query.statement, 1, l_id));

		CountRowFn l_count;
		this->exists_query.execute_sync_single(*connection, l_count);
		this->exists_query.clear(*connection);

		if (l_count.count == 0)
		{
			this->insert_fromfile(p_path);
		}
		else
		{
			this->update_fromfile(p_path);
		}
	};

	inline void insert_or_update(const std::string& p_path, const com::Vector<char>& p_data)
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

		connection->handleSQLiteError(sqlite3_bind_int64(this->exists_query.statement, 1, l_id));

		CountRowFn l_count;
		this->exists_query.execute_sync_single(*connection, l_count);
		this->exists_query.clear(*connection);

		if (l_count.count == 0)
		{
			this->insert(p_path, p_data);
		}
		else
		{
			this->update(p_path, p_data);
		}
	}

	inline void request(const size_t p_id, com::Vector<char>& out_bytes)
	{
		OPTICK_EVENT();

		connection->handleSQLiteError(sqlite3_bind_int64(this->request_query.statement, 1, p_id));

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
		this->request_query.execute_sync_single(*connection, l_rowfn);
		this->request_query.clear(*connection);
	}

	inline void request(const std::string& p_path, com::Vector<char>& out_bytes)
	{
		size_t l_id = Hash<std::string>::hash(p_path);
		this->request(l_id, out_bytes);
	};
};
