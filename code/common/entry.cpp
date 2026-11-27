/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2020 carabobz@gmail.com

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "DataManager.h"
#include "LockGuard.h"
#include "log.h"
#include "mutex.h"
#include "protocol.h"

Lock entryListLock;

// Defined in main_server.cpp / main_client.cpp
DataManager & getDataManager();

/*****************************************************************************/
static std::vector<std::string> getPath(va_list ap)
{
	std::vector<std::string> path;

	char *entry = va_arg(ap, char*);
	while (entry != nullptr)
	{
		path.push_back(entry);
		entry = va_arg(ap, char*);
	}

	return path;
}

/*****************************************************************************/
static std::vector<std::string> splitPath(const std::string &path)
{
	std::vector<std::string> result;
	size_t start = 0;

	while (start < path.size())
	{
		size_t pos = path.find('.', start);
		if (pos == std::string::npos)
		{
			result.push_back(path.substr(start));
			break;
		}
		if (pos > start)
		{
			result.push_back(path.substr(start, pos - start));
		}
		start = pos + 1;
	}

	return result;
}

/*****************************************************************************/
static bool containsKey(DataManager &dm, const char *table, const char *file,
		const std::vector<std::string> &path, const std::string &key)
{
	try
	{
		for (auto &k : dm.getKeys(table, file, path))
		{
			if (k == key)
			{
				return true;
			}
		}
	} catch (...)
	{
	}

	return false;
}

/*****************************************************************************/
static bool groupExists(DataManager &dm, const char *table, const char *file,
		const std::vector<std::string> &path)
{
	try
	{
		dm.getKeys(table, file, path);
		return true;
	} catch (...)
	{
		return false;
	}
}

/*****************************************************************************/
static std::string __get_unused_group_on_path(const char *table, const char *file,
		const std::vector<std::string> &path)
{
	DataManager &dm = getDataManager();

	for (int index = 0;; index++)
	{
		char tag[16];
		sprintf(tag, "A%05x", index);

		if (!containsKey(dm, table, file, path, tag))
		{
			return std::string(tag);
		}
	}
}

/*****************************************************************************/
static std::string __get_unused_group(const char *table, const char *file, va_list ap)
{
	DataManager &dm = getDataManager();
	std::vector<std::string> path = getPath(ap);

	for (int index = 0;; index++)
	{
		char tag[16];
		sprintf(tag, "A%05x", index);

		if (groupExists(dm, table, file, path)
				&& containsKey(dm, table, file, path, tag))
		{
			continue;
		}

		std::vector<std::string> fullPath = path;
		fullPath.push_back(tag);

		if (dm.set(table, file, fullPath, json::object()))
		{
			return std::string(tag);
		}
	}
}

/*****************************************************************************/
static std::string __copy_group(const char *src_table, const char *src_file,
		const char *dst_table, const char *dst_file, const char *group_name,
		va_list ap)
{
	std::vector<std::string> path = getPath(ap);
	std::vector<std::string> srcPath = path;
	srcPath.push_back(group_name);

	DataManager &dm = getDataManager();

	json value;
	try
	{
		value = dm.get<json>(src_table, src_file, srcPath);
	} catch (...)
	{
		return std::string();
	}

	std::string group_name_used = group_name;

	if (groupExists(dm, dst_table, dst_file, srcPath))
	{
		group_name_used = __get_unused_group_on_path(dst_table, dst_file, path);
	}

	std::vector<std::string> dstPath = path;
	dstPath.push_back(group_name_used);

	if (!dm.set(dst_table, dst_file, dstPath, value))
	{
		return std::string();
	}

	return group_name_used;
}

/*****************************************************************************/
void entry_remove(const std::string &filename)
{
	wlog(LOGDEVELOPER, "Removing entry : %s", filename);
}

/*****************************************************************************/
int entry_read_int(const std::string &table, const std::string &file, int *res, ...)
{
	va_list ap;

	va_start(ap, res);
	std::vector<std::string> path = getPath(ap);
	va_end(ap);

	LockGuard guard(entryListLock);

	int value = 0;
	if (getDataManager().tryGet<int>(table, file, path, value) == false)
	{
		return false;
	}

	*res = value;
	return true;
}

/*****************************************************************************/
bool entry_read_string(const std::string &table, const std::string &file,
		std::string &res, ...)
{
	va_list ap;

	va_start(ap, res);
	std::vector<std::string> path = getPath(ap);
	va_end(ap);

	LockGuard guard(entryListLock);

	res.clear();

	return getDataManager().tryGet<std::string>(table, file, path, res);
}

