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

#ifndef WOG_FILE_H
#define WOG_FILE_H

#include "Connection.h"
#include <SDL_stdinc.h>
#include <string>

class Context;

const std::string NO_SUGGESTED_NAME = "";

void file_lock(const char *filename);
void file_unlock(const char *filename);
void file_update(Connection *connection, const char *filename);
std::pair<bool, std::string> file_new(const std::string &table,
		const std::string &suggested_name = NO_SUGGESTED_NAME);
int file_get_contents(const char *filename, void **contents,
		int_fast32_t *length);
int file_set_contents(const char *filename, const void *contents, int length);
bool file_copy(const std::string &src_table, const std::string &src_name,
		const std::string &dst_table, const std::string &dst_name);
int file_create_directory(const std::string &p_rFullName);
int file_delete(const std::string &table, const std::string &filename);
Uint32 file_get_timestamp(const char *p_pTable, const char *p_pFilename);
void file_write(const std::string &filePath, const std::string &data);

#endif
