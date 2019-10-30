/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2019 carabobz@gmail.com

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
#include "common.h"
#include "Context.h"
#include "npc.h"
#include "log.h"
#include "util.h"
#include "entry.h"
#include "syntax.h"
#include "mutex.h"
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
	Uint32 timeout_ms;
	char * script = nullptr;
	char ** parameters = nullptr;

	/* Do not start every NPC at the same moment */
	usleep((random() % NPC_TIMEOUT) * 1000);

	context_new_VM(context);

	wlog(LOGDESIGNER, "Start AI script for %s(%s)", context->m_id, context->getCharacterName().c_str());

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
		if (entry_read_string(CHARACTER_TABLE, context->m_id, &script, CHARACTER_KEY_AI, nullptr) == RET_NOK)
		{
			werr(LOGUSER, "No AI script for %s", context->m_id);
			break;
		}
		entry_read_list(CHARACTER_TABLE, context->m_id, &parameters, CHARACTER_KEY_AI_PARAMS, nullptr);

		if (context->m_next_execution_time < SDL_GetTicks())
		{
			SDL_LockMutex(npc_mutex);
			timeout_ms = action_execute_script(context, script, (const char **) parameters);
			SDL_UnlockMutex(npc_mutex);
			context->m_next_execution_time = SDL_GetTicks() + timeout_ms;
		}

		/* The previous call to action_execute_script may have changed
		 the connected status. So we test it to avoid waiting for the
		 timeout duration before disconnecting */
		if (context->isConnected() == true)
		{
			SDL_LockMutex(context->m_condition_mutex);
			SDL_CondWaitTimeout(context->m_condition, context->m_condition_mutex, timeout_ms);
			SDL_UnlockMutex(context->m_condition_mutex);
		}
	}

	wlog(LOGDESIGNER, "End AI script for %s(%s)", context->m_id, context->getCharacterName().c_str());

	/* Send connected  = FALSE to other context */
	context_spread(context);

	/* clean up */
	context_free(context);

	return 0;
}

/***************************
 ***************************/
void instantiate_npc(const char * id)
{
	char * type;
	char * name;
	char * map;
	int is_npc;
	int x;
	int y;
	Context * ctx;
	char buf[512];

	// check if it's a NPC
	if (entry_read_int(CHARACTER_TABLE, id, &is_npc, CHARACTER_KEY_NPC, nullptr) == RET_NOK)
	{
		return;
	}

	if (!is_npc)
	{
		return;
	}

	// read data of this npc
	if (entry_read_int(CHARACTER_TABLE, id, &x, CHARACTER_KEY_TILE_X, nullptr) == RET_NOK)
	{
		return;
	}

	if (entry_read_int(CHARACTER_TABLE, id, &y, CHARACTER_KEY_TILE_Y, nullptr) == RET_NOK)
	{
		return;
	}

	if (entry_read_string(CHARACTER_TABLE, id, &map, CHARACTER_KEY_MAP, nullptr) == RET_NOK)
	{
		return;
	}

	if (entry_read_string(CHARACTER_TABLE, id, &name, CHARACTER_KEY_NAME, nullptr) == RET_NOK)
	{
		name = strdup("");
	}

	if (entry_read_string(CHARACTER_TABLE, id, &type, CHARACTER_KEY_TYPE, nullptr) == RET_NOK)
	{
		free(map);
		free(name);
		return;
	}

	wlog(LOGDESIGNER, "Creating npc %s of type %s in map %s at %d,%d", name, type, map, x, y);
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

	context_set_id(ctx, id);
	ctx->m_condition = SDL_CreateCond();
	ctx->m_condition_mutex = SDL_CreateMutex();

	context_spread(ctx);

	sprintf(buf, "npc:%s", id);
	SDL_CreateThread(npc_script, buf, (void*) ctx);
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
