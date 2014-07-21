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

/* Return a string representing the checksum of the file or NULL on error */
/* filename is the directory + name */
/* The returned string MUST be FREED */

char * checksum_file(const char * filename)
{
	FILE *fp;
	int ch;
	unsigned long int checksum = 0;
	char text[128];
	
	fp = fopen(filename,"r"); // read mode
 
	if( fp == NULL )
	{
		return NULL;
	}

	/* TODO: implement a safer crc calculation */
	while( ( ch = fgetc(fp) ) != EOF ) {
		checksum += ch;
	}

	fclose(fp);
	
	snprintf(text,128,"%ld",checksum);

	wlog(LOGDEBUG,"Checksum for %s is %s",filename,text);

	return strdup(text);
}
