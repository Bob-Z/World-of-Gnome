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

#include "common.h"
#include <stdio.h>
#include <stdlib.h>

/***************************************************************************
 Return a string representing the checksum of the file or nullptr on error
 filename is the directory + name
 The returned string MUST be FREED
 ***************************************************************************/
char * checksum_file(const char * filename)
{
	FILE *fp = nullptr;
	int ch = 0;
	unsigned long long checksum = 5381;
	char text[128];

	fp = fopen(filename, "r"); // read mode

	if (fp == nullptr)
	{
		return nullptr;
	}

	while ((ch = fgetc(fp)) != EOF)
	{
		checksum = checksum * 33 ^ ch;
	}

	fclose(fp);

	snprintf(text, 128, "%llu", checksum);

	wlog(LOGDEVELOPER, "Checksum for %s is %s", filename, text);

	return strdup(text);
}

/***************************************************************************
 the returned string MUST BE FREED
 ***************************************************************************/
char * strconcat(const char * str, ...)
{
	va_list ap;
	char * res = nullptr;
	int size = 0;
	char * entry = nullptr;

	if (str != nullptr)
	{
		res = strdup(str);
		size = strlen(res);
	}

	va_start(ap, str);

	entry = va_arg(ap, char*);
	while (entry != nullptr)
	{
		if (res == nullptr)
		{
			res = static_cast<char*>(calloc(1, 1));
		}

		size += strlen(entry);
		res = (char *) realloc(res, size + 1);
		strcat(res, entry);

		entry = va_arg(ap, char*);
	}

	va_end(ap);

	return res;
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
			ret_array = (char **) realloc(ret_array,
					array_index * sizeof(char*));
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
			ret_array = (char **) realloc(ret_array,
					array_index * sizeof(char*));
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
char ** to_array(const std::vector<std::string> & p_rParam)
{
	int array_index = 0;
	char ** ret_array = nullptr;

	for (auto l_Param : p_rParam)
	{
		array_index++;
		ret_array = (char **) realloc(ret_array, array_index * sizeof(char*));
		ret_array[array_index - 1] = strdup(l_Param.c_str());
	}

	// Terminal nullptr
	array_index++;
	ret_array = (char **) realloc(ret_array, array_index * sizeof(char*));
	ret_array[array_index - 1] = nullptr;

	return ret_array;
}
