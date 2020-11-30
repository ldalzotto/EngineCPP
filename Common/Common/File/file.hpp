#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include "Common/Include/platform_include.hpp"
#include "Common/Container/tree.hpp"
#include "Common/Container/vector.hpp"
#include "Common/Container/string.hpp"


enum FilePathMemoryLayout
{
	STRING = 0,
	SLICE = 1
};

template<unsigned FilePathMemoryLayoutType>
struct FilePath { };

template<>
struct FilePath<FilePathMemoryLayout::STRING>
{
	inline void allocate(size_t p_initial_size)
	{
		this->path.allocate(p_initial_size);
	};

	inline FilePath<FilePathMemoryLayout::STRING> clone() const
	{
		FilePath<FilePathMemoryLayout::STRING> l_target;
		l_target.path = this->path.clone();
		return l_target;
	};

	inline void free()
	{
		this->path.free();
	};

	inline FilePath<FilePathMemoryLayout::STRING> clone()
	{
		FilePath<FilePathMemoryLayout::STRING> l_target;
		l_target.path = this->path.clone();
		return l_target;
	};

	inline StringSlice get_extension()
	{
		size_t l_index = 0;
		size_t l_dot_index;
		while (this->path.find(".", l_index, &l_dot_index)) {
			l_index = l_dot_index + 1;
		};
		return StringSlice(this->path.c_str(), l_index, this->path.Memory.Size);
	};

	inline StringSlice get_filename()
	{
		size_t l_index = 0;
		size_t l_dot_index;
		while (this->path.find("/", l_index, &l_dot_index)) {
			if (l_dot_index != this->path.Memory.Size - 2)
			{
				l_index = l_dot_index + 1;
			}
			else
			{
				break;
			}
		};
		return StringSlice(this->path.c_str(), l_index, this->path.Memory.Size);
	};

	String<> path;

	inline String<> get_as_string()
	{
		return this->path.clone();
	};
};

template<>
struct FilePath<FilePathMemoryLayout::SLICE>
{

	inline FilePath() {};

	inline FilePath(const StringSlice& p_path)
	{
		this->path = p_path;
	};

	inline void free() { };

	inline StringSlice get_extension()
	{
		StringSlice l_slice = this->path;
		size_t l_index;
		while (l_slice.find(".", &l_index)) {
			l_slice.Begin = l_index + 1;
		};
		return l_slice;
	};

	StringSlice path;

	inline String<> get_as_string()
	{
		String<> l_path; l_path.allocate(this->path.size());
		l_path.append(this->path);
		return l_path;
	};

};

enum FileType
{
	UNDEFINED = 0,
	FOLDER = 1,
	CONTENT = 2
};

template<unsigned FilePathMemoryLayoutType = FilePathMemoryLayout::SLICE>
struct File
{
	FilePath<FilePathMemoryLayoutType> path = FilePath<FilePathMemoryLayoutType>();
	HANDLE file = NULL;
	FileType type = FileType::UNDEFINED;
	StringSlice extension;
	size_t last_modfied_time;
	size_t creation_time;
	size_t last_access_time;

	inline void allocate(const FileType p_type, const FilePath<FilePathMemoryLayoutType>& p_path, const size_t p_last_modified_time = 0, const size_t p_creation_time = 0, const size_t p_last_access_time = 0)
	{
		this->path = p_path;
		this->type = p_type;
		this->extension = this->path.get_extension();
		this->last_modfied_time = p_last_modified_time;
		this->creation_time = p_creation_time;
		this->last_access_time = p_last_access_time;
	};

	inline void free()
	{
		this->path.free();
		if (this->file != NULL)
		{
			CloseHandle(this->file);
		}
	};

	inline File<FilePathMemoryLayoutType> clone() const
	{
		File<FilePathMemoryLayoutType> l_target;
		l_target.path = this->path.clone();
		l_target.type = this->type;
		l_target.extension = this->extension;
		l_target.extension.Memory = l_target.path.path.c_str();
		l_target.last_modfied_time = this->last_modfied_time;
		l_target.last_access_time = this->last_access_time;
		l_target.creation_time = this->creation_time;
		return l_target;
	};

	struct ForEachFileDefault
	{
		inline void foreach(const File<FilePathMemoryLayout::STRING>& p_file, size_t p_depth) {};
	};

	inline bool exists()
	{
		String<> l_path = this->path.get_as_string();
		bool l_found = false;
		{
			WIN32_FIND_DATAA l_find_data;
			HANDLE l_file = FindFirstFile(l_path.c_str(), &l_find_data);
			if (l_file != INVALID_HANDLE_VALUE)
			{
				l_found = true;
				FindClose(l_file);
			}
		}
		l_path.free();
		return l_found;
	};

