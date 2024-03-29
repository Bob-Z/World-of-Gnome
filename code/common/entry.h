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

void entry_remove(const char *filename);

int entry_read_int(const std::string &table, const std::string &file, int *res,
		...);
int entry_read_string(const std::string &table, const std::string &file,
		char **res, ...);
int entry_read_list_index(const std::string &table, const std::string &file,
		char **res, int index, ...);
int entry_read_list(const std::string &table, const std::string &file,
		char ***res, ...);

int entry_write_int(const std::string &table, const std::string &file, int data,
		...);
int entry_write_string(const std::string &table, const std::string &file,
		const std::string &data, ...);
int entry_write_list_index(const std::string &table, const std::string &file,
		const std::string &data, int index, ...);
int entry_write_list(const char *table, const char *file, char **data, ...);

char* entry_get_unused_list_entry(const char *table, const char *file, ...);
char* entry_get_unused_group(const char *table, const char *file, ...);
int entry_get_group_list(const std::string &table, const std::string &file,
		char ***res, ...);
int entry_add_to_list(const char *table, const char *file,
		const char *to_be_added, ...);
int entry_remove_group(const char *table, const char *file, const char *group,
		...);

int entry_remove_from_list(const char *table, const char *file,
		const char *to_be_removed, ...);
int entry_list_create(const char *table, const char *file, ...);
int entry_group_create(const char *table, const char *file, ...);
char* entry_copy_group(const char *src_table, const char *src_file,
		const char *dst_table, const char *dst_file, const char *group_name,
		...);
int entry_update(const std::string &type, const std::string &table,
		const std::string &file, const std::string &path,
		const std::string &value);
int entry_destroy(const std::string &table, const std::string &file);
bool entry_exist(const std::string &table, const std::string &file, ...);
#endif
