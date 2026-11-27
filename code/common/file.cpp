/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2020 carabobz@gmail.com

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include "client_server.h"
#include "const.h"
#include "file.h"

#include "LockGuard.h"
#include "log.h"
#include "mutex.h"
#include "network.h"
#include <bits/types/FILE.h>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <features.h>
#include <fstream>
#include <SDL_stdinc.h>
#include <SDL_timer.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>
#include <utility>

typedef struct file_tag
{
	Uint32 timestamp;
	Lock lock;
} file_t;

std::unordered_map<std::string, file_t *> file_list;

static file_t * find_file(const std::string &filename)
{
	auto iter = file_list.find(filename);
	if (iter == file_list.end())
	{
		return nullptr;
	}

	return iter->second;
}

Lock file_list_lock;

/****************************************
 filename is "table/dir/file"
 *****************************************/
void file_lock(const std::string &filename)
{
	file_t *file_data;

	LockGuard guard(file_list_lock);
	file_data = find_file(filename);
	if (file_data == nullptr)
	{
		file_data = new file_t;
		file_data->timestamp = 0;

		file_list[filename] = file_data;
	}

	file_data->lock.lock();
}

/****************************************
 filename is "table/dir/file"
 *****************************************/
void file_unlock(const std::string &filename)
{
	file_t *file_data;

	LockGuard guard(file_list_lock);
	file_data = find_file(filename);

	if (file_data == nullptr)
	{
		return;
	}

	file_data->lock.unlock();
}

/****************************************
 filename is "table/dir/file"
 *****************************************/
void file_update(Connection *connection, const std::string &filename)
{
	if (connection == nullptr)
	{
		return;
	}

	if (client_server == SERVER)
	{
		return;
	}

	file_t *file_data;

	LockGuard guard(file_list_lock);
	file_data = find_file(filename);

	if (file_data == nullptr)
	{
		return;
	}

	// Avoid flooding the server
	Uint32 current_time = SDL_GetTicks();
	if (file_data->timestamp != 0
			&& file_data->timestamp + FILE_REQUEST_TIMEOUT_MS > current_time)
	{
		return;
	}

	network_send_req_file(*connection, filename);

	file_data->timestamp = current_time;
}

/***************************************************
 return 0 if directory was successfully created
 ****************************************************/
static int mkdir_all(const std::string &path_name)
{
	int ret = -1;
	std::string directory;

	size_t start = 0;
	while (start < path_name.size())
	{
		size_t pos = path_name.find('/', start);
		if (pos == std::string::npos)
		{
			pos = path_name.size();
		}
		if (pos > start)
		{
			directory += "/" + path_name.substr(start, pos - start);
			ret = mkdir(directory.c_str(), 0775);
		}
		start = pos + 1;
	}

	return ret;
}

/****************************
 Parameter table: Name of the table to create the new file
 Parameter suggested_name: Name of the file you want to create. If empty, an available file is created.
 If the suggested name is not available the function return false.
 Return the name of an available empty file.
 On success the file is created on disk
 ****************************/
std::pair<bool, std::string> file_new(const std::string &table,
		const std::string &suggested_name)
{
	DIR *dir = nullptr;
	struct dirent *ent = nullptr;
	char tag[10];
	int index = 0;
	int fd = -1;
	struct stat sts;

	const std::string dir_path = base_directory + "/" + table;

	std::string selected_name;

	LockGuard guard(character_dir_lock);

	if (suggested_name != NO_SUGGESTED_NAME)
	{
		const std::string file_path = dir_path + "/"
				+ std::string(suggested_name);
		if (stat(file_path.c_str(), &sts) != -1)
		{
			ERR("Suggested file " + std::string(suggested_name) + " already exists");
			return std::pair<bool, std::string>
			{ false, NO_SUGGESTED_NAME };
		}
		selected_name = suggested_name;
	}
	else
	{
		dir = opendir(dir_path.c_str());

		if (dir == nullptr)
		{
			mkdir_all(dir_path);
			dir = opendir(dir_path.c_str());
			if (dir == nullptr)
			{
				ERR("Cannot create path " + std::string(dir_path));

				return std::pair<bool, std::string>
				{ false, NO_SUGGESTED_NAME };
			}
		}

		sprintf(tag, "A%05x", index);

		while ((ent = readdir(dir)) != nullptr)
		{
			if (strcmp(ent->d_name, ".") == 0)
			{
				continue;
			}
			if (strcmp(ent->d_name, "..") == 0)
			{
				continue;
			}
			if (strcmp(ent->d_name, tag) == 0)
			{
				index++;
				sprintf(tag, "A%05x", index);
				rewinddir(dir);
				continue;
			}
		}

		closedir(dir);

		selected_name = std::string(tag);
	}

	const std::string selected_file_path = dir_path + "/" + selected_name;

	fd = creat(selected_file_path.c_str(),
	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);

	close(fd);

	return std::pair<bool, std::string>
	{ true, selected_name };
}

/****************************
 filename is "table/dir/file"
 Fill contents with data from filename file.
 return false on error
 ****************************/