	inline void create()
	{
		String<> l_path = this->path.get_as_string();
		this->file = CreateFile(l_path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		l_path.free();
	};

	inline void open()
	{
		String<> l_path = this->path.get_as_string();
		this->file = CreateFile(l_path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		l_path.free();
	};

	inline void create_or_open()
	{
		this->create();
		if (this->file == INVALID_HANDLE_VALUE)
		{
			this->open();
		}
	};

	inline void append(StringSlice& p_str)
	{
		SetFilePointer(this->file, 0, NULL, FILE_END);
		WriteFile(this->file, (p_str.Memory + p_str.Begin), (DWORD)p_str.size(), NULL, NULL);
	};

	inline void read(size_t p_begin, size_t p_nb, String<>& out_buffer)
	{
		SetFilePointer(this->file, (LONG)p_begin, NULL, FILE_BEGIN);

		size_t l_read_buffer_count = p_nb;
		if (l_read_buffer_count > (out_buffer.Memory.Capacity - 1))
		{
			l_read_buffer_count = out_buffer.Memory.Capacity - 1;
		};
		unsigned long l_char_read;
		ReadFile(this->file, out_buffer.Memory.Memory, (DWORD)l_read_buffer_count, &l_char_read, NULL);
		out_buffer.Memory.Size = l_char_read;
	};

	template<class ForEachFile = ForEachFileDefault>
	inline void walk(bool p_recursive, ForEachFile& p_foreach_file = ForEachFileDefault(), size_t p_depth = 1)
	{
		WIN32_FIND_DATA l_find_data;
		HANDLE l_find = INVALID_HANDLE_VALUE;

		String<> l_current_file_first;
		l_current_file_first.allocate(0);
		{
			l_current_file_first.append(this->path.path).append("*");
			l_find = FindFirstFile(l_current_file_first.Memory.Memory, &l_find_data);
		}
		l_current_file_first.free();

		while (l_find != INVALID_HANDLE_VALUE)
		{
			if (p_recursive && (l_find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				if (p_recursive)
				{
					if (!StringSlice(".").equals(StringSlice(l_find_data.cFileName)) &&
						!StringSlice("..").equals(StringSlice(l_find_data.cFileName)))
					{
						//l_find_data.
						FilePath<FilePathMemoryLayout::STRING> l_new_path;
						l_new_path.allocate(0);
						l_new_path.path.append(this->path.path);
						l_new_path.path.append(l_find_data.cFileName);
						l_new_path.path.append("/");

						File<FilePathMemoryLayout::STRING> l_new_file;
						l_new_file.allocate(FileType::FOLDER, l_new_path,
							FILETIME_to_mics(l_find_data.ftLastWriteTime),
							FILETIME_to_mics(l_find_data.ftCreationTime),
							FILETIME_to_mics(l_find_data.ftLastAccessTime));
						{
							p_foreach_file.foreach(l_new_file, p_depth);
							l_new_file.walk<ForEachFile>(p_recursive, p_foreach_file, p_depth + 1);
						}
						l_new_file.free();
					}
				}

			}
			else
			{
				FilePath<FilePathMemoryLayout::STRING> l_file_path;
				l_file_path.allocate(0);
				l_file_path.path.append(this->path.path);
				l_file_path.path.append(l_find_data.cFileName);

				File<FilePathMemoryLayout::STRING> l_new_file;
				ULARGE_INTEGER l_last_write_time = *(ULARGE_INTEGER*)&l_find_data.ftLastWriteTime;
				l_new_file.allocate(FileType::CONTENT, l_file_path,
					FILETIME_to_mics(l_find_data.ftLastWriteTime),
					FILETIME_to_mics(l_find_data.ftCreationTime),
					FILETIME_to_mics(l_find_data.ftLastAccessTime));
				{
					p_foreach_file.foreach(l_new_file, p_depth);
				}
				l_new_file.free();
			}

			if (!FindNextFile(l_find, &l_find_data))
			{
				goto end;
			};
		}

	end:
		FindClose(l_find);
	};

	struct FindFileFilterFnDefault
	{
		inline static bool filter(const File<FilePathMemoryLayout::STRING>& p_file)
		{
			return true;
		};
	};

	template<class FindFileFilterFn = FindFileFilterFnDefault>
	inline void find_files(com::Vector<File<FilePathMemoryLayout::STRING>>& out_paths_buffer, bool p_recursive)
	{

		struct FindFilesForeach
		{
			com::Vector<File<FilePathMemoryLayout::STRING>>* out_paths_buffer;

			inline FindFilesForeach() {};
			inline FindFilesForeach(com::Vector<File<FilePathMemoryLayout::STRING>>& p_out_paths_buffer)
			{
				out_paths_buffer = &p_out_paths_buffer;
			};

			inline void foreach(const File<FilePathMemoryLayout::STRING>& p_file, const size_t p_depth)
			{
				if (p_file.type == FileType::CONTENT)
				{
					if (FindFileFilterFn::filter(p_file))
					{
						//We clone the path because the incoming p_file is disposed by the walk algorithm
						File<FilePathMemoryLayout::STRING> l_file = p_file.clone();
						this->out_paths_buffer->push_back(l_file);
					}
				}
			};
		};

		this->walk<FindFilesForeach>(p_recursive, FindFilesForeach(out_paths_buffer));


	};

	inline static void free_file_vector(com::Vector<File<FilePathMemoryLayoutType>>& p_files)
	{
		for (size_t i = 0; i < p_files.Size; i++)
		{
			p_files[i].free();
		}
		p_files.free();
	};
};

using FileStr = File<FilePathMemoryLayout::STRING>;

struct FileTree : public NTree<File<FilePathMemoryLayout::STRING>>
{
	inline void allocate(const File<>& p_root_file)
	{
		FilePath<FilePathMemoryLayout::STRING> l_root_path;
		l_root_path.allocate(0);
		l_root_path.path.append(p_root_file.path.path);
		File<FilePathMemoryLayout::STRING> l_root;
		l_root.allocate(FileType::FOLDER, l_root_path);
		this->push_root_value(l_root);


		com::PoolToken l_root_token;
		//this->resolve(com::PoolToken<NTreeNode>(0)).node.

		struct FileForeach
		{
			FileTree* tree;
			com::Vector<com::TPoolToken<NTreeNode>> node_levels;
			com::TPoolToken<FileStr> last_pushed_token;
			FileForeach() {};

			inline void allocate(FileTree& p_tree)
			{
				this->tree = &p_tree;
				this->node_levels.push_back(0);
			};

			inline void free()
			{
				this->node_levels.free();
			};

			inline void foreach(File<FilePathMemoryLayout::STRING>& p_file, size_t p_depth)
			{
				//One level depper
				if (this->node_levels.Size + 1 == p_depth)
				{
					this->node_levels.push_back(this->last_pushed_token.cast<NTreeNode>());
					com::TPoolToken<FileStr> l_token = this->tree->push_value(this->node_levels[p_depth - 1], p_file.clone());
					this->last_pushed_token = l_token;
				}
				else if (this->node_levels.Size == p_depth) // same level
				{
					this->last_pushed_token = this->tree->push_value(this->node_levels[p_depth - 1], p_file.clone());
					// this->node_levels.push_back(com::PoolToken<NTreeNode>(l_token.Index));
				}
				else
				{
					while (this->node_levels.Size != p_depth)
					{
						this->node_levels.erase_at(this->node_levels.Size - 1, 1);
					}

					auto l_token = this->tree->push_value(this->node_levels[p_depth - 1], p_file.clone());
					this->last_pushed_token = l_token.Index;
					// this->node_levels.push_back(com::PoolToken<NTreeNode>(l_token.Index));

				}
			};
		};
		FileForeach l_file_foreach;
		l_file_foreach.allocate(*this);
		{
			l_root.walk(true, l_file_foreach);
		}
		l_file_foreach.free();

	};

	inline void free()
	{
		if (this->Indices.size() > 0)
		{
			struct FreeForeach
			{
				inline void foreach(NTreeResolve<File<FilePathMemoryLayout::STRING>>& p_node)
				{
					p_node.element->free();
				};
			};

			this->traverse(com::PoolToken(0), FreeForeach());
		}

		NTree<File<FilePathMemoryLayout::STRING>>::free();
	};

	inline FileTree move()
	{
		return *(FileTree*)&NTree<File<FilePathMemoryLayout::STRING>>::move();
	}
};

struct FileAlgorithm
{
	inline static void read_bytes(const std::string& p_path, com::Vector<char>& out_file)
	{
		{
			std::ifstream l_stream(p_path, std::ios::binary | std::ios::in | std::ios::ate);
			if (l_stream.is_open())
			{
				size_t l_size = l_stream.tellg();
				l_stream.seekg(0, std::ios::beg);
				out_file.resize(l_size);
				out_file.Size = out_file.Capacity;
				l_stream.read((char*)out_file.Memory, l_size);
				l_stream.close();
			}
		}
	};

	inline static size_t get_char_nb(const std::string& p_path)
	{
		size_t l_size = 0;
		std::ifstream l_stream(p_path, std::ios::binary | std::ios::in | std::ios::ate);
		if (l_stream.is_open())
		{
			l_size = l_stream.tellg();
			l_stream.close();
		}
		return l_size;
	};

};