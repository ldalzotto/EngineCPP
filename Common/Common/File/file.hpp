#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include "Common/Include/platform_include.hpp"
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

	inline StringSlice calculate_extension()
	{
		size_t l_index = 0;
		size_t l_dot_index;
		while (this->path.find(".", l_index, &l_dot_index)) {
			l_index = l_dot_index + 1;
		};
		return StringSlice(this->path.c_str(), l_index, this->path.Memory.Size);
	};

	String<> path;
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

	inline StringSlice calculate_extension()
	{
		StringSlice l_slice = this->path;
		size_t l_index;
		while (l_slice.find(".", &l_index)) {
			l_slice.Begin = l_index + 1;
		};
		return l_slice;
	};

	StringSlice path;
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

	FileType type = FileType::UNDEFINED;
	StringSlice extension;

	inline void allocate(const FileType p_type, const FilePath<FilePathMemoryLayoutType>& p_path)
	{
		this->path = p_path;
		this->type = p_type;
		this->extension = this->path.calculate_extension();
	};

	inline void free()
	{
		this->path.free();
	};

	struct ForEachFileDefault
	{
		inline void foreach(const File<FilePathMemoryLayout::STRING>& p_file) {};
	};

	template<class ForEachFile = ForEachFileDefault>
	inline void walk(bool p_recursive, ForEachFile& p_foreach_file = ForEachFileDefault())
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
						FilePath<FilePathMemoryLayout::STRING> l_new_path;
						l_new_path.allocate(0);
						l_new_path.path.append(this->path.path);
						l_new_path.path.append(l_find_data.cFileName);
						l_new_path.path.append("/");

						File<FilePathMemoryLayout::STRING> l_new_file;
						l_new_file.allocate(FileType::FOLDER, l_new_path);
						{
							p_foreach_file.foreach(l_new_file);
							l_new_file.walk<ForEachFile>(p_recursive, p_foreach_file);
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
				l_new_file.allocate(FileType::CONTENT, l_file_path);
				{
					p_foreach_file.foreach(l_new_file);
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

	template<class FindFileFilterFn>
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

			inline void foreach(const File<FilePathMemoryLayout::STRING>& p_file)
			{
				if (p_file.type == FileType::CONTENT)
				{
					if (FindFileFilterFn::filter(p_file))
					{
						//We clone the path because the incoming p_file is disposed by the walk algorithm
						File<FilePathMemoryLayout::STRING> l_file;
						l_file.type = p_file.type;
						l_file.path = p_file.path.clone();
						this->out_paths_buffer->push_back(l_file);
					}
				}
			};
		};

		this->walk<FindFilesForeach>(p_recursive, FindFilesForeach(out_paths_buffer));


	};
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

};