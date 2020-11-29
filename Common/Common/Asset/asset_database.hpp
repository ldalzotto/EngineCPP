#pragma once

#include <sqlite3.h>
#include <string>
#include <cstdio>
#include <fstream>

#include "Common/File/file.hpp"

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

	enum class Allocate_Step
	{
		UNKNOWN = 0,
		FILE_CREATED = 1
	};

	inline Allocate_Step allocate(const char* p_databasepath)
	{
		File<FilePathMemoryLayout::SLICE> l_database_file;
		bool l_database_file_created = false;
		FilePath<FilePathMemoryLayout::SLICE> l_database_file_path = FilePath<FilePathMemoryLayout::SLICE>(StringSlice(p_databasepath));
		l_database_file.allocate(FileType::CONTENT, l_database_file_path);
		if(!l_database_file.exists())
		{
			l_database_file.create();
			l_database_file_created = true;
		}
		l_database_file.free();

		this->handleSQLiteError(sqlite3_open(p_databasepath, &this->connection));

		if (l_database_file_created)
		{
			return Allocate_Step::FILE_CREATED;
		}
		
		return Allocate_Step::UNKNOWN;
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