/*****************************************************************************/
static bool __read_list_index(const std::string &table, const std::string &file,
		std::string &res, int index, va_list ap)
{
	res.clear();

	LockGuard guard(entryListLock);

	std::vector<std::string> path = getPath(ap);

	std::vector<std::string> list;
	if (getDataManager().tryGet<std::vector<std::string> >(table, file, path,
			list) == false)
	{
		return false;
	}

	if (index >= 0 && (size_t) index < list.size())
	{
		res = list[index];
		return true;
	}

	return false;
}

/*****************************************************************************/
bool entry_read_list_index(const std::string &table, const std::string &file,
		std::string &res, int index, ...)
{
	bool ret;
	va_list ap;

	va_start(ap, index);
	ret = __read_list_index(table, file, res, index, ap);
	va_end(ap);

	return ret;
}

/*****************************************************************************/
static int __read_list(const std::string &table, const std::string &file,
		std::vector<std::string> &res, va_list ap)
{
	res.clear();

	LockGuard guard(entryListLock);

	std::vector<std::string> path = getPath(ap);

	if (getDataManager().tryGet<std::vector<std::string> >(table, file, path,
			res) == false || res.empty())
	{
		res.clear();
		return false;
	}

	return true;
}

/*****************************************************************************/
int entry_read_list(const std::string &table, const std::string &file,
		std::vector<std::string> &res, ...)
{
	int ret;
	va_list ap;

	va_start(ap, res);
	ret = __read_list(table, file, res, ap);
	va_end(ap);

	return ret;
}

/*****************************************************************************/
int entry_write_int(const std::string &table, const std::string &file, int data, ...)
{
	int ret;
	va_list ap;

	va_start(ap, data);
	ret = getDataManager().set<int>(table, file, getPath(ap), data);
	va_end(ap);

	return ret;
}

/*****************************************************************************/
int entry_write_string(const std::string &table, const std::string &file,
		const std::string &data, ...)
{
	int ret;
	va_list ap;

	va_start(ap, data);
	ret = getDataManager().set<std::string>(table, file, getPath(ap), data);
	va_end(ap);

	return ret;
}

/*****************************************************************************/
int entry_write_list_index(const std::string &table, const std::string &file,
		const std::string &data, int index, ...)
{
	int ret = false;
	va_list ap;

	va_start(ap, index);
	std::vector<std::string> path = getPath(ap);
	va_end(ap);

	if (index < 0)
	{
		return false;
	}

	try
	{
		DataManager &dm = getDataManager();
		std::vector<std::string> list = dm.get<std::vector<std::string> >(table,
				file, path);

		if ((int) list.size() <= index)
		{
			list.resize(index + 1);
		}
		list[index] = data;

		ret = dm.set<std::vector<std::string> >(table, file, path, list);
	} catch (...)
	{
	}

	return ret;
}

/*****************************************************************************/
int entry_write_list(const char *table, const char *file, char **data, ...)
{
	int ret = false;
	va_list ap;

	va_start(ap, data);
	std::vector<std::string> path = getPath(ap);
	va_end(ap);

	std::vector<std::string> list;
	for (int i = 0; data[i] != nullptr; i++)
	{
		list.push_back(data[i]);
	}

	ret = getDataManager().set<std::vector<std::string> >(table, file, path,
			list);

	return ret;
}

/*****************************************************************************/
int entry_add_to_list(const char *table, const char *file,
		const char *to_be_added, ...)
{
	int ret = false;
	va_list ap;

	va_start(ap, to_be_added);
	std::vector<std::string> path = getPath(ap);
	va_end(ap);

	try
	{
		DataManager &dm = getDataManager();
		std::vector<std::string> list = dm.get<std::vector<std::string> >(table,
				file, path);

		list.push_back(to_be_added);

		ret = dm.set<std::vector<std::string> >(table, file, path, list);
	} catch (...)
	{
	}

	return ret;
}

/*****************************************************************************/
int entry_remove_group(const char *table, const char *file, const char *group,
		...)
{
	va_list ap;

	va_start(ap, group);
	std::vector<std::string> path = getPath(ap);
	va_end(ap);

	path.push_back(group);

	return getDataManager().erase(table, file, path);
}

