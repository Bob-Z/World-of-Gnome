/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2016-2020 carabobz@gmail.com

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

#include "client_server.h"
#include "file.h"
#include "log.h"
#include "syntax.h"
#include <map>
#include <SDL_mixer.h>
#include <SDL_rwops.h>
#include <string>

static std::map<std::string, Mix_Chunk*> g_SoundList;

/*****************************************************************************/
int sfx_play(Connection & connection, const std::string & fileName, int channel, int loops)
{
	auto l_It = g_SoundList.find(fileName);

	if (l_It != g_SoundList.end())
	{
		return Mix_PlayChannel(channel, l_It->second, loops);
	}

	const std::string tableFilename = SFX_TABLE + "/" + fileName;
	const std::string fullName = base_directory + "/" + tableFilename;

	file_lock(tableFilename.c_str());

	SDL_RWops * fileDesc = SDL_RWFromFile(fullName.c_str(), "r");
	if (fileDesc == nullptr)
	{
		std::string errorText = std::string("sfx_play: cannot open ") + fullName.c_str();
		werr(LOGDESIGNER, errorText.c_str());
		file_update(&connection, tableFilename.c_str());
		file_unlock(tableFilename.c_str());
		return -1;
	}

	Mix_Chunk * chunk = Mix_LoadWAV_RW(fileDesc, 1);
	if (chunk == nullptr)
	{
		std::string l_Err = std::string("sfx_play: cannot read ") + fullName.c_str();
		werr(LOGDESIGNER, l_Err.c_str());
		file_update(&connection, tableFilename.c_str());
		file_unlock(tableFilename.c_str());
		return -1;
	}

	g_SoundList[fileName] = chunk;

	file_unlock(tableFilename.c_str());

	return Mix_PlayChannel(channel, chunk, loops);
}

/*****************************************************************************/
void sfx_stop(int channel)
{
	Mix_HaltChannel(channel);
}

/*****************************************************************************/
void sfx_set_volume(int channel, int volumePerCent)
{
	Mix_Volume(channel, volumePerCent * MIX_MAX_VOLUME / 100);
}
