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

#ifndef ENTRY_H
#define ENTRY_H

#include <string>
#include <vector>

void entry_remove(const std::string &filename);

int entry_read_int(const std::string &table, const std::string &file, int *res,
		const std::vector<std::string> &path);
bool entry_read_string(const std::string &table, const std::string &file,
		std::string &res, const std::vector<std::string> &path);
bool entry_read_list_index(const std::string &table, const std::string &file,
		std::string &res, int index, const std::vector<std::string> &path);
int entry_read_list(const std::string &table, const std::string &file,
		std::vector<std::string> &res, const std::vector<std::string> &path);

int entry_write_int(const std::string &table, const std::string &file, int data,
		const std::vector<std::string> &path);
int entry_write_string(const std::string &table, const std::string &file,
		const std::string &data, const std::vector<std::string> &path);
int entry_write_list_index(const std::string &table, const std::string &file,
		const std::string &data, int index, const std::vector<std::string> &path);
int entry_write_list(const char *table, const char *file, char **data,
		const std::vector<std::string> &path);

std::string entry_get_unused_list_entry(const char *table, const char *file,
		const std::vector<std::string> &path);
std::string entry_get_unused_group(const char *table, const char *file,
		const std::vector<std::string> &path);
int entry_get_group_list(const std::string &table, const std::string &file,
		std::vector<std::string> &res, const std::vector<std::string> &path);
int entry_add_to_list(const char *table, const char *file,
		const char *to_be_added, const std::vector<std::string> &path);
int entry_remove_group(const char *table, const char *file, const char *group,
		const std::vector<std::string> &path);

int entry_remove_from_list(const char *table, const char *file,
		const char *to_be_removed, const std::vector<std::string> &path);
int entry_list_create(const char *table, const char *file,
		const std::vector<std::string> &path);
int entry_group_create(const char *table, const char *file,
		const std::vector<std::string> &path);
std::string entry_copy_group(const char *src_table, const char *src_file,
		const char *dst_table, const char *dst_file, const char *group_name,
		const std::vector<std::string> &path);
int entry_update(const std::string &type, const std::string &table,
		const std::string &file, const std::string &path,
		const std::string &value);
int entry_destroy(const std::string &table, const std::string &file);
bool entry_exist(const std::string &table, const std::string &file,
		const std::vector<std::string> &path);
#endif
