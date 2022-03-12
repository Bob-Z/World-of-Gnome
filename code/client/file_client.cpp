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

#include "client_conf.h"
#include "client_server.h"
#include "const.h"
#include "Context.h"
#include "entry.h"
#include "file.h"
#include "FileReceivedObserver.h"
#include "imageDB.h"
#include "log.h"
#include "screen.h"
#include "syntax.h"
#include <cstdio>
#include <string>
#include <vector>

std::vector<FileReceivedObserver*> fileReceivedObserver;

/*****************************************************************************/
void file_add_observer(FileReceivedObserver *observer)
{
	fileReceivedObserver.push_back(observer);
}

/*************************************
 return 0 on success
 **************************************/
int file_add(const std::string &fileName, const std::string &data)
{
	const std::string tempFileName = fileName + APP_NAME + "tmp";
	const std::string tempPath = base_directory + "/" + tempFileName;

	file_create_directory(tempPath);

	if (file_set_contents(tempFileName.c_str(), data.c_str(),
			data.size()) == false)
	{
		werr(LOGDESIGNER, "Error writing file %s with size %d",
				tempFileName.c_str(), data.size());
		return -1;
	}

	const std::string file_path = base_directory + "/" + fileName;

	if (rename(tempPath.c_str(), file_path.c_str()) == -1)
	{
		LOG("Error renaming file " + tempPath + " to " + file_path);
		return false;
	}

	LOG("write file " + file_path);

	// Update the entry DB
	entry_remove(fileName.c_str());
	// Update the image DB
	image_DB_remove(fileName);
	// Make sure the new file is drawn (if needed)

	for (auto &observer : fileReceivedObserver)
	{
		observer->fileReceived(fileName);
	}

	screen_compose();

	return 0;
}

/******************************************************************************
 Remove character file to be sure they are always downloaded at start-up time
 *****************************************************************************/
void file_clean(Context *context)
{
	file_delete(CHARACTER_TABLE.c_str(), context->getId());
}

/*****************************************************************************/
void file_request_from_network(Connection &connection, const std::string &table,
		const std::string &file_name)
{
	const std::string tablePath = table + "/" + file_name;

	file_lock(tablePath.c_str());
	file_update(&connection, tablePath.c_str());
	file_unlock(tablePath.c_str());

	return;
}
