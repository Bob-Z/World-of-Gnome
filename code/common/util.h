/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2017 carabobz@gmail.com

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

#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <string>
#include <vector>

char * checksum_file(const char * filename);
char * strconcat(const char * str, ...);
void deep_free(char ** to_delete);
char * _strsep(char **stringp, const char *delim);
void unserializeNetworkFrame(char * p_pNetworkFrame,
		std::vector<std::string> & p_rData);
char ** add_array(char ** array1, char ** array2);
char ** to_array(const std::vector<std::string> & p_rParam);
#endif
