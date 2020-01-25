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

#include "Context.h"
#include "client_server.h"
#include "const.h"
#include "file.h"
#include "list.h"
#include "log.h"
#include "protocol.h"
#include "mutex.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libconfig.h>
#include <SDL_mutex.h>
#include <string>
#include <strings.h>
#include <sys/stat.h>

list_t * entry_list = nullptr;

/*********************
 *********************/
static void config_print_error(const std::string & file, const config_t * config)
{
	werr(LOGUSER, "libconfig error %s@%d : %s\n", file.c_str(), config_error_line(config), config_error_text(config));
}

/*********************
 *********************/
static void write_config(const config_t * config, const char * table, const char * file)
{
	const std::string file_name = std::string(table) + "/" + std::string(file);

	const std::string file_path = base_directory + "/" + file_name;

	file_lock(file_name.c_str());
	config_write_file((config_t*) config, file_path.c_str());
	file_unlock(file_name.c_str());
}
/*********************
 *********************/
static void free_config(config_t * config)
{
	config_destroy(config);
	free(config);
}
/*********************
 *********************/
static const config_t * load_config(const char * filename)
{
	int ret;
	config_t * config = nullptr;
	struct stat sts;

	const std::string file_path = base_directory + "/" + std::string(filename);

	config = (config_t*) malloc(sizeof(config_t));
	if (config == nullptr)
	{
		return nullptr;
	}
	bzero(config, sizeof(config_t));

	ret = config_read_file(config, file_path.c_str());
	if (ret == CONFIG_FALSE)
	{
		// For client, only print errors when file is present
		// If file is not present, let get_config ask for a network update
		if (stat(file_path.c_str(), &sts) != -1 || client_server == SERVER)
		{
			config_print_error(file_path, config);
		}
		return nullptr;
	}

	return config;
}

/****************************************************
 Remove an entry from the DB
 ******************************************************/
void entry_remove(const char * filename)
{
	config_t * old_config;

	wlog(LOGDEVELOPER, "Removing entry : %s", filename);
	// Clean-up old anim if any
	SDL_LockMutex(entry_mutex);
	old_config = (config_t*) list_find(entry_list, filename);
	if (old_config)
	{
		free_config(old_config);
	}

	list_update(&entry_list, filename, nullptr);

	SDL_UnlockMutex(entry_mutex);
}

/*********************
 returned config should not be freed
 *********************/
static const config_t * get_config(const char * table, const char * file)
{
	if (file == nullptr)
	{
		return nullptr;
	}

	const config_t * config = nullptr;
	std::string file_name;

	if (table == nullptr)
	{
		file_name = std::string(file);
	}
	else
	{
		file_name = std::string(table) + "/" + std::string(file);
	}

//	wlog(LOGDEBUG,"Entry get : %s",filename);

	config = (config_t*) list_find(entry_list, file_name.c_str());

	if (config != nullptr)
	{
//		wlog(LOGDEBUG,"Entry found : %s",filename);
		return config;
	}

	file_lock(file_name.c_str());

//	wlog(LOGDEBUG,"Entry asked : %s",filename);
	file_update(context_get_player(), file_name.c_str());

	if ((config = load_config(file_name.c_str())) == nullptr)
	{
		file_unlock(file_name.c_str());
		return nullptr;
	}
	file_unlock(file_name.c_str());

//	wlog(LOGDEBUG,"Entry loaded : %s",filename);
	list_update(&entry_list, file_name.c_str(), (config_t*) config);

	return config;
}

/*********************
 returned string MUST BE FREED
 *********************/
static std::string add_entry_to_path(char * path, char * entry)
{
	if (path == nullptr)
	{
		return std::string(entry);
	}
	else
	{
		const std::string new_path = std::string(path) + "." + std::string(entry);
		return new_path;
	}
}

/*********************
 Create a libconfig path from a list of string
 the returned path MUST BE FREED
 *********************/
