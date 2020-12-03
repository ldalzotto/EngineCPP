#include "Common/Include/platform_include.hpp"
#include "AssetServer/asset_server.hpp"

#include <string>

#include <Common/Container/vector.hpp>
#include <Common/File/file.hpp>
#include <Common/Container/tree.hpp>
#include <Common/Clock/clock.hpp>
#include <SceneSerialization/scene_serialization.hpp>
#include <Middleware/scene_middleware.hpp>

#include<iostream>
#include<fstream>
#include<sstream>

#include <Render/assets.hpp>

#include "Common/Asset/asset_database.hpp"

#include "../Render/Render/Render.cpp"
#include "obj_reader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct AssetMeta
{
	size_t path_hash;
	size_t last_compiled_ts;
};

struct AssetToolDatabase
{
	AssetDatabaseConnection connection;

	inline void allocate(const std::string& p_asset_folder_path)
	{
		FilePath<FilePathMemoryLayout::STRING> l_asset_folder_path;
		l_asset_folder_path.allocate(p_asset_folder_path.size());
		l_asset_folder_path.path.append(p_asset_folder_path.c_str());
		l_asset_folder_path.path.append("/.tmp/asset_compile.db");

		File<FilePathMemoryLayout::STRING> l_asset_folder_tmp_file;
		l_asset_folder_tmp_file.allocate(FileType::FOLDER, l_asset_folder_path);
		l_asset_folder_tmp_file.create();

		this->connection.allocate(l_asset_folder_path.path.c_str());

		AssetQuery l_create_table_query;
		l_create_table_query.allocate(this->connection, "create table if not exists asset_meta(id INTEGER PRIMARY KEY, last_compilation_ts INTEGER);");
		l_create_table_query.insert(this->connection);
		l_create_table_query.free(this->connection);

		l_asset_folder_tmp_file.free();
	};

	inline bool get_assetmeta(size_t p_asset_path, AssetMeta* out_asset_meta)
	{
		AssetQuery l_get_executiontime_query;
		l_get_executiontime_query.allocate(this->connection, "select * from asset_meta where asset_meta.id = ?");
		sqlite3_bind_int64(l_get_executiontime_query.statement, 1, p_asset_path);
		struct GetExecRow
		{
			AssetMeta asset_meta;
			bool asset_meta_found = false;

			inline void execute(sqlite3_stmt* p_stmt)
			{
				this->asset_meta.path_hash = sqlite3_column_int64(p_stmt, 0);
				this->asset_meta.last_compiled_ts = sqlite3_column_int64(p_stmt, 1);
				this->asset_meta_found = true;
			};
		};
		GetExecRow l_row_exec;
		l_get_executiontime_query.execute_sync_single(this->connection, l_row_exec);
		l_get_executiontime_query.free(this->connection);
		*out_asset_meta = l_row_exec.asset_meta;
		return l_row_exec.asset_meta_found;
	};

	inline void set_assetmeta(AssetMeta& p_assetmeta)
	{
		AssetQuery l_count_query;
		l_count_query.allocate(this->connection, "select count(*) from asset_meta where asset_meta.id = ?");
		sqlite3_bind_int64(l_count_query.statement, 1, p_assetmeta.path_hash);

		struct RowCount { size_t l_count = 0; inline void execute(sqlite3_stmt* p_stmt) { this->l_count = sqlite3_column_int64(p_stmt, 0); } };
		RowCount l_row_count;
		l_count_query.execute_sync_single(this->connection, l_row_count);
		l_count_query.free(this->connection);

		AssetQuery l_get_executiontime_query;
		if (l_row_count.l_count == 0)
		{
			l_get_executiontime_query.allocate(this->connection, "insert into asset_meta (id, last_compilation_ts) values (?, ?)");
			sqlite3_bind_int64(l_get_executiontime_query.statement, 1, p_assetmeta.path_hash);
			sqlite3_bind_int64(l_get_executiontime_query.statement, 2, p_assetmeta.last_compiled_ts);
		}
		else
		{
			l_get_executiontime_query.allocate(this->connection, "update asset_meta set last_compilation_ts = ? where id = ?");
			sqlite3_bind_int64(l_get_executiontime_query.statement, 1, p_assetmeta.last_compiled_ts);
			sqlite3_bind_int64(l_get_executiontime_query.statement, 2, p_assetmeta.path_hash);
		}

		l_get_executiontime_query.insert(this->connection);
		l_get_executiontime_query.clear(this->connection);
		l_get_executiontime_query.free(this->connection);
	};

	inline void free()
	{
		this->connection.free();
	};
};

struct AssetDatabaseUpdate
{
	AssetServerHandle asset_server;
	AssetToolDatabase asset_tool_database;

	inline void allocate(const std::string& p_executable_path)
	{
		this->asset_server.allocate(p_executable_path);
		this->asset_tool_database.allocate(this->asset_server.get_asset_basepath());
	};

