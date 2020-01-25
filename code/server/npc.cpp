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

#include "action.h"
#include "client_server.h"
#include "entry.h"
#include "log.h"
#include "mutex.h"
#include "npc.h"

#include "Context.h"
#include "syntax.h"
#include "util.h"
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define NPC_TIMEOUT	(2000)

/**********************************
 npc_script
 *********************************/
static int npc_script(void * data)
{
	Context * context = (Context *) data;
	Uint32 timeOutMs = 0U;
	char * script = nullptr;
	char ** parameters = nullptr;

	/* Do not start every NPC at the same moment */
	usleep((random() % NPC_TIMEOUT) * 1000);

	wlog(LOGDESIGNER, "Start AI script for %s(%s)", context->getId().c_str(), context->getCharacterName().c_str());

	while (context->isConnected() == true)
	{
		if (script)
		{
			free(script);
		}
		if (parameters)
		{
			deep_free(parameters);
		}
		if (entry_read_string(CHARACTER_TABLE, context->getId().c_str(), &script, CHARACTER_KEY_AI, nullptr) == false)
		{
			werr(LOGUSER, "No AI script for %s", context->getId().c_str());
			break;
		}
		entry_read_list(CHARACTER_TABLE, context->getId().c_str(), &parameters, CHARACTER_KEY_AI_PARAMS, nullptr);

		if (context->getNextExecutionTick() < SDL_GetTicks())
		{
			SDL_LockMutex(npc_mutex);
			timeOutMs = action_execute_script(context, script, (const char **) parameters);
			SDL_UnlockMutex(npc_mutex);
			context->setNextExecutionTick(SDL_GetTicks() + timeOutMs);
		}

		/* The previous call to action_execute_script may have changed
		 the connected status. So we test it to avoid waiting for the
		 timeout duration before disconnecting */
		if (context->isConnected() == true)
		{
			context->sleep(timeOutMs);
		}
	}

	wlog(LOGDESIGNER, "End AI script for %s(%s)", context->getId().c_str(), context->getCharacterName().c_str());

	// Send connected  = FALSE to other context
	context_spread(context);

	context_free(context);

	return 0;
}

/***************************
 ***************************/
void instantiate_npc(const std::string & id)
{
	char * type = nullptr;
	char * name = nullptr;
	char * map = nullptr;
	int is_npc = 0;
	int x = 0;
	int y = 0;
	Context * ctx = nullptr;

	// check if it's a NPC
	if (entry_read_int(CHARACTER_TABLE, id.c_str(), &is_npc, CHARACTER_KEY_NPC, nullptr) == false)
	{
		return;
	}

	if (!is_npc)
	{
		return;
	}

	// read data of this npc
	if (entry_read_int(CHARACTER_TABLE, id.c_str(), &x, CHARACTER_KEY_TILE_X, nullptr) == false)
	{
		return;
	}

	if (entry_read_int(CHARACTER_TABLE, id.c_str(), &y, CHARACTER_KEY_TILE_Y, nullptr) == false)
	{
		return;
	}

	if (entry_read_string(CHARACTER_TABLE, id.c_str(), &map, CHARACTER_KEY_MAP, nullptr) == false)
	{
		return;
	}

	if (entry_read_string(CHARACTER_TABLE, id.c_str(), &name, CHARACTER_KEY_NAME, nullptr) == false)
	{
		name = strdup("");
	}

	if (entry_read_string(CHARACTER_TABLE, id.c_str(), &type, CHARACTER_KEY_TYPE, nullptr) == false)
	{
		free(map);
		free(name);
		return;
	}

	wlog(LOGDESIGNER, "Creating NPC %s of type %s in map %s at %d,%d", name, type, map, x, y);
	ctx = context_new();
	ctx->setUserName("CPU");
	ctx->setCharacterName(name);
	free(name);
	ctx->setInGame(true);
	ctx->setConnected(true);

	ctx->setMap(std::string(map));
	free(map);

	ctx->setType(std::string(type));
	free(type);

	ctx->setTileX(x);
	ctx->setTileY(y);

	ctx->setId(id);

	context_spread(ctx);

	std::string threadName = "NPC: " + id;
	SDL_CreateThread(npc_script, threadName.c_str(), (void*) ctx);
}
/**************************
 init non playing character
 ***************************/
void init_npc(void)
{
	DIR * dir;
	struct dirent * ent;

	// Read all files in NPC directory
	const std::string file_path = base_directory + "/" + std::string(CHARACTER_TABLE);

	dir = opendir(file_path.c_str());
	if (dir == nullptr)
	{
		return;
	}

	while ((ent = readdir(dir)) != nullptr)
	{
		// skip hidden file
		if (ent->d_name[0] == '.')
		{
			continue;
		}

		instantiate_npc(ent->d_name);
	}
	closedir(dir);
}
