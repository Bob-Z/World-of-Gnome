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

#include "log.h"
#include <bits/types/FILE.h>
#include <stdlib.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

/***************************************************************************
 Return a string representing the checksum of the file + a bool to true on success
 filename is the directory + name
 ***************************************************************************/
std::pair<bool, std::string> checksum_file(const std::string & file_name)
{
	FILE *fp = nullptr;

	unsigned long long checksum = 5381;

	std::pair<bool, std::string> result
	{ false, "0" };

	fp = fopen(file_name.c_str(), "r"); // read mode

	if (fp == nullptr)
	{
		return result;
	}

	int ch = 0;
	while ((ch = fgetc(fp)) != EOF)
	{
		checksum = checksum * 33 ^ ch;
	}

	fclose(fp);

	char text[128];
	snprintf(text, sizeof(text), "%llu", checksum);

	result.first = true;
	result.second = text;

	//wlog(LOGDEVELOPER, "Checksum for %s is %s", file_name.c_str(), result.second.c_str());

	return result;
}

/*********************
 Free an array of element ( returned by entry_read_list, get_group_list ...)
 The last element of the list must be nullptr
 *********************/
void deep_free(char ** to_delete)
{
	char ** current = to_delete;

	if (to_delete == nullptr)
	{
		return;
	}

	while (*current)
	{
		free(*current);
		current++;
	}

	if (to_delete)
	{
		free(to_delete);
	}
}

/*******************************************************************************
 Extract token from string
 Works just like strsep but delim is a full string, not separate characters
 ******************************************************************************/
char * _strsep(char **stringp, const char *delim)
{
	char * next_delim;
	char * start = *stringp;

	if (*stringp == nullptr)
	{
		return nullptr;
	}

	next_delim = strstr(*stringp, delim);
	if (next_delim)
	{
		*next_delim = 0;
		*stringp = next_delim + strlen(delim);
	}
	else
	{
		*stringp = nullptr;
	}
	return start;
}

/********************************
 Add two arrays
 Return pointer must free (not deep_free)
 ********************************/
char ** add_array(char ** array1, char ** array2)
{
	int array_index = 0;
	char ** current_array = nullptr;
	char ** ret_array = nullptr;

	if (array1 != nullptr)
	{
		current_array = array1;
		while (*current_array != nullptr)
		{
			array_index++;
			ret_array = (char **) realloc(ret_array, array_index * sizeof(char*));
			ret_array[array_index - 1] = *current_array;
			current_array++;
		}
	}

	if (array2 != nullptr)
	{
		current_array = array2;
		while (*current_array != nullptr)
		{
			array_index++;
			ret_array = (char **) realloc(ret_array, array_index * sizeof(char*));
			ret_array[array_index - 1] = *current_array;
			current_array++;
		}
	}

	// Terminal nullptr
	array_index++;
	ret_array = (char **) realloc(ret_array, array_index * sizeof(char*));
	ret_array[array_index - 1] = nullptr;

	return ret_array;
}

/********************************
 // TODO use only std::vector
 Convert a vector to an array
 Return pointer must be deep free
 ********************************/
char ** to_array(const std::vector<std::string> & parameterArray)
{
	int array_index = 0;
	char ** ret_array = nullptr;

	for (auto && parameter : parameterArray)
	{
		array_index++;
		ret_array = (char **) realloc(ret_array, array_index * sizeof(char*));
		ret_array[array_index - 1] = strdup(parameter.c_str());
	}

	// Terminal nullptr
	array_index++;
	ret_array = (char **) realloc(ret_array, array_index * sizeof(char*));
	ret_array[array_index - 1] = nullptr;

	return ret_array;
}
