/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2014 carabobz@gmail.com

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

#include <common.h>
#include <stdio.h>
#include <stdlib.h>

/***************************************************************************
Return a string representing the checksum of the file or NULL on error
filename is the directory + name
The returned string MUST be FREED
***************************************************************************/
char * checksum_file(const char * filename)
{
	FILE *fp;
	int ch;
	unsigned long long checksum = 5381;
	char text[128];

	fp = fopen(filename,"r"); // read mode

	if( fp == NULL ) {
		return NULL;
	}

	while ( ( ch = fgetc(fp) ) != EOF ) {
		checksum = checksum * 33 ^ ch;
	}

	fclose(fp);

	snprintf(text,128,"%llu",checksum);

	wlog(LOGDEBUG,"Checksum for %s is %s",filename,text);

	return strdup(text);
}

/***************************************************************************
the returned string MUST BE FREED
***************************************************************************/
char * strconcat(const char * str, ...)
{
	va_list ap;
	char * res = NULL;
	int size = 0;
	char * entry;

	res = strdup(str);
	size = strlen(res);

	va_start(ap, str);

	entry=va_arg(ap,char*);
	while( entry != NULL ) {
		size += strlen(entry);
		res = realloc(res,size+1);
		strcat(res,entry);

		entry=va_arg(ap,char*);
	}

	va_end(ap);

	return res;
}

/*********************
Free an array of element ( returned by entry_read_list, get_group_list ...)
The last element of the list must be NULL
*********************/
void deep_free(char ** to_delete)
{
	char ** current = to_delete;

	if( to_delete == NULL ) {
		return;
	}

	while(*current) {
		free(*current);
		current++;
	}

	if(to_delete) {
		free(to_delete);
	}
}

/*********************
Extract token from string
Works just like strsep but delim is a full string, not separate characters
*********************/
char * _strsep(char **stringp, const char *delim)
{
	char * next_delim;
	char * start = *stringp;

	if(*stringp == NULL) {
		return NULL;
	}

	next_delim = strstr(*stringp,delim);
	if(next_delim) {
		*next_delim = 0;
		*stringp = next_delim + strlen(delim);
	} else {
		*stringp = NULL;
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
	char ** current_array = NULL;
	char ** ret_array = NULL;

	if( array1 != NULL ) {
		current_array = array1;
		while( *current_array != NULL ) {
			array_index++;
			ret_array = realloc( ret_array, array_index * sizeof(char*) );
			ret_array[array_index-1] = *current_array;
			current_array++;
		}
	}

	if( array2 != NULL ) {
		current_array = array2;
		while( *current_array != NULL ) {
			array_index++;
			ret_array = realloc( ret_array, array_index * sizeof(char*) );
			ret_array[array_index-1] = *current_array;
			current_array++;
		}
	}

	/* Terminal NULL */
	array_index++;
	ret_array = realloc( ret_array, array_index * sizeof(char*) );
	ret_array[array_index-1] = NULL;

	return ret_array;
}
