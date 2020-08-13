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

/* This manage an image data base */

#include "imageDB.h"

#include "client_server.h"
#include "Context.h"
#include "file.h"
#include "list.h"
#include "LockGuard.h"
#include "log.h"
#include "mutex.h"
#include "reader.h"
#include "sdl.h"
#include "syntax.h"
#include <string>

static Lock imageDbLock;

static std::map<std::string, SiAnim*> imageArray;

static SiAnim * def_anim = sdl_get_minimal_anim();

/*****************************************************************************/
static SiAnim * image_load(const std::string & fileName)
{
	const std::string filePath = base_directory + "/" + fileName;

	SiAnim * anim = anim_load(filePath);

	return anim;
}

/*****************************************************************************/
SiAnim * imageDB_get_anim(Context * context, const std::string & imageName)
{
	SiAnim * anim = nullptr;

	const std::string fileName = std::string(IMAGE_TABLE) + "/" + std::string(imageName);

	LockGuard guard(imageDbLock);

	// Search for a previously loaded anim
	auto iter = imageArray.find(fileName);
	if (iter != imageArray.end())
	{
		return iter->second;
	}

	// Try to load from a file
	file_lock(fileName.c_str());

	anim = image_load(fileName);

	if (anim != nullptr)
	{
		imageArray[fileName] = anim;
		file_unlock(fileName.c_str());
		return anim;
	}

	// Request an update to the server
	file_update(context->getConnection(), fileName.c_str());

	file_unlock(fileName.c_str());

	return def_anim;
}

/*****************************************************************************/
std::vector<SiAnim*> imageDB_get_anim_array(Context * context, const std::vector<std::string> & imageNameArray)
{
	std::vector<SiAnim*> animArray;

	for (auto && imageName : imageNameArray)
	{
		animArray.push_back(imageDB_get_anim(context, imageName));
	}

	return animArray;
}

/*****************************************************************************/
void image_DB_remove(const std::string & fileName)
{
	LOG("Image remove: " + fileName);

	// Clean-up old anim if any
	LockGuard guard(imageDbLock);

	auto iter = imageArray.find(fileName);
	if (iter != imageArray.end())
	{
		imageArray.erase(iter);
	}
}
