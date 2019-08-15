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

#include <context.h>
#include <network.h>
#include <stdlib.h>
#include <util.h>
#include <cstring>

#include "network_server.h"

/**************************************
 Spread the data of a context to all in_game context
 **************************************/
void context_spread(context_t * context)
{
	context_t * ctx = nullptr;

	context_lock_list();

	ctx = context_get_list_start();

	if (ctx == nullptr)
	{
		context_unlock_list();
		return;
	}

	do
	{
		// Skip if not in game
		if (ctx->in_game == false)
		{
			continue;
		}

		if (context_is_npc(ctx) == true)
		{
			continue;
		}

		if (ctx->id == nullptr)
		{
			continue;
		}

		// Skip if not on the same map or previous map
		if ((ctx->map != nullptr) && (context->map != nullptr)
				&& (context->prev_map != nullptr))
		{
			if (strcmp(context->map, ctx->map) != 0
					&& strcmp(context->prev_map, ctx->map) != 0)
			{
				continue;
			}
		}

		network_send_context_to_context(ctx, context);

	} while ((ctx = ctx->next) != nullptr);

	/* The existing context on the previous map should have
	 been deleted, we don't need this any more -> this will
	 generate less network request */
	free(context->prev_map);
	context->prev_map = nullptr;

	context_unlock_list();
}

/**************************************
 if "map" == nullptr : server sends a message to all connected client
 if "map" != nullptr : server sends a message to all connected clients on the map
 **************************************/
void context_broadcast_text(const char * map, const char * text)
{
	context_t * ctx = nullptr;

	context_lock_list();

	ctx = context_get_list_start();

	if (ctx == nullptr)
	{
		context_unlock_list();
		return;
	}

	do
	{
		// Skip if not in game
		if (ctx->in_game == false)
		{
			continue;
		}

		if (context_is_npc(ctx) == true)
		{
			continue;
		}

		// Skip if the player has not selected its character
		if (ctx->id == nullptr)
		{
			continue;
		}

		if (ctx->map == nullptr)
		{
			continue;
		}

		if (map)
		{
			// Skip if not on the same map
			if (strcmp(map, ctx->map) != 0)
			{
				continue;
			}
		}

		network_send_text(ctx->id, text);

	} while ((ctx = ctx->next) != nullptr);

	context_unlock_list();
}

/**************************************
 Broadcast upload of a map file to all in_game context on that map
 **************************************/
void context_broadcast_map(const char * map)
{
	context_t * ctx = nullptr;
	char * filename;

	context_lock_list();

	ctx = context_get_list_start();

	if (ctx == nullptr)
	{
		context_unlock_list();
		return;
	}

	filename = strconcat(MAP_TABLE, "/", map, nullptr);

	do
	{
		if (context_is_npc(ctx) == true)
		{
			continue;
		}

		// Skip if not in game
		if (ctx->in_game == false)
		{
			continue;
		}

		// Skip if not on the same map
		if (ctx->map)
		{
			if (strcmp(map, ctx->map) != 0)
			{
				continue;
			}
		}

		network_send_file(ctx, filename);

	} while ((ctx = ctx->next) != nullptr);

	free(filename);

	context_unlock_list();
}

/**************************************
 Broadcast upload of a character file to all in_game context on the same map
 **************************************/
void context_broadcast_character(const char * character)
{
	context_t * ctx = nullptr;
	context_t * character_ctx = nullptr;
	char * filename;

	context_lock_list();

	character_ctx = context_find(character);

	ctx = context_get_list_start();

	if (ctx == nullptr)
	{
		context_unlock_list();
		return;
	}

	filename = strconcat(CHARACTER_TABLE, "/", character, nullptr);

	do
	{
		if (context_is_npc(ctx) == true)
		{
			continue;
		}

		// Skip if not in game
		if (ctx->in_game == false)
		{
			continue;
		}

		// Skip if not on the same map
		if (ctx->map)
		{
			if (strcmp(character_ctx->map, ctx->map) != 0)
			{
				continue;
			}
		}

		network_send_file(ctx, filename);

	} while ((ctx = ctx->next) != nullptr);

	free(filename);

	context_unlock_list();
}

/**************************************
 Send the data of all existing context to the passed context
 Useful at start time
 **************************************/
void context_request_other_context(context_t * context)
{
	context_t * ctx = nullptr;

	context_lock_list();

	ctx = context_get_list_start();

	if (ctx == nullptr)
	{
		context_unlock_list();
		return;
	}

	do
	{
		// Skip the calling context
		if (context == ctx)
		{
			continue;
		}

		// Skip if not on the same map
		if (ctx->map != nullptr)
		{
			if (strcmp(context->map, ctx->map) != 0)
			{
				continue;
			}
		}

		network_send_context_to_context(context, ctx);

	} while ((ctx = ctx->next) != nullptr);

	context_unlock_list();
}
