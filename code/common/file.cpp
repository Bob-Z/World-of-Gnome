/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2019 carabobz@gmail.com

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

#include <bits/types/FILE.h>
#include "client_server.h"
#include "common.h"
#include "const.h"
#include <dirent.h>
#include <fcntl.h>
#include <features.h>
#include "file.h"
#include "list.h"
#include "log.h"
#include "mutex.h"
#include "network.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <SDL_mutex.h>
#include <SDL_stdinc.h>
#include <SDL_timer.h>
#include "types.h"
#include <unistd.h>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <utility>

list_t * file_list = nullptr;

typedef struct file_tag
{
	Uint32 timestamp;
	SDL_mutex * mutex;
} file_t;

/****************************************
 filename is "table/dir/file"
 *****************************************/
void file_lock(const char * filename)
{
	file_t * file_data;

	SDL_LockMutex(file_list_mutex);
	file_data = (file_t *) list_find(file_list, filename);
	if (file_data == nullptr)
	{
		file_data = (file_t*) malloc(sizeof(file_t));
		file_data->timestamp = 0;
		file_data->mutex = SDL_CreateMutex();

		list_update(&file_list, filename, file_data);
	}
	SDL_UnlockMutex(file_list_mutex);

	SDL_LockMutex(file_data->mutex);
}

/****************************************
 filename is "table/dir/file"
 *****************************************/
void file_unlock(const char * filename)
{
	file_t * file_data;

	SDL_LockMutex(file_list_mutex);
	file_data = (file_t *) list_find(file_list, filename);
	SDL_UnlockMutex(file_list_mutex);

	if (file_data == nullptr)
	{
		return;
	}

	SDL_UnlockMutex(file_data->mutex);
}

/****************************************
 filename is "table/dir/file"
 *****************************************/
void file_update(Context * context, const char * filename)
{
	Uint32 current_time = SDL_GetTicks();
	file_t * file_data;

	if (client_server == SERVER)
	{
		return;
	}

	SDL_LockMutex(file_list_mutex);
	file_data = (file_t *) list_find(file_list, filename);
	SDL_UnlockMutex(file_list_mutex);

	if (file_data == nullptr)
	{
		return;
	}

	// Avoid flooding the server
	if (file_data->timestamp != 0 && file_data->timestamp + FILE_REQUEST_TIMEOUT > current_time)
	{
		//wlog(LOGDEBUG,"Previous request of file  %s has been %d ms ago",filename,current_time - file_data->timestamp );
		return;
	}

	network_send_req_file(context, std::string(filename));

	file_data->timestamp = current_time;
}

/***************************************************
 return 0 if directory was successfully created
 ****************************************************/
static int mkdir_all(const std::string & path_name)
{
	char * token = nullptr;
	char * source = nullptr;
	int ret = -1;
	char * directory = nullptr;
	char * saveptr = nullptr;

	source = strdup(path_name.c_str());

	token = strtok_r(source, "/", &saveptr);

	directory = strdup("");

	while (token != nullptr)
	{
		const std::string new_directory = std::string(directory) + "/" + std::string(token);
		free(directory);
		directory = strdup(new_directory.c_str());
		ret = mkdir(directory, 0775);
		token = strtok_r(nullptr, "/", &saveptr);
	}

	free(directory);
	free(source);

	return ret;
}

/****************************
 Parameter table: Name of the table to create the new file
 Parameter suggested_name: Name of the file you want to create. If empty, an available file is created.
 If the suggested name is not available the function return false.
 Return the name of an available empty file.
 On success the file is created on disk
 ****************************/