bool file_get_contents(const std::string &filename, std::string &contents)
{
	struct stat sts;
	int fd;
	ssize_t size;
	char error_buf[SMALL_BUF];
	char *error_str;

	contents.clear();

	const std::string file_path = base_directory + "/" + filename;

	file_lock(filename);

	if (stat(file_path.c_str(), &sts) == -1)
	{
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
		strerror_r(errno,error_buf,SMALL_BUF);
		error_str = error_buf;
#else
		error_str = strerror_r(errno, error_buf, SMALL_BUF);
#endif
		file_unlock(filename);
		ERR_DESIGN("Error stat on file " + std::string(file_path) + ": " + std::string(error_str) + "\n");
		return false;
	}

	fd = open(file_path.c_str(), O_RDONLY);
	if (fd == -1)
	{
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
		strerror_r(errno,error_buf,SMALL_BUF);
		error_str = error_buf;
#else
		error_str = strerror_r(errno, error_buf, SMALL_BUF);
#endif
		file_unlock(filename);
		ERR_DESIGN("Error open on file " + std::string(file_path) + ": " + std::string(error_str) + "\n");
		return false;
	}

	std::string buf(sts.st_size, '\0');

	size = read(fd, buf.data(), buf.size());
	if (size == -1)
	{
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
		strerror_r(errno,error_buf,SMALL_BUF);
		error_str = error_buf;
#else
		error_str = strerror_r(errno, error_buf, SMALL_BUF);
#endif
		close(fd);
		file_unlock(filename);
		ERR_DESIGN("Error read on file " + std::string(file_path) + ": " + std::string(error_str) + "\n");
		return false;
	}

	close(fd);
	file_unlock(filename);

	contents.assign(buf.data(), size);

	return true;
}

/****************************
 file_set_contents
 filename is "table/dir/file"
 return false on error
 ****************************/
int file_set_contents(const std::string &filename, const std::string &contents)
{
	int fd;
	ssize_t size;
	char error_buf[SMALL_BUF];
	char *error_str;

	const std::string file_path = base_directory + "/" + filename;

	file_lock(filename);

	fd = creat(file_path.c_str(),
	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (fd == -1)
	{
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
		strerror_r(errno,error_buf,SMALL_BUF);
		error_str = error_buf;
#else
		error_str = strerror_r(errno, error_buf, SMALL_BUF);
#endif
		file_unlock(filename);
		ERR_DESIGN("Error open on file " + std::string(file_path) + ": " + std::string(error_str) + "\n");
		return false;
	}

	size = write(fd, contents.data(), contents.size());
	if (size == -1)
	{
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
		strerror_r(errno,error_buf,SMALL_BUF);
		error_str = error_buf;
#else
		error_str = strerror_r(errno, error_buf, SMALL_BUF);
#endif
		close(fd);
		file_unlock(filename);
		ERR_DESIGN("Error write on file " + std::string(file_path) + ": " + std::string(error_str) + "\n");
		return false;
	}

	close(fd);

	file_unlock(filename);

	return true;
}

/******************************************************
 return true on success
 ******************************************************/
bool file_copy(const std::string &src_table, const std::string &src_name,
		const std::string &dst_table, const std::string &dst_name)
{
	FILE *src;
	FILE *dst;

	const std::string src_full_path = base_directory + "/"
			+ std::string(src_table) + "/" + std::string(src_name);

	src = fopen(src_full_path.c_str(), "rb");
	if (src == nullptr)
	{
		ERR_DESIGN("Failed to open source file " + std::string(src_full_path) + "\n");
		return false;
	}

	const std::string dst_full_path = base_directory + "/"
			+ std::string(dst_table) + "/" + std::string(dst_name);

	dst = fopen(dst_full_path.c_str(), "wb");
	if (dst == nullptr)
	{
		ERR_DESIGN("Failed to open destination file " + std::string(dst_full_path) + "\n");
		fclose(src);
		return false;
	}

	for (int i = getc(src); i != EOF; i = getc(src))
	{
		putc(i, dst);
	}

	fclose(src);
	fclose(dst);

	return true;
}

/***************************************************
 create directory of file path
 file_path is a path + a file name. Only the path is
 created here, not the file itself.
 return 0 if directory was successfully created
 ****************************************************/
int file_create_directory(const std::string &file_path)
{
	const std::string path_without_file = file_path.substr(0,
			file_path.find_last_of("\\/"));
	;

	return mkdir_all(path_without_file);;
}

/***************************************************
 Delete a file from file system
 return 0 if file was successfully deleted
 ****************************************************/
int file_delete(const std::string &table, const std::string &filename)
{
	const std::string file_path = base_directory + "/" + table + "/" + filename;
	return unlink(file_path.c_str());
}

/***************************************************
 return time stamp of last update
 ****************************************************/
Uint32 file_get_timestamp(const char *table, const char *file_name)
{
	file_t *file_data;
	Uint32 time_stamp = 0;

	const std::string table_path = std::string(table) + "/"
			+ std::string(file_name);

	LockGuard guard(file_list_lock);

	file_data = find_file(table_path);
	if (file_data != nullptr)
	{
		time_stamp = file_data->timestamp;
	}

	return time_stamp;
}

/*****************************************************************************/
void file_write(const std::string &filePath, const std::string &data)
{
	file_lock(filePath);

	try
	{
		std::ofstream stream(base_directory + "/" + filePath);
		stream << data;

	} catch (...)
	{
		file_unlock(filePath);
	}
	file_unlock(filePath);
}
