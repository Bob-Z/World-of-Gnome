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

#include "../common/common.h"
#include <dirent.h>
#include "imageDB.h"
#include "screen.h"
#include "option_client.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

/*************************************
 return 0 on success
**************************************/
int file_add(context_t * context,char * data,Uint32 command_size)
{
	char * ptr = data;
	Uint32 filename_size;
	char * filename = nullptr;
	char * tmpfilename = nullptr;
	char * tmpfullname = nullptr;
	char * fullname = nullptr;
	ret_code_t res;

	// Get the data from the network frame
	// First 4 bytes are the size of the file name
	if( command_size < sizeof(Uint32) ) {
		werr(LOGDEV,"Invalid file received");
		return -1;
	}
	filename_size = *((Uint32 *)ptr);

	// Following bytes are the file name, relative to the application base directory ( $HOME/.config/wog/client/ )
	ptr += sizeof(Uint32);
	filename = (char*)malloc(filename_size);
	memcpy(filename,ptr,filename_size);
	wlog(LOGDEBUG,"Received file %s",filename);
	if( filename == nullptr ) {
		werr(LOGDEV,"Unable to allocate %d bytes for file name",filename_size);
		return -1;
	}

	// Next is a Uint32 representing the size of the file's data
	ptr += filename_size;
	Uint32 filedata_size = *((Uint32 *)ptr);

	// Finally are the data bytes
	ptr += sizeof(Uint32);

	// Write the data to disk
	tmpfilename = strconcat(filename,APP_NAME,"tmp",nullptr);
	tmpfullname = strconcat(base_directory,"/",tmpfilename,nullptr);

	file_create_directory(tmpfullname);

	res = file_set_contents(tmpfilename,ptr,filedata_size);
	if( res == RET_NOK ) {
		werr(LOGDEV,"Error writing file %s with size %d",tmpfullname, filedata_size);
		free(tmpfullname);
		return -1;
	}

	fullname = strconcat(base_directory,"/",filename,nullptr);

	rename(tmpfullname,fullname);

	free(tmpfilename);
	free(tmpfullname);

	wlog(LOGDEBUG,"write file %s",fullname);
	free(fullname);

	// Update the entry DB
	entry_remove(filename);
	// Update the image DB
	image_DB_remove(filename);
	// Update options if needed
	option_get();
	// Make sure the new file is drawn (if needed)
	screen_compose();

	free(filename);
	return 0;
}

/*********************************************************************************
 Remove character file to be sure they are always downloaded at start-up time
**********************************************************************************/
void file_clean(context_t * context)
{
	file_delete(CHARACTER_TABLE,context->id);
}

/***************************************************
 Request a file from network
****************************************************/
void file_request_from_network(context_t * p_pCtx, const char * p_pTable, const char * p_pFilename)
{
        char * l_pTablePath;

        l_pTablePath = strconcat(p_pTable,"/",p_pFilename,nullptr);
        file_lock(l_pTablePath);
        file_update(p_pCtx, l_pTablePath);
        file_unlock(l_pTablePath);
        free(l_pTablePath);

        return;
}

