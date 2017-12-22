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
int file_add(context_t * context, NetworkFrame & p_rFrame)
{
	std::string l_FileName;
	p_rFrame.pop(l_FileName);

	void * l_FileData = nullptr;
	int_fast32_t l_FileLength = 0;

	p_rFrame.pop(l_FileData, l_FileLength);

	// Write data to disk
	std::string l_TmpFileName = l_FileName + APP_NAME + "tmp";
	std::string l_TmpFullName = std::string(base_directory) + std::string("/")
			+ l_TmpFileName;

	file_create_directory(l_TmpFullName);

	if (file_set_contents(l_TmpFileName.c_str(), l_FileData,
			l_FileLength) == RET_NOK)
	{
		werr(LOGDEV, "Error writing file %s with size %d",
				l_TmpFullName.c_str(), l_FileLength);
		free(l_FileData);
		return -1;
	}

	free(l_FileData);

	std::string l_FullName = std::string(base_directory) + std::string("/")
			+ l_FileName;

	rename(l_TmpFullName.c_str(), l_FullName.c_str());

	wlog(LOGDEBUG, "write file %s", l_FullName.c_str());

	// Update the entry DB
	entry_remove(l_FileName.c_str());
	// Update the image DB
	image_DB_remove(l_FileName.c_str());
	// Update options if needed
	option_read_client_conf();
	// Make sure the new file is drawn (if needed)
	screen_compose();

	return 0;
}

/*********************************************************************************
 Remove character file to be sure they are always downloaded at start-up time
 **********************************************************************************/
void file_clean(context_t * context)
{
	file_delete(CHARACTER_TABLE, context->id);
}

/***************************************************
 Request a file from network
 ****************************************************/
void file_request_from_network(context_t * p_pCtx, const char * p_pTable,
		const char * p_pFilename)
{
	char * l_pTablePath;

	l_pTablePath = strconcat(p_pTable, "/", p_pFilename, nullptr);
	file_lock(l_pTablePath);
	file_update(p_pCtx, l_pTablePath);
	file_unlock(l_pTablePath);
	free(l_pTablePath);

	return;
}

