/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2016-2019 carabobz@gmail.com

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
#include <SDL2/SDL_mixer.h>
#include <map>

static std::map<std::string, Mix_Chunk*> g_SoundList;

/*******************************************************************************
 ******************************************************************************/
int sfx_play(context_t* p_Ctx, const std::string & p_FileName, int p_Channel,
		int p_Loops)
{
	auto l_It = g_SoundList.find(p_FileName);

	if (l_It != g_SoundList.end())
	{
		return Mix_PlayChannel(p_Channel, l_It->second, p_Loops);
	}

	const std::string l_TableFilename = std::string(SFX_TABLE)
			+ std::string("/") + p_FileName;
	const std::string l_FullName = std::string(base_directory) + "/"
			+ l_TableFilename;

	file_lock(l_TableFilename.c_str());

	SDL_RWops * l_pFileDesc = SDL_RWFromFile(l_FullName.c_str(), "r");
	if (l_pFileDesc == nullptr)
	{
		std::string l_Err = std::string("sfx_play: cannot open ")
				+ l_FullName.c_str();
		werr(LOGDESIGNER, l_Err.c_str());
		file_update(p_Ctx, l_TableFilename.c_str());
		file_unlock(l_TableFilename.c_str());
		return -1;
	}

	Mix_Chunk * l_pChunk = Mix_LoadWAV_RW(l_pFileDesc, 1);
	if (l_pChunk == nullptr)
	{
		std::string l_Err = std::string("sfx_play: cannot read ")
				+ l_FullName.c_str();
		werr(LOGDESIGNER, l_Err.c_str());
		file_update(p_Ctx, l_TableFilename.c_str());
		file_unlock(l_TableFilename.c_str());
		return -1;
	}

	g_SoundList[p_FileName] = l_pChunk;

	file_unlock(l_TableFilename.c_str());

	return Mix_PlayChannel(p_Channel, l_pChunk, p_Loops);
}

/*******************************************************************************
 ******************************************************************************/
void sfx_stop(int p_Channel)
{
	Mix_HaltChannel(p_Channel);
}

/*******************************************************************************
 ******************************************************************************/
void sfx_set_volume(int p_Channel, int p_VolumePerCent)
{
	Mix_Volume(p_Channel, p_VolumePerCent * MIX_MAX_VOLUME / 100);
}