/*****************************************************************************/
std::string entry_get_unused_list_entry(const char *table, const char *file, ...)
{
	char tag[16];

	DataManager &dm = getDataManager();

	va_list ap;
	va_start(ap, file);
	std::vector<std::string> path = getPath(ap);
	va_end(ap);

	std::vector<std::string> list;
	try
	{
		list = dm.get<std::vector<std::string> >(table, file, path);
	} catch (...)
	{
	}

	int index = 0;
	for (;; index++)
	{
		sprintf(tag, "A%05x", index);
		if (std::find(list.begin(), list.end(), std::string(tag)) == list.end())
		{
			break;
		}
	}

	try
	{
		list.push_back(tag);
		dm.set<std::vector<std::string> >(table, file, path, list);
	} catch (...)
	{
		return std::string();
	}

	return std::string(tag);
}

/*****************************************************************************/
std::string entry_get_unused_group(const char *table, const char *file, ...)
{
	std::string ret;
	va_list ap;

	va_start(ap, file);
	ret = __get_unused_group(table, file, ap);
	va_end(ap);

	return ret;
}

/*****************************************************************************/
int entry_get_group_list(const std::string &table, const std::string &file,
		std::vector<std::string> &res, ...)
{
	int ret = false;
	va_list ap;

	va_start(ap, res);
	std::vector<std::string> path = getPath(ap);
	va_end(ap);

	res.clear();

	try
	{
		res = getDataManager().getKeys(table, file, path);
		ret = (res.empty() == false);
	} catch (...)
	{
		res.clear();
	}

	return ret;
}

/*****************************************************************************/
int entry_remove_from_list(const char *table, const char *file,
		const char *to_be_removed, ...)
{
	int ret = false;
	va_list ap;

	va_start(ap, to_be_removed);
	std::vector<std::string> path = getPath(ap);
	va_end(ap);

	try
	{
		DataManager &dm = getDataManager();
		std::vector<std::string> list = dm.get<std::vector<std::string> >(table,
				file, path);

		for (size_t i = 0; i < list.size(); i++)
		{
			if (list[i] == to_be_removed)
			{
				list.erase(list.begin() + i);
				ret = dm.set<std::vector<std::string> >(table, file, path,
						list);
				break;
			}
		}
	} catch (...)
	{
	}

	return ret;
}

/*****************************************************************************/
int entry_list_create(const char *table, const char *file, ...)
{
	int ret;
	va_list ap;

	va_start(ap, file);
	ret = getDataManager().set<std::vector<std::string> >(table, file,
			getPath(ap), std::vector<std::string>());
	va_end(ap);

	return ret;
}

/*****************************************************************************/
int entry_group_create(const char *table, const char *file, ...)
{
	int ret;
	va_list ap;

	va_start(ap, file);
	ret = getDataManager().set<json>(table, file, getPath(ap), json::object());
	va_end(ap);

	return ret;
}

/*****************************************************************************/
std::string entry_copy_group(const char *src_table, const char *src_file,
		const char *dst_table, const char *dst_file, const char *group_name,
		...)
{
	std::string ret;
	va_list ap;

	va_start(ap, group_name);
	ret = __copy_group(src_table, src_file, dst_table, dst_file, group_name,
			ap);
	va_end(ap);

	return ret;
}

/*****************************************************************************/
int entry_update(const std::string &type, const std::string &table,
		const std::string &file, const std::string &path,
		const std::string &value)
{
	DataManager &dm = getDataManager();
	std::vector<std::string> pathVec = splitPath(path);

	if (type == ENTRY_TYPE_INT)
	{
		return dm.set<int>(table, file, pathVec, atoi(value.c_str()));
	}
	else if (type == ENTRY_TYPE_STRING)
	{
		return dm.set<std::string>(table, file, pathVec, value);
	}

	return true;
}

/*****************************************************************************/
int entry_destroy(const std::string &table, const std::string &file)
{
	const std::string filePath = table + "/" + file;

	getDataManager().reset(filePath);
	entry_remove(filePath);

	return true;
}

/*****************************************************************************/
static bool __exist(const char *table, const char *file, va_list ap)
{
	DataManager &dm = getDataManager();
	std::vector<std::string> path = getPath(ap);

	try
	{
		dm.getKeys(table, file, path);
		return true;
	} catch (...)
	{
		return false;
	}
}

/*****************************************************************************/
bool entry_exist(const std::string &table, const std::string &file, ...)
{
	bool ret;
	va_list ap;

	va_start(ap, file);
	ret = __exist(table.c_str(), file.c_str(), ap);
	va_end(ap);

	return ret;
}