static char * get_path(va_list ap)
{
	char * path = nullptr;
	char * entry = nullptr;

	entry = va_arg(ap, char*);
	if (entry == nullptr)
	{
		return nullptr;
	}

	while (entry != nullptr)
	{
		const std::string new_path = add_entry_to_path(path, entry);
		free(path);
		path = strdup(new_path.c_str());
		if (path == nullptr)
		{
			return nullptr;
		}
		entry = va_arg(ap, char*);
	}

	return path;
}

/*********************
 return false on error
 *********************/
static int __read_int(const char * table, const char * file, int * res, va_list ap)
{
	const config_t * config = nullptr;
	char * path = nullptr;
	int result;

	SDL_LockMutex(entry_mutex);
	config = get_config(table, file);
	if (config == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	path = get_path(ap);
	if (path == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	if (config_lookup_int(config, path, &result) == CONFIG_FALSE)
	{
		SDL_UnlockMutex(entry_mutex);
		free(path);
		return false;
	}
	free(path);

	SDL_UnlockMutex(entry_mutex);

	*res = result;

	return true;
}

/*********************
 return false on error
 *********************/
int entry_read_int(const char * table, const char * file, int * res, ...)
{
	int ret;
	va_list ap;

	va_start(ap, res);
	ret = __read_int(table, file, res, ap);
	va_end(ap);

	return ret;
}

/*********************
 return false on error
 res MUST BE FREED by caller
 *********************/
static int __read_string(const char * table, const char * file, char ** res, va_list ap)
{
	const config_t * config = nullptr;
	char * path = nullptr;
	const char * result = nullptr;

	*res = nullptr;

	SDL_LockMutex(entry_mutex);
	config = get_config(table, file);
	if (config == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	path = get_path(ap);
	if (path == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	if (config_lookup_string(config, path, &result) == CONFIG_FALSE)
	{
//		g_warning("%s: Can't read %s/%s/%s",__func__,table,file,path);
		SDL_UnlockMutex(entry_mutex);
		free(path);
		return false;
	}
	free(path);
	SDL_UnlockMutex(entry_mutex);

	*res = strdup(result);

	return true;
}

/*********************
 Fill res with a pointer to a string.
 This string MUST BE FREED by caller.
 return false on error
 *********************/
int entry_read_string(const char * table, const char * file, char ** res, ...)
{
	int ret;
	va_list ap;

	va_start(ap, res);
	ret = __read_string(table, file, res, ap);
	va_end(ap);

	return ret;
}

/*********************
 return false on error
 res MUST BE FREED by caller
 *********************/
static int __read_list_index(const char * table, const char * file, char ** res, int index, va_list ap)
{
	const config_t * config = nullptr;
	config_setting_t * setting = nullptr;
	char * path;
	const char * result;

	*res = nullptr;

	SDL_LockMutex(entry_mutex);
	config = get_config(table, file);
	if (config == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	path = get_path(ap);
	if (path == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	setting = config_lookup(config, path);
	if (setting == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		free(path);
		return false;
	}
	free(path);

	result = config_setting_get_string_elem(setting, index);
	if (result == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	SDL_UnlockMutex(entry_mutex);
	*res = strdup(result);
	return true;
}

/*********************
 return false on error
 *********************/
int entry_read_list_index(const char * table, const char * file, char ** res, int index, ...)
{
	int ret;
	va_list ap;

	va_start(ap, index);
	ret = __read_list_index(table, file, res, index, ap);
	va_end(ap);

	return ret;
}

/*********************
 return false on error
 res must be freed with deep_free
 *********************/
static int __read_list(const char * table, const char * file, char *** res, va_list ap)
{
	const config_t * config = nullptr;
	config_setting_t * setting = nullptr;
	char * path;
	int i = 0;
	const char * elem;

	*res = nullptr;

	SDL_LockMutex(entry_mutex);
	config = get_config(table, file);
	if (config == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	path = get_path(ap);
	if (path == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	setting = config_lookup(config, path);
	if (setting == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		free(path);
		return false;
	}
	free(path);

	while ((elem = config_setting_get_string_elem(setting, i)) != nullptr)
	{
		i++;
		*res = (char**) realloc(*res, (i + 1) * sizeof(char *));
		(*res)[i - 1] = strdup(elem);
		(*res)[i] = nullptr;

	}

	SDL_UnlockMutex(entry_mutex);

	if (*res == nullptr)
	{
		return false;
	}

	return true;
}

/*********************
 return false on error
 res must be freed with deep_free
 *********************/
int entry_read_list(const char * table, const char * file, char *** res, ...)
{
	int ret;
	va_list ap;

	va_start(ap, res);
	ret = __read_list(table, file, res, ap);
	va_end(ap);

	return ret;
}

/*********************
 *********************/
static config_setting_t * create_tree(const config_t * config, config_setting_t * prev_setting, char * prev_entry, const char * path, int type, va_list ap)
{
	char * entry = nullptr;
	config_setting_t * setting = nullptr;
	config_setting_t * ret = nullptr;

	if (prev_setting == nullptr)
	{
		prev_setting = config_root_setting(config);
	}

	entry = va_arg(ap, char*);

	// no more leaf
	if (entry == nullptr)
	{
		// check if leaf exists
		setting = config_lookup(config, path);
		if (setting == nullptr)
		{
			// create the leaf
			setting = config_setting_add(prev_setting, prev_entry, type);
		}
		return setting;
	}

	// still not finished
	// check if we need to create a group
	if (path)
	{
		setting = config_lookup(config, path);
		if (setting == nullptr)
		{
			// create a group with previous path
			setting = config_setting_add(prev_setting, prev_entry,
			CONFIG_TYPE_GROUP);
			if (setting == nullptr)
			{
				return nullptr;
			}
		}
	}

	// update path
	std::string new_path;
	if (path != nullptr)
	{
		new_path = std::string(path) + "." + std::string(entry);
	}
	else
	{
		new_path = std::string(entry);
	}

	ret = create_tree(config, setting, entry, new_path.c_str(), type, ap);

	return ret;
}

/*********************
 return false on error
 *********************/
static int __write_int(const char * table, const char * file, int data, va_list ap)
{
	config_setting_t * setting;
	const config_t * config;

	SDL_LockMutex(entry_mutex);
	config = get_config(table, file);
	if (config == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	setting = create_tree(config, nullptr, nullptr, nullptr, CONFIG_TYPE_INT, ap);

	/* update int */
	if (config_setting_set_int(setting, data) == CONFIG_FALSE)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	write_config(config, table, file);
	SDL_UnlockMutex(entry_mutex);
	return true;
}

/*********************
 return false on error
 *********************/
int entry_write_int(const char * table, const char * file, int data, ...)
{
	int ret;
	va_list ap;

	va_start(ap, data);
	ret = __write_int(table, file, data, ap);
	va_end(ap);

	return ret;
}

/*********************
 return false on error
 *********************/
static int __write_string(const char * table, const char * file, const char * data, va_list ap)
{
	config_setting_t * setting;
	const config_t * config;

	SDL_LockMutex(entry_mutex);
	config = get_config(table, file);
	if (config == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	setting = create_tree(config, nullptr, nullptr, nullptr, CONFIG_TYPE_STRING, ap);

	/* update string */
	if (config_setting_set_string(setting, data) == CONFIG_FALSE)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	write_config(config, table, file);
	SDL_UnlockMutex(entry_mutex);
	return true;
}

/*********************
 return false on error
 *********************/
int entry_write_string(const char * table, const char * file, const char * data, ...)
{
	int ret;
	va_list ap;

	va_start(ap, data);
	ret = __write_string(table, file, data, ap);
	va_end(ap);

	return ret;
}

/*********************
 return false on error
 *********************/
static int __write_list_index(const char * table, const char * file, const char * data, int index, va_list ap)
{
	config_setting_t * setting = nullptr;
	const config_t * config;
	int list_size;

	SDL_LockMutex(entry_mutex);
	config = get_config(table, file);
	if (config == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	setting = create_tree(config, nullptr, nullptr, nullptr, CONFIG_TYPE_ARRAY, ap);

	// Create empty entry before index
	list_size = config_setting_length(setting);
	while (list_size <= index)
	{
		if (config_setting_set_string_elem(setting, -1, "") == nullptr)
		{
			SDL_UnlockMutex(entry_mutex);
			return false;
		}
		list_size++;
	}

	if (config_setting_set_string_elem(setting, index, data) == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	write_config(config, table, file);
	SDL_UnlockMutex(entry_mutex);
	return true;
}

/*********************
 return false on error
 *********************/
int entry_write_list_index(const char * table, const char * file, const char * data, int index, ...)
{
	int ret;
	va_list ap;

	va_start(ap, index);
	ret = __write_list_index(table, file, data, index, ap);
	va_end(ap);

	return ret;
}

/*********************
 return false on error
 *********************/
static int __write_list(const char * table, const char * file, char ** data, va_list ap)
{
	config_setting_t * setting;
	const config_t * config;
	int i = 0;

	SDL_LockMutex(entry_mutex);
	config = get_config(table, file);
	if (config == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	setting = create_tree(config, nullptr, nullptr, nullptr, CONFIG_TYPE_ARRAY, ap);

	while (data[i] != nullptr)
	{
		// First try to update existing setting
		if (config_setting_set_string_elem(setting, i, data[i]) == nullptr)
		{
			// If it down not exist, add a new setting at the end of the list
			if (config_setting_set_string_elem(setting, -1, data[i]) == nullptr)
			{
				SDL_UnlockMutex(entry_mutex);
				return false;
			}
		}
		i++;
	}

	write_config(config, table, file);
	SDL_UnlockMutex(entry_mutex);

	return true;
}

/*********************
 return false on error
 *********************/
int entry_write_list(const char * table, const char * file, char ** data, ...)
{
	int ret;
	va_list ap;

	va_start(ap, data);
	ret = __write_list(table, file, data, ap);
	va_end(ap);

	return ret;
}

/*********************
 return false on error
 *********************/
static int __add_to_list(const char * table, const char * file, const char * to_be_added, va_list ap)
{
	const config_t * config = nullptr;
	config_setting_t * setting = nullptr;
	char * path;

	SDL_LockMutex(entry_mutex);
	config = get_config(table, file);
	if (config == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	path = get_path(ap);
	if (path == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	setting = config_lookup(config, path);
	if (setting == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		free(path);
		return false;
	}
	free(path);

	if (config_setting_set_string_elem(setting, -1, to_be_added) == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	write_config(config, table, file);
	SDL_UnlockMutex(entry_mutex);

	return true;
}

/*********************
 return false on error
 *********************/
int entry_add_to_list(const char * table, const char * file, const char * to_be_added, ...)
{
	int ret;
	va_list ap;

	va_start(ap, to_be_added);
	ret = __add_to_list(table, file, to_be_added, ap);
	va_end(ap);

	return ret;
}

/*********************
 return false on error
 *********************/
static int __remove_group(const char * table, const char * file, const char * group, va_list ap)
{

	const config_t * config;
	config_setting_t * setting = nullptr;
	char * path;

	SDL_LockMutex(entry_mutex);
	config = get_config(table, file);
	if (config == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	path = get_path(ap);
	if (path != nullptr)
	{
		setting = config_lookup(config, path);
		free(path);
	}
	else
	{
		setting = config_root_setting(config);
	}

	if (setting == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	if (config_setting_remove(setting, group) == CONFIG_FALSE)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	write_config(config, table, file);
	SDL_UnlockMutex(entry_mutex);

	return true;
}

/*********************
 return false on error
 *********************/
int entry_remove_group(const char * table, const char * file, const char * group, ...)
{
	int ret;
	va_list ap;

	va_start(ap, group);
	ret = __remove_group(table, file, group, ap);
	va_end(ap);

	return ret;
}
/**********************
 get_unused_list_entry
 find an unused tag in a list, add it to the list and return it.
 returned string must be freed
 **********************/
char * entry_get_unused_list_entry(const char * table, const char * file, ...)
{
	char ** list;
	char tag[7];
	int index = 0;
	int i = 0;
	va_list ap;

	sprintf(tag, "A%05x", index);

	va_start(ap, file);
	if (__read_list(table, file, &list, ap))
	{
		while (list[i] != nullptr)
		{
			if (strcmp(list[i], tag) == 0)
			{
				index++;
				sprintf(tag, "A%05x", index);
				i = 0;
			}
			i++;
		}
	}
	va_end(ap);

	va_start(ap, file);
	if (!__add_to_list(table, file, tag, ap))
	{
		va_end(ap);
		return nullptr;
	}
	va_end(ap);

	return strdup(tag);
}

/**********************
 __get_unused_group
 find an unused group, create it and return its name
 returned string must be freed
 **********************/
char * __get_unused_group(const char * table, const char * file, va_list ap)
{
	char tag[7];
	int index = 0;
	char * path;
	config_setting_t * setting;
	const config_t * config;

	SDL_LockMutex(entry_mutex);
	config = get_config(table, file);
	if (config == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return nullptr;
	}

	sprintf(tag, "A%05x", index);

	path = get_path(ap);

	if (path != nullptr)
	{
		setting = config_lookup(config, path);
		free(path);
	}
	else
	{
		setting = config_root_setting(config);
	}

	while (config_setting_add(setting, tag, CONFIG_TYPE_GROUP) == nullptr)
	{
		index++;
		sprintf(tag, "A%05x", index);
	}
	SDL_UnlockMutex(entry_mutex);

	return strdup(tag);
}

/**********************
 get_unused_group
 find an unused group, create it and return its name
 returned string must be freed
 **********************/
char * entry_get_unused_group(const char * table, const char * file, ...)
{
	char * ret;
	va_list ap;

	va_start(ap, file);
	ret = __get_unused_group(table, file, ap);
	va_end(ap);

	return ret;
}

/**********************
 __get_unused_group_on_path
 find an unused group, DO NOT CREATE IT and return its name
 returned string must be freed
 return nullptr on error
 **********************/
char * __get_unused_group_on_path(const char * table, const char * file, char * path)
{
	char tag[7];
	int index = 0;
	config_setting_t * setting;
	const config_t * config;

	SDL_LockMutex(entry_mutex);
	config = get_config(table, file);
	if (config == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return nullptr;
	}

	sprintf(tag, "A%05x", index);

	if (path != nullptr)
	{
		setting = config_lookup(config, path);
		free(path);
	}
	else
	{
		setting = config_root_setting(config);
	}

	while (config_setting_add(setting, tag, CONFIG_TYPE_GROUP) == nullptr)
	{
		index++;
		sprintf(tag, "A%05x", index);
	}

	config_setting_remove(setting, tag);
	SDL_UnlockMutex(entry_mutex);

	return strdup(tag);
}

/**********************
 Return a list of the name of the elements in the specified group
 res must be freed  (deep_free)
 return false on error
 **********************/
int entry_get_group_list(const char * table, const char * file, char *** res, ...)
{
	char * path;
	const config_t * config;
	va_list ap;
	config_setting_t * setting;
	config_setting_t * elem_setting;
	int index = 0;

	*res = nullptr;

	SDL_LockMutex(entry_mutex);
	config = get_config(table, file);
	if (config == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	va_start(ap, res);
	path = get_path(ap);
	va_end(ap);

	if (path != nullptr && path[0] != 0)
	{
		setting = config_lookup(config, path);
		free(path);
	}
	else
	{
		setting = config_root_setting(config);
	}

	/* The path does not exist in the conf file */
	if (setting == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	while ((elem_setting = config_setting_get_elem(setting, index)) != nullptr)
	{
		index++;
		*res = (char **) realloc(*res, (index + 1) * sizeof(char *));
		(*res)[index - 1] = strdup(config_setting_name(elem_setting));
		(*res)[index] = nullptr;
	}

	SDL_UnlockMutex(entry_mutex);

	if (*res == nullptr)
	{
		return false;
	}

	return true;
}

/*********************
 return false on failure
 *********************/
static int __remove_from_list(const char * table, const char * file, const char * to_be_removed, va_list ap)
{
	const config_t * config = nullptr;
	config_setting_t * setting = nullptr;
	char * path;
	int i = 0;
	const char * elem;

	SDL_LockMutex(entry_mutex);
	config = get_config(table, file);
	if (config == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	path = get_path(ap);
	if (path == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	setting = config_lookup(config, path);
	if (setting == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		free(path);
		return false;
	}
	free(path);

	while ((elem = config_setting_get_string_elem(setting, i)) != nullptr)
	{
		if (strcmp(elem, to_be_removed) == 0)
		{
			if (config_setting_remove_elem(setting, i) == CONFIG_FALSE)
			{
				SDL_UnlockMutex(entry_mutex);
				return false;
			}

			write_config(config, table, file);
			SDL_UnlockMutex(entry_mutex);
			return true;
		}
		i++;
	}

	SDL_UnlockMutex(entry_mutex);
	return false;
}

/*********************
 return false on error
 *********************/
int entry_remove_from_list(const char * table, const char * file, const char * to_be_removed, ...)
{
	int ret;
	va_list ap;

	va_start(ap, to_be_removed);
	ret = __remove_from_list(table, file, to_be_removed, ap);
	va_end(ap);
	return ret;
}

/*********************
 return false on error
 *********************/
int entry_copy_config(config_setting_t * source, config_setting_t * destination);
int entry_copy_aggregate(config_setting_t * source, config_setting_t * dest, int type)
{
	const char * setting_name;
	config_setting_t * new_dest;
	int index = 0;
	char tag[7];

	setting_name = config_setting_name(source);
	if (setting_name == nullptr)
	{
		return false;
	}

	new_dest = config_setting_add(dest, setting_name, type);
	if (new_dest == nullptr)
	{
		// Try to find an available name
		sprintf(tag, "A%05x", index);

		while ((new_dest = config_setting_add(dest, setting_name, type)) == nullptr)
		{
			index++;
			sprintf(tag, "A%05x", index);
		}
	}

	if (!entry_copy_config(source, new_dest))
	{
		config_setting_remove(dest, setting_name);
		return false;
	}
	return true;
}

/*********************
 return false on error
 *********************/
int entry_copy_config(config_setting_t * source, config_setting_t * dest)
{
	config_setting_t * new_source;
	config_setting_t * new_dest;
	int index = -1;
	int int_value;
	long long long_value;
	double double_value;
	const char * string;

	while ((new_source = config_setting_get_elem(source, index + 1)) != nullptr)
	{
		index++;
		if (config_setting_is_group(new_source))
		{
			if (!entry_copy_aggregate(new_source, dest, CONFIG_TYPE_GROUP))
			{
				return false;
			}
		}

		else if (config_setting_is_array(new_source))
		{
			if (!entry_copy_aggregate(new_source, dest, CONFIG_TYPE_ARRAY))
			{
				return false;
			}
		}

		else if (config_setting_is_list(new_source))
		{
			if (!entry_copy_aggregate(new_source, dest, CONFIG_TYPE_LIST))
			{
				return false;
			}
		}

		else
		{
			switch (config_setting_type(new_source))
			{
			case CONFIG_TYPE_INT:
				int_value = config_setting_get_int(new_source);
				new_dest = config_setting_add(dest, config_setting_name(new_source), CONFIG_TYPE_INT);
				config_setting_set_int(new_dest, int_value);
				continue;
			case CONFIG_TYPE_INT64:
				long_value = config_setting_get_int64(new_source);
				new_dest = config_setting_add(dest, config_setting_name(new_source), CONFIG_TYPE_INT64);
				config_setting_set_int64(new_dest, long_value);
				continue;
			case CONFIG_TYPE_FLOAT:
				double_value = config_setting_get_float(new_source);
				new_dest = config_setting_add(dest, config_setting_name(new_source), CONFIG_TYPE_FLOAT);
				config_setting_set_float(new_dest, double_value);
				continue;
			case CONFIG_TYPE_BOOL:
				int_value = config_setting_get_bool(new_source);
				new_dest = config_setting_add(dest, config_setting_name(new_source), CONFIG_TYPE_BOOL);
				config_setting_set_bool(new_dest, int_value);
				continue;
			case CONFIG_TYPE_STRING:
				string = config_setting_get_string(new_source);
				new_dest = config_setting_add(dest, config_setting_name(new_source), CONFIG_TYPE_STRING);
				config_setting_set_string(new_dest, string);
				continue;
			default:
				return false;
			}
		}
	}

	return true;
}

/*********************
 returnRET_NOK on error
 *********************/
static int __list_create(const char * table, const char * file, va_list ap)
{
	const config_t * config;

	SDL_LockMutex(entry_mutex);
	config = get_config(table, file);
	if (config == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	create_tree(config, nullptr, nullptr, nullptr, CONFIG_TYPE_LIST, ap);

	write_config(config, table, file);
	SDL_UnlockMutex(entry_mutex);
	return true;
}

/*********************
 return false on error
 *********************/
int entry_list_create(const char * table, const char * file, ...)
{
	int ret = false;
	va_list ap;

	va_start(ap, file);
	ret = __list_create(table, file, ap);
	va_end(ap);

	return ret;
}

/*********************
 return false on error
 *********************/
static int __group_create(const char * table, const char * file, va_list ap)
{
	const config_t * config = nullptr;

	SDL_LockMutex(entry_mutex);
	config = get_config(table, file);
	if (config == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	create_tree(config, nullptr, nullptr, nullptr, CONFIG_TYPE_GROUP, ap);

	write_config(config, table, file);
	SDL_UnlockMutex(entry_mutex);
	return true;
}

/*********************
 return false on error
 *********************/
int entry_group_create(const char * table, const char * file, ...)
{
	int ret;
	va_list ap;

	va_start(ap, file);
	ret = __group_create(table, file, ap);
	va_end(ap);

	return ret;
}

/*********************
 return nullptr on error
 *********************/
static char * __copy_group(const char * src_table, const char * src_file, const char * dst_table, const char * dst_file, const char * group_name, va_list ap)
{
	const config_t * src_config = nullptr;
	const config_t * dst_config = nullptr;
	config_setting_t * src_setting = nullptr;
	config_setting_t * dst_setting = nullptr;
	char * path = nullptr;
	char * new_group_name = nullptr;
	const char * group_name_used = nullptr;

	path = get_path(ap);
	std::string full_path = add_entry_to_path(path, (char *) group_name);

	SDL_LockMutex(entry_mutex);
	src_config = get_config(src_table, src_file);
	if (src_config == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		free(path);
		return nullptr;
	}

	src_setting = config_lookup(src_config, full_path.c_str());
	if (src_setting == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		free(path);
		return nullptr;
	}

	dst_config = get_config(dst_table, dst_file);
	if (dst_config == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		free(path);
		return nullptr;
	}

	dst_setting = config_lookup(dst_config, full_path.c_str());

	// if the setting does not exist, create it
	if (dst_setting == nullptr)
	{
		group_name_used = group_name;
	}
	// else find a new name for it
	else
	{
		new_group_name = __get_unused_group_on_path(dst_table, dst_file, path);
		group_name_used = new_group_name;
	}

	if (path != nullptr)
	{
		dst_setting = config_lookup(dst_config, path);
	}
	else
	{
		dst_setting = config_root_setting(dst_config);
	}
	dst_setting = config_setting_add(dst_setting, group_name_used,
	CONFIG_TYPE_GROUP);

	free(path);

	if (!entry_copy_config(src_setting, dst_setting))
	{
		SDL_UnlockMutex(entry_mutex);
		return nullptr;
	}

	SDL_UnlockMutex(entry_mutex);
	if (new_group_name != nullptr)
	{
		return new_group_name;
	}

	return strdup(group_name);

}
/**************************************
 return a copy of the name used for the destination
 MUST BE FREED !
 ***************************************/
char * entry_copy_group(const char * src_table, const char * src_file, const char * dst_table, const char * dst_file, const char * group_name, ...)
{
	char * ret;
	va_list ap;

	va_start(ap, group_name);
	ret = __copy_group(src_table, src_file, dst_table, dst_file, group_name, ap);
	va_end(ap);
	return ret;
}

/*********************************************
 Update an entry from a network frame
 return false on error
 *********************************************/
int entry_update(const std::string & type, const std::string & table, const std::string & file, const std::string & path, const std::string & value)
{
	SDL_LockMutex(entry_mutex);

	const config_t * l_pConfig = get_config(table.c_str(), file.c_str());
	if (l_pConfig == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	config_setting_t * l_pSetting = nullptr;

	l_pSetting = config_lookup(l_pConfig, path.c_str());
	if (l_pSetting == nullptr)
	{
		SDL_UnlockMutex(entry_mutex);
		return false;
	}

	if (type == ENTRY_TYPE_INT)
	{
		int l_IntValue;

		l_IntValue = atoll(value.c_str());
		if (config_setting_set_int(l_pSetting, l_IntValue) == CONFIG_FALSE)
		{
			werr(LOGUSER, "Error setting %s/%s/%s to %d", table.c_str(), file.c_str(), path.c_str(), l_IntValue);
		}
		else
		{
			write_config(l_pConfig, table.c_str(), file.c_str());
		}
	}
	else if (type == ENTRY_TYPE_STRING)
	{
		if (config_setting_set_string(l_pSetting, value.c_str()) == CONFIG_FALSE)
		{
			werr(LOGUSER, "Error setting %s/%s/%s to %s", table.c_str(), file.c_str(), path.c_str(), value.c_str());
		}
		else
		{
			write_config(l_pConfig, table.c_str(), file.c_str());
		}
	}

	SDL_UnlockMutex(entry_mutex);

	return true;
}

/***********************************************
 Delete a character's entry
 return false on error
 ***********************************************/
int entry_destroy(const char * table, const char * file)
{
	const std::string file_path = std::string(table) + "/" + std::string(file);

	entry_remove(file_path.c_str());

	return true;
}

/*********************
 return true if entry or group exists
 *********************/
static bool __exist(const char * table, const char * file, va_list ap)
{
	const config_t * config;
	char * path;
	config_setting_t * setting = nullptr;

	SDL_LockMutex(entry_mutex);
	config = get_config(table, file);
	SDL_UnlockMutex(entry_mutex);
	if (config == nullptr)
	{
		return false;
	}

	path = get_path(ap);
	if (path == nullptr)
	{
		return false;
	}

	setting = config_lookup(config, path);
	free(path);
	if (setting == nullptr)
	{
		return false;
	}

	return true;
}

/*********************
 return true if entry or group exists
 *********************/
bool entry_exist(const char * table, const char * file, ...)
{
	bool ret;
	va_list ap;

	va_start(ap, file);
	ret = __exist(table, file, ap);
	va_end(ap);

	return ret;
}