std::pair<bool, std::string> file_new(const std::string & table, const std::string & suggested_name)
{
	DIR * dir = nullptr;
	struct dirent * ent = nullptr;
	char tag[10];
	int index = 0;
	int fd = -1;
	struct stat sts;

	const std::string dir_path = base_directory + "/" + table;

	std::string selected_name;

	SDL_LockMutex(character_dir_mutex);

	if (suggested_name != NO_SUGGESTED_NAME)
	{
		const std::string file_path = dir_path + "/" + std::string(suggested_name);
		if (stat(file_path.c_str(), &sts) != -1)
		{
			SDL_UnlockMutex(character_dir_mutex);

			werr(LOGDEVELOPER, "Suggested file %s already exists", suggested_name.c_str());
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
				werr(LOGDEVELOPER, "Cannot create path %s", dir_path.c_str());

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

	SDL_UnlockMutex(character_dir_mutex);

	return std::pair<bool, std::string>
	{ true, selected_name };
}

/****************************
 filename is "table/dir/file"
 Fill contents and size with data from filename file.
 contents MUST BE FREED by caller
 return RET_NOK on error
 ****************************/
ret_code_t file_get_contents(const char *filename, void **contents, int_fast32_t *length)
{
	struct stat sts;
	int fd;
	ssize_t size;
	char error_buf[SMALL_BUF];
	char * error_str;

	*contents = nullptr;

	const std::string file_path = base_directory + "/" + std::string(filename);

	file_lock(file_path.c_str());

	if (stat(file_path.c_str(), &sts) == -1)
	{
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
		strerror_r(errno,error_buf,SMALL_BUF);
		error_str = error_buf;
#else
		error_str = strerror_r(errno, error_buf, SMALL_BUF);
#endif
		file_unlock(filename);
		werr(LOGDESIGNER, "Error stat on file %s: %s\n", file_path.c_str(), error_str);
		return RET_NOK;
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
		werr(LOGDESIGNER, "Error open on file %s: %s\n", file_path.c_str(), error_str);
		return RET_NOK;
	}

	void * buf;
	buf = malloc(sts.st_size);
	if (buf == nullptr)
	{
		close(fd);
		file_unlock(filename);
		return RET_NOK;
	}

	size = read(fd, buf, sts.st_size);
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
		free(buf);
		werr(LOGDESIGNER, "Error read on file %s: %s\n", file_path.c_str(), error_str);
		return RET_NOK;
	}

	close(fd);
	file_unlock(filename);

	*contents = buf;
	*length = size;

	return RET_OK;
}

/****************************
 file_set_contents
 filename is "table/dir/file"
 return RET_NOK on error
 ****************************/
ret_code_t file_set_contents(const char *filename, const void *contents, int length)
{
	int fd;
	ssize_t size;
	char error_buf[SMALL_BUF];
	char * error_str;

	const std::string file_path = base_directory + "/" + std::string(filename);

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
		werr(LOGDESIGNER, "Error open on file %s: %s\n", file_path.c_str(), error_str);
		return RET_NOK;
	}

	size = write(fd, contents, length);
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
		werr(LOGDESIGNER, "Error write on file %s: %s\n", file_path.c_str(), error_str);
		return RET_NOK;
	}

	close(fd);

	file_unlock(filename);

	return RET_OK;
}

/******************************************************
 return true on success
 ******************************************************/
bool file_copy(const char * src_table, const char * src_name, const char * dst_table, const char * dst_name)
{
	FILE *src;
	FILE *dst;

	const std::string src_full_path = base_directory + "/" + std::string(src_table) + "/" + std::string(src_name);

	src = fopen(src_full_path.c_str(), "rb");
	if (src == nullptr)
	{
		werr(LOGDESIGNER, "Failed to open source file %s\n", src_full_path);
		return false;
	}

	const std::string dst_full_path = base_directory + "/" + std::string(dst_table) + "/" + std::string(dst_name);

	dst = fopen(dst_full_path.c_str(), "wb");
	if (dst == nullptr)
	{
		werr(LOGDESIGNER, "Failed to open destination file %s\n", dst_full_path);
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
int file_create_directory(const std::string & file_path)
{
	const std::string path_without_file = file_path.substr(0, file_path.find_last_of("\\/"));
	;

	return mkdir_all(path_without_file);;
}

/***************************************************
 Delete a file from file system
 return 0 if file was successfully deleted
 ****************************************************/
int file_delete(const char * table, const char * filename)
{
	const std::string file_path = base_directory + "/" + std::string(table) + "/" + std::string(filename);
	return unlink(file_path.c_str());
}

/***************************************************
 return timestamp of last update
 ****************************************************/
Uint32 file_get_timestamp(const char * table, const char * file_name)
{
	file_t * file_data;
	Uint32 time_stamp = 0;

	const std::string table_path = std::string(table) + "/" + std::string(file_name);
	SDL_LockMutex(file_list_mutex);
	file_data = (file_t *) list_find(file_list, table_path.c_str());
	if (file_data != nullptr)
	{
		time_stamp = file_data->timestamp;
	}
	SDL_UnlockMutex(file_list_mutex);

	return time_stamp;
}
