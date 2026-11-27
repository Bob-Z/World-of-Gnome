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
#include "Context.h"
#include "entry.h"
#include "LockGuard.h"
#include "log.h"
#include "mutex.h"
#include "npc.h"
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
static int npc_script(void *data)
{
	Context *context = (Context*) data;
	Uint32 timeOutMs = 0U;
	std::string script;
	std::vector<std::string> parameters;

	// Do not start every NPC at the same moment
	usleep((random() % NPC_TIMEOUT) * 1000);

	LOG_DESIGN("Start AI script for " + std::string(context->getId()) + "(" + std::string(context->getCharacterName()) + ")");

	context->setInGame(true);

	while (context->isInGame() == true)
	{
		if (!script.empty())
		{
		}
		if (!parameters.empty())
		{
		}
		if (entry_read_string(CHARACTER_TABLE, context->getId(),
				script, { CHARACTER_KEY_AI }) == false)
		{
			ERR_USER("No AI script for " + std::string(context->getId()));
			break;
		}

		entry_read_list(CHARACTER_TABLE, context->getId(),
				parameters, { CHARACTER_KEY_AI_PARAMS });

		if (context->getNextExecutionTick() < SDL_GetTicks())
		{
			LockGuard guard(npc_lock);

			std::vector<std::string> params;
			if (!parameters.empty())
			{
				for (size_t i = 0; i < parameters.size(); i++)
				{
					params.push_back(std::string(parameters[i]));
				}
			}

			timeOutMs = action_execute_script(context, std::string(script),
					params);

			context->setNextExecutionTick(SDL_GetTicks() + timeOutMs);
		}

		/* The previous call to action_execute_script may have changed
		 the connected status. So we test it to avoid waiting for the
		 timeout duration before disconnecting */
		if (context->isInGame() == true)
		{
			context->sleep(timeOutMs);
		}
	}

	LOG_DESIGN("End AI script for " + std::string(context->getId()) + "(" + std::string(context->getCharacterName()) + ")");

	// Send connected  = FALSE to other context
	context_spread(context);

	context_free(context);

	return 0;
}

/***************************
 ***************************/
void instantiate_npc(const std::string &id)
{
	int is_npc = 0;
	if (entry_read_int(CHARACTER_TABLE, id, &is_npc, { CHARACTER_KEY_NPC }) == false)
	{
		return;
	}

	if (is_npc == false)
	{
		return;
	}

	int x = 0;
	if (entry_read_int(CHARACTER_TABLE, id, &x, { CHARACTER_KEY_TILE_X }) == false)
	{
		return;
	}

	int y = 0;
	if (entry_read_int(CHARACTER_TABLE, id, &y, { CHARACTER_KEY_TILE_Y }) == false)
	{
		return;
	}

	std::string map;
	if (entry_read_string(CHARACTER_TABLE, id, map, { CHARACTER_KEY_MAP }) == false)
	{
		return;
	}

	std::string name;
	entry_read_string(CHARACTER_TABLE, id, name, { CHARACTER_KEY_NAME });

	std::string type;
	if (entry_read_string(CHARACTER_TABLE, id, type, { CHARACTER_KEY_TYPE }) == false)
	{
		return;
	}

	LOG_DESIGN("Creating NPC " + std::string(name) + " of type " + std::string(type) + " in map " + std::string(map) + " at " + std::to_string(x) + "," + std::to_string(y));
	Context *ctx = context_new();

	ctx->setCharacterName(name);
	ctx->setInGame(true);

	ctx->setMap(std::string(map));

	ctx->setType(std::string(type));

	ctx->setTile(x, y);

	ctx->setId(id);

	context_spread(ctx);

	std::string threadName = "NPC: " + id;
	ctx->setNpcThread(
			SDL_CreateThread(npc_script, threadName.c_str(), (void*) ctx));
}
/**************************
 init non playing character
 ***************************/
void init_npc(void)
{
	DIR *dir;
	struct dirent *ent;

	// Read all files in NPC directory
	const std::string file_path = base_directory + "/"
			+ std::string(CHARACTER_TABLE);

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