	inline void free()
	{
		this->asset_tool_database.free();
		this->asset_server.free();
	};


	inline void compile_all_assets()
	{
		std::string l_asset_base_path = this->asset_server.get_asset_basepath();
		size_t l_execution_time = clock_currenttime_mics();

		FileTree l_assetfile_tree;
		File<> l_root_file;

		l_root_file.allocate(FileType::FOLDER, FilePath<FilePathMemoryLayout::SLICE>(l_asset_base_path.c_str()));
		l_assetfile_tree.allocate(l_root_file);

		struct ForeachFile
		{
			AssetDatabaseUpdate* asset_database_update;
			size_t execution_time;

			inline ForeachFile() {};
			inline ForeachFile(AssetDatabaseUpdate* p_asset_database_update, size_t p_execution_time) {
				this->asset_database_update = p_asset_database_update;
				this->execution_time = p_execution_time;
			};

			inline void foreach(NTreeResolve<File<FilePathMemoryLayout::STRING>>& p_file)
			{
				this->asset_database_update->compile_file(*p_file.element, this->execution_time);
			};
		};

		l_assetfile_tree.traverse(com::PoolToken(0), ForeachFile(this, l_execution_time));
		// l_asset_database_update.asset_tool_database.set_last_executiontime(l_execution_time);
		l_assetfile_tree.free();
	}

private:
	inline void compile_file(FileStr& p_file, size_t p_execution_time)
	{


		std::string l_folder_path = this->asset_server.get_asset_basepath();
		StringSlice l_root_asset_folder_absolute = StringSlice(l_folder_path.c_str());
		StringSlice l_asset_folder_relative = StringSlice(p_file.path.path.c_str(), l_root_asset_folder_absolute.End, strlen(p_file.path.path.c_str()));

		if (p_file.type == FileType::CONTENT)
		{

			size_t l_asset_file_relative_hash = Hash<std::string>::hash(std::string(l_asset_folder_relative.Memory + l_asset_folder_relative.Begin));

			AssetMeta l_asset_meta;
			if (this->asset_tool_database.get_assetmeta(l_asset_file_relative_hash, &l_asset_meta))
			{
				if (l_asset_meta.last_compiled_ts > p_file.last_modfied_time &&
					l_asset_meta.last_compiled_ts > p_file.last_access_time &&
					l_asset_meta.last_compiled_ts > p_file.creation_time)
				{
					return;
				}
			}

			bool l_file_compiled = false;

			size_t tmp;
			if (p_file.extension.find(StringSlice("obj"), &tmp))
			{
				RenderHeap2::Resource::MeshResourceAllocator::MeshAsset l_mesh_resource;
				com::Vector<char> l_mesh_resource_bytes;

				ObjReader::ReadObj(std::string(p_file.path.path.c_str()), l_mesh_resource.vertices, l_mesh_resource.indices);

				l_mesh_resource.sertialize_to(l_mesh_resource_bytes);

				this->asset_server.insert_or_update_resource(std::string(l_asset_folder_relative.Memory + l_asset_folder_relative.Begin), l_mesh_resource_bytes);

				l_mesh_resource.free();
				l_mesh_resource_bytes.free();

				l_file_compiled = true;

				// this->out->updated_files.push_back(p_file);
			}
			else if (p_file.extension.find(StringSlice("vert"), &tmp) || p_file.extension.find(StringSlice("frag"), &tmp))
			{
				String<> l_shadertmp_path;
				l_shadertmp_path.allocate(0);
				l_shadertmp_path.append(l_root_asset_folder_absolute);
				l_shadertmp_path.append(".tmp/shader_tmp.spv");

				String<> l_shadercompilation_out_file_path;
				l_shadercompilation_out_file_path.allocate(0);
				l_shadercompilation_out_file_path.append(l_root_asset_folder_absolute);
				l_shadercompilation_out_file_path.append(".tmp/compile.txt");

				String<> l_shadercompile_command;
				l_shadercompile_command.allocate(0);

				l_shadercompile_command.append("glslc ");
				l_shadercompile_command.append(p_file.path.path);
				l_shadercompile_command.append(" -o ");
				l_shadercompile_command.append(l_shadertmp_path);
				l_shadercompile_command.append(" > ");
				l_shadercompile_command.append(l_shadercompilation_out_file_path);

				{
					if (system(l_shadercompile_command.c_str()) == 0)
					{
						com::Vector<char> l_shadercompiled_binary;
						FileAlgorithm::read_bytes(std::string(l_shadertmp_path.c_str()), l_shadercompiled_binary);

						this->asset_server.insert_or_update_resource(std::string(l_asset_folder_relative.Memory + l_asset_folder_relative.Begin), l_shadercompiled_binary);

						l_shadercompiled_binary.free();
					}
				}

				l_shadercompile_command.free();
				l_shadertmp_path.free();

				l_file_compiled = true;
			}
			else if (p_file.extension.find(StringSlice("png"), &tmp) || p_file.extension.find(StringSlice("jpg"), &tmp))
			{
				RenderHeap2::Resource::TextureResourceAllocator::TextureAsset l_texture_resource;

				l_texture_resource.pixels.Memory = (char*)stbi_load(p_file.path.path.c_str(), &l_texture_resource.size.x, &l_texture_resource.size.y, &l_texture_resource.channel_number, STBI_rgb_alpha);
				l_texture_resource.channel_number = 4;
				l_texture_resource.pixels.Size = l_texture_resource.size.x * l_texture_resource.size.y * l_texture_resource.channel_number;
				l_texture_resource.pixels.Capacity = l_texture_resource.pixels.Size;

				com::Vector<char> l_texture_resource_bytes;
				l_texture_resource.sertialize_to(l_texture_resource_bytes);
				this->asset_server.insert_or_update_resource(std::string(l_asset_folder_relative.Memory + l_asset_folder_relative.Begin), l_texture_resource_bytes);
				l_texture_resource_bytes.free();

				l_file_compiled = true;
			}
			else if (p_file.extension.find(StringSlice("json"), &tmp))
			{


				com::Vector<char> l_file_bytes;
				FileAlgorithm::read_bytes(std::string(p_file.path.path.c_str()), l_file_bytes);

				String<> l_file_binary;
				l_file_binary.Memory = l_file_bytes;
				l_file_binary.Memory.Size = l_file_bytes.Size;
				l_file_binary.Memory.Capacity = l_file_bytes.Capacity;
				l_file_binary.remove_chars(' ');
				l_file_binary.remove_chars('\n');
				l_file_binary.remove_chars('\r');
				l_file_binary.remove_chars('\t');

				Deserialization::JSON::JSONObjectIterator l_json_deserializer = Deserialization::JSON::StartDeserialization(l_file_binary);
				{
					l_json_deserializer.next_field("type");
					Deserialization::JSON::FieldNode& l_json_type = l_json_deserializer.stack_fields[l_json_deserializer.current_field];

					com::Vector<char> l_asset_bytes;

					if (l_json_type.value.equals(StringSlice("material")))
					{
						l_json_deserializer.free();
						MaterialAsset l_material_asset = JSONDeserializer<MaterialAsset>::deserialize(l_json_deserializer);
						l_material_asset.serialize(l_asset_bytes);

						l_material_asset.free();
					}
					else if (l_json_type.value.equals(StringSlice("scene")))
					{
						l_json_deserializer.free();
						SceneAsset l_scene_asset = SceneSerializer2::JSON_to_SceneAsset(l_json_deserializer);
						l_scene_asset.serialize(l_asset_bytes);
					}
					else if (l_json_type.value.equals(StringSlice("shader")))
					{
						l_json_deserializer.free();
						ShaderAsset l_shader_asset = JSONDeserializer<ShaderAsset>::deserialize(l_json_deserializer);
						l_shader_asset.serialize(l_asset_bytes);
					}
					else if (l_json_type.value.equals(StringSlice("shader_layout")))
					{
						l_json_deserializer.free();
						ShaderLayoutAsset l_shaderlayout_asset = JSONDeserializer<ShaderLayoutAsset>::deserialize(l_json_deserializer);
						l_shaderlayout_asset.serialize(l_asset_bytes);
					}
					if (l_asset_bytes.Memory != nullptr)
					{
						this->asset_server.insert_or_update_resource(std::string(l_asset_folder_relative.Memory + l_asset_folder_relative.Begin), l_asset_bytes);
						l_asset_bytes.free();
						l_file_compiled = true;
					}
				}
				l_json_deserializer.free();
				l_file_bytes.free();
			}

			if (l_file_compiled)
			{
				l_asset_meta.last_compiled_ts = p_execution_time;
				l_asset_meta.path_hash = l_asset_file_relative_hash;
				this->asset_tool_database.set_assetmeta(l_asset_meta);

				printf("File compiled : ");
				printf(p_file.path.path.c_str());
				printf("\n");
			}

		}
	};
};

int main(int argc, char** argv)
{
	AssetDatabaseUpdate l_asset_database_update;
	l_asset_database_update.allocate(argv[0]);

	std::string l_asset_base_path = l_asset_database_update.asset_server.get_asset_basepath();

	l_asset_database_update.compile_all_assets();

	if (argc > 1 && StringSlice(argv[1]).equals("daemon"))
	{
		while (true)
		{
			HANDLE dwChangeHandles = FindFirstChangeNotification(l_asset_base_path.c_str(), FALSE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE);
			switch (WaitForMultipleObjects(1, &dwChangeHandles, TRUE, INFINITE))
			{
			case WAIT_OBJECT_0:
			{
				l_asset_database_update.compile_all_assets();
			}
			break;
			}
		}
	}
}
