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
#include "character.h"
#include "common.h"
#include "map_server.h"
#include "network_server.h"
#include "npc.h"
#include <dirent.h>
#include <string.h>

/*********************************************
 Send playable character templates
 *********************************************/
void character_playable_send_list(context_t * context)
{
	char * marquee;
	DIR * dir;
	char * dirname;
	struct dirent * ent;

	// Read all files in character template directory
	dirname = strconcat(base_directory, "/", CHARACTER_TEMPLATE_TABLE, nullptr);

	dir = opendir(dirname);
	if (dir == nullptr)
	{
		return;
	}
	free(dirname);

	std::vector<std::string> l_Array;

	while ((ent = readdir(dir)) != nullptr)
	{
		// skip hidden file
		if (ent->d_name[0] == '.')
		{
			continue;
		}

		if (entry_read_string(CHARACTER_TEMPLATE_TABLE, ent->d_name, &marquee,
		CHARACTER_KEY_MARQUEE, nullptr) == RET_OK)
		{
			if (marquee[0] == '\0')
			{
				free(marquee);
				continue;
			}
			free(marquee);
		}
		else
		{
			char ** marquee_list = nullptr;
			if (entry_read_list(CHARACTER_TEMPLATE_TABLE, ent->d_name,
					&marquee_list, CHARACTER_KEY_MARQUEE, nullptr) == RET_NOK)
			{
				wlog(LOGDESIGNER, "%s has no marquee", ent->d_name);
				continue;
			}
			if (marquee_list[0][0] == '\0')
			{
				deep_free(marquee_list);
				continue;
			}
			deep_free(marquee_list);
		}

		// add file name to network frame
		l_Array.push_back(std::string(ent->d_name));
	}

	closedir(dir);

	NetworkFrame l_Frame;
	l_Frame.push(l_Array);

	network_send_command(context, CMD_SEND_PLAYABLE_CHARACTER, l_Frame, false);
}

/*********************************************
 *********************************************/
void character_user_send(context_t * p_pCtx, const char * p_pCharacterId)
{
	char * l_pType = nullptr;
	char * l_pName = nullptr;

	if (entry_read_string(CHARACTER_TABLE, p_pCharacterId, &l_pType,
	CHARACTER_KEY_TYPE, nullptr) == RET_NOK)
	{
		return;
	}

	if (entry_read_string(CHARACTER_TABLE, p_pCharacterId, &l_pName,
	CHARACTER_KEY_NAME, nullptr) == RET_NOK)
	{
		free(l_pType);
		return;
	}

	network_send_user_character(p_pCtx, p_pCharacterId, l_pType, l_pName);

	free(l_pType);
	free(l_pName);
}

/*********************************************
 *********************************************/
void character_user_send_list(context_t * context)
{
	char ** l_pCharacterList = nullptr;

	if (entry_read_list(USERS_TABLE, context->user_name, &l_pCharacterList,
	USERS_CHARACTER_LIST, nullptr) == RET_NOK)
	{
		return;
	}

	int l_Index = 0;

	while (l_pCharacterList[l_Index] != nullptr)
	{
		character_user_send(context, l_pCharacterList[l_Index]);
		l_Index++;
	}

	deep_free(l_pCharacterList);
}

/*****************************
 Disconnect a character.
 This kill a NPC AI thread
 return -1 if fails
 *****************************/
int character_disconnect(const char * id)
{
	context_t * ctx;

	werr(LOGDEVELOPER, "Disconnecting %s", id);

	ctx = context_find(id);
	context_set_in_game(ctx, false);
	context_set_connected(ctx, false);
	context_spread(ctx);

	if (context_is_npc(ctx) == true)
	{
		/* Wake up NPC */
		if (SDL_TryLockMutex(ctx->cond_mutex) == 0)
		{
			SDL_CondSignal(ctx->cond);
			SDL_UnlockMutex(ctx->cond_mutex);
		}
	}

	return 0;
}
/*****************************
 Kick a character out of the game
 It does not disconnect it.
 An NPC could re pop from an out of game state.
 A player can go back in game after choosing a new character id
 return -1 if fails
 *****************************/
int character_out_of_game(const char * id)
{
	context_t * ctx;

	werr(LOGDEVELOPER, "Kicking %s out of the game", id);

	ctx = context_find(id);
	context_set_in_game(ctx, false);
	context_spread(ctx);

	if (context_is_npc(ctx) == true)
	{
		/* Wake up NPC */
		if (SDL_TryLockMutex(ctx->cond_mutex) == 0)
		{
			SDL_CondSignal(ctx->cond);
			SDL_UnlockMutex(ctx->cond_mutex);
		}
	}

	return 0;
}

/******************************************************
 Create a new character based on the specified template
 return the id of the newly created character
 the returned string MUST BE FREED by caller
 return nullptr if fails
 *******************************************************/
char * character_create_from_template(context_t * ctx, const char * my_template,
		const char * map, int layer, int x, int y)
{
	char * new_id;

	new_id = file_new(CHARACTER_TABLE, nullptr);
	if (file_copy(CHARACTER_TEMPLATE_TABLE, my_template, CHARACTER_TABLE,
			new_id) == false)
	{
		file_delete(CHARACTER_TABLE, new_id);
		return nullptr;
	}

	// Check if new character is allowed to be created here
	if (map_check_tile(ctx, new_id, map, layer, x, y) == 0)
	{
		entry_destroy(CHARACTER_TABLE, new_id);
		file_delete(CHARACTER_TABLE, new_id);
		free(new_id);
		return nullptr;
	}

	if (entry_write_string(CHARACTER_TABLE, new_id, map, CHARACTER_KEY_MAP,
			nullptr) == RET_NOK)
	{
		entry_destroy(CHARACTER_TABLE, new_id);
		file_delete(CHARACTER_TABLE, new_id);
		free(new_id);
		return nullptr;
	}

	if (entry_write_int(CHARACTER_TABLE, new_id, x, CHARACTER_KEY_TILE_X,
			nullptr) == RET_NOK)
	{
		entry_destroy(CHARACTER_TABLE, new_id);
		file_delete(CHARACTER_TABLE, new_id);
		free(new_id);
		return nullptr;
	}

	if (entry_write_int(CHARACTER_TABLE, new_id, y, CHARACTER_KEY_TILE_Y,
			nullptr) == RET_NOK)
	{
		entry_destroy(CHARACTER_TABLE, new_id);
		file_delete(CHARACTER_TABLE, new_id);
		free(new_id);
		return nullptr;
	}

	if (layer != -1) // FIXME
	{
		if (entry_write_int(CHARACTER_TABLE, new_id, layer, CHARACTER_LAYER,
				nullptr) == RET_NOK)
		{
			entry_destroy(CHARACTER_TABLE, new_id);
			file_delete(CHARACTER_TABLE, new_id);
			free(new_id);
			return nullptr;
		}
	}

	return new_id;
}

/***********************************************************************
 ***********************************************************************/
static void execute_aggro(context_t * agressor, context_t * target,
		char * script, int aggro_dist)
{
	int dist;
	const char * param[] =
	{ nullptr, nullptr, nullptr };

	dist = context_distance(agressor, target);

	if (dist <= aggro_dist)
	{
		param[1] = "1";
	}
	else
	{
		param[1] = "0";
	}

	param[0] = target->id;
	param[2] = nullptr;
	action_execute_script(agressor, script, param);

}
/***********************************************************************
 Call aggro script for each context in every npc context aggro dist
 ***********************************************************************/
void character_update_aggro(context_t * agressor)
{
	context_t * target = nullptr;
	context_t * npc = nullptr;
	int aggro_dist;
	char * aggro_script;

	if (agressor == nullptr)
	{
		return;
	}

	if (agressor->map == nullptr)
	{
		return;
	}

	if (agressor->id == nullptr)
	{
		return;
	}

	/* If the current context is an NPC it might be an aggressor: compute its aggro */
	if (character_get_npc(agressor->id) && agressor->luaVM != nullptr)
	{
		if (entry_read_int(CHARACTER_TABLE, agressor->id, &aggro_dist,
		CHARACTER_KEY_AGGRO_DIST, nullptr) == RET_OK)
		{
			if (entry_read_string(CHARACTER_TABLE, agressor->id, &aggro_script,
			CHARACTER_KEY_AGGRO_SCRIPT, nullptr) == RET_OK)
			{
				target = context_get_first();

				while (target != nullptr)
				{
					/* Skip current context */
					if (agressor == target)
					{
						target = target->next;
						continue;
					}
					if (target->id == nullptr)
					{
						target = target->next;
						continue;
					}
					if (target->map == nullptr)
					{
						target = target->next;
						continue;
					}
					/* Skip if not on the same map */
					if (strcmp(agressor->map, target->map) != 0)
					{
						target = target->next;
						continue;
					}
					execute_aggro(agressor, target, aggro_script, aggro_dist);
					target = target->next;
				}
				free(aggro_script);
			}
		}
	}

	/* Compute aggro of all other NPC to the current context */
	target = agressor;
	npc = context_get_first();

	while (npc != nullptr)
	{
		/* Skip current context */
		if (target == npc)
		{
			npc = npc->next;
			continue;
		}
		if (npc->id == nullptr)
		{
			npc = npc->next;
			continue;
		}
		if (npc->map == nullptr)
		{
			npc = npc->next;
			continue;
		}
		if (npc->luaVM == nullptr)
		{
			npc = npc->next;
			continue;
		}
		/* Skip if not on the same map */
		if (strcmp(npc->map, target->map) != 0)
		{
			npc = npc->next;
			continue;
		}
		if (entry_read_int(CHARACTER_TABLE, npc->id, &aggro_dist,
		CHARACTER_KEY_AGGRO_DIST, nullptr) == RET_NOK)
		{
			npc = npc->next;
			continue;
		}
		if (entry_read_string(CHARACTER_TABLE, npc->id, &aggro_script,
		CHARACTER_KEY_AGGRO_SCRIPT, nullptr) == RET_NOK)
		{
			npc = npc->next;
			continue;
		}

		execute_aggro(npc, target, aggro_script, aggro_dist);

		free(aggro_script);

		npc = npc->next;
	}
}
/*************************************************************
 *************************************************************/
static void do_set_pos(context_t * ctx, const char * map, int x, int y,
		bool change_map)
{
	context_set_map(ctx, map);
	context_set_pos_tx(ctx, x);
	context_set_pos_ty(ctx, y);

	entry_write_string(CHARACTER_TABLE, ctx->id, map, CHARACTER_KEY_MAP,
			nullptr);
	entry_write_int(CHARACTER_TABLE, ctx->id, x, CHARACTER_KEY_TILE_X, nullptr);
	entry_write_int(CHARACTER_TABLE, ctx->id, y, CHARACTER_KEY_TILE_Y, nullptr);

	context_spread(ctx);
	if (change_map == true)
	{
		context_request_other_context(ctx);
	}
}

/*************************************************************
 Move every context on the same coordinate as platform context
 *************************************************************/
static void platform_move(context_t * platform, const char * map, int x, int y,
		bool change_map)
{
	context_t * current = context_get_first();
	int is_platform;

	if (entry_read_int(CHARACTER_TABLE, platform->id, &is_platform,
	CHARACTER_KEY_PLATFORM, nullptr) == RET_NOK)
	{
		return;
	}

	if (!is_platform)
	{
		return;
	}

	while (current)
	{
		if (current == platform)
		{
			current = current->next;
			continue;
		}
		if (platform->tile_x == current->tile_x
				&& platform->tile_y == current->tile_y
				&& !strcmp(platform->map, current->map))
		{
			do_set_pos(current, map, x, y, change_map);
		}
		current = current->next;
	}
}

/******************************************************
 return 0 if new position OK or if position has not changed.
 return -1 if the position was not set (because tile not allowed or out of bound)
 ******************************************************/
int character_set_pos(context_t * ctx, const char * map, int x, int y)
{
	char ** event_id;
	char * script;
	char ** param = nullptr;
	int i;
	bool change_map = false;
	int width = x + 1;
	int height = y + 1;
	int warpx = 0;
	int warpy = 0;
	int ctx_layer = 0;
	char layer_name[SMALL_BUF];
	char buf[SMALL_BUF];
	char * coord[3];
	int ret_value;
	int layer;

	if (ctx == nullptr)
	{
		return -1;
	}

	// Do nothing if no move
	if (!strcmp(ctx->map, map) && ctx->tile_x == x && ctx->tile_y == y)
	{
		return 0;
	}

	ctx_layer = 0;
	entry_read_int(CHARACTER_TABLE, ctx->id, &ctx_layer, CHARACTER_LAYER,
			nullptr);
	sprintf(layer_name, "%s%d", MAP_KEY_LAYER, ctx_layer);

	entry_read_int(MAP_TABLE, map, &width, MAP_KEY_WIDTH, nullptr);
	entry_read_int(MAP_TABLE, map, &height, MAP_KEY_HEIGHT, nullptr);
	entry_read_int(MAP_TABLE, map, &warpx, MAP_KEY_WARP_X, nullptr);
	entry_read_int(MAP_TABLE, map, &warpy, MAP_KEY_WARP_Y, nullptr);

	// Offscreen script
	entry_read_string(MAP_TABLE, map, &script, MAP_OFFSCREEN, nullptr);
	if (script != nullptr && (x < 0 || y < 0 || x >= width || y >= height))
	{
		snprintf(buf, SMALL_BUF, "%d", x);
		coord[0] = strdup(buf);
		snprintf(buf, SMALL_BUF, "%d", y);
		coord[1] = strdup(buf);
		coord[2] = nullptr;

		ret_value = action_execute_script(ctx, script, (const char **) coord);

		free(coord[0]);
		free(coord[1]);
		free(script);

		return ret_value;
	}
	if (script)
	{
		free(script);
	}

	// Coordinates warping
	if (x < 0)
	{
		if (warpy == 0)
		{
			return -1;
		}
		x = width - 1;
	}
	if (y < 0)
	{
		if (warpy == 0)
		{
			return -1;
		}
		y = height - 1;
	}
	if (x >= width)
	{
		if (warpx == 0)
		{
			return -1;
		}
		x = 0;
	}
	if (y >= height)
	{
		if (warpy == 0)
		{
			return -1;
		}
		y = 0;
	}

	// Check if this character is allowed to go to the target tile
	layer = ctx_layer;
	while (layer >= 0)
	{
		ret_value = map_check_tile(ctx, ctx->id, map, layer, x, y);
		/* not allowed */
		if (ret_value == 0)
		{
			return -1;
		}
		/* allowed */
		if (ret_value == 1)
		{
			break;
		}
		layer--;
	}

	if (layer < 0)
	{
		return -1;
	}

	if (strcmp(ctx->map, map))
	{
		change_map = true;
	}

	/* If this character is a platform, move all characters on it */
	platform_move(ctx, map, x, y, change_map);

	do_set_pos(ctx, map, x, y, change_map);

	event_id = map_get_event(map, ctx_layer, x, y);

	if (event_id)
	{
		i = 0;
		while (event_id[i])
		{
			script = nullptr;
			if (entry_read_string(MAP_TABLE, map, &script, layer_name,
			MAP_ENTRY_EVENT_LIST, event_id[i], MAP_EVENT_SCRIPT,
					nullptr) == RET_OK)
			{
				entry_read_list(MAP_TABLE, map, &param, layer_name,
				MAP_ENTRY_EVENT_LIST, event_id[i], MAP_EVENT_PARAM, nullptr);
			}
			else if (entry_read_string(MAP_TABLE, map, &script,
			MAP_ENTRY_EVENT_LIST, event_id[i], MAP_EVENT_SCRIPT,
					nullptr) == RET_OK)
			{
				entry_read_list(MAP_TABLE, map, &param, MAP_ENTRY_EVENT_LIST,
						event_id[i], MAP_EVENT_PARAM, nullptr);
			}

			if (script == nullptr)
			{
				i++;
				continue;
			}

			action_execute_script(ctx, script, (const char **) param);

			free(script);
			deep_free(param);
			param = nullptr;

			i++;
		}
		deep_free(event_id);
	}

	character_update_aggro(ctx);
	return 0;
}

/*********************************************************
 Set NPC to the value passed.
 If the value is != 0 , the NPC is instanciated
 return -1 on error
 *********************************************************/
int character_set_npc(const char * id, int npc)
{
	if (entry_write_int(CHARACTER_TABLE, id, npc, CHARACTER_KEY_NPC,
			nullptr) == RET_NOK)
	{
		return -1;
	}

	if (npc)
	{
		instantiate_npc(id);
	}

	return 0;
}

/*********************************************************
 Get NPC value.
 return 0 if not NPC
 return 1 if NPC
 *********************************************************/
int character_get_npc(const char * id)
{
	int npc;

	if (entry_read_int(CHARACTER_TABLE, id, &npc, CHARACTER_KEY_NPC,
			nullptr) == RET_NOK)
	{
		return 0;
	}

	return npc;
}

/*********************************************************
 return -1 if error
 *********************************************************/
int character_set_portrait(const char * id, const char * portrait)
{
	context_t * ctx;

	if (entry_write_string(CHARACTER_TABLE, id, portrait,
	CHARACTER_KEY_PORTRAIT, nullptr) == RET_NOK)
	{
		return -1;
	}

	ctx = context_find(id);
	network_send_character_file(ctx);

	return RET_OK;
}

/*********************************************************
 Get character portrait.
 must be freed
 *********************************************************/
char * character_get_portrait(const char * id)
{
	char * portrait;

	if (entry_read_string(CHARACTER_TABLE, id, &portrait,
	CHARACTER_KEY_PORTRAIT, nullptr) == RET_NOK)
	{
		return nullptr;
	}

	return portrait;
}

/*********************************************************
 Set AI script name
 return -1 on error
 *********************************************************/
int character_set_ai_script(const char * id, const char * script_name)
{
	if (entry_write_string(CHARACTER_TABLE, id, script_name, CHARACTER_KEY_AI,
			nullptr) == RET_NOK)
	{
		return -1;
	}

	return 0;
}

/*********************************************************
 Wake-up NPC. Execute it's AI script immediatly
 return -1 on error
 *********************************************************/
int character_wake_up(const char * id)
{
	context_t * ctx;

	ctx = context_find(id);

	/* Wake up NPC */
	ctx->next_execution_time = 0;
	if (SDL_TryLockMutex(ctx->cond_mutex) == 0)
	{
		SDL_CondSignal(ctx->cond);
		SDL_UnlockMutex(ctx->cond_mutex);
	}

	return 0;
}

/***********************************
 Write a new filename in a character's sprite list
 return RET_NOK on error
 ***********************************/
int character_set_sprite(const char * id, int index, const char * filename)
{
	if (id == nullptr || filename == nullptr)
	{
		return RET_NOK;
	}

	if (entry_write_list_index(CHARACTER_TABLE, id, filename, index,
	CHARACTER_KEY_SPRITE, nullptr) == RET_NOK)
	{
		return RET_NOK;
	}

	return RET_OK;
}

/***********************************
 Write a new filename in a character's sprite direction list
 return RET_NOK on error
 ***********************************/
int character_set_sprite_dir(const char * id, const char * dir, int index,
		const char * filename)
{
	const char * key;

	if (id == nullptr || filename == nullptr || dir == nullptr)
	{
		return RET_NOK;
	}

	switch (dir[0])
	{
	case 'N':
	case 'n':
		key = CHARACTER_KEY_DIR_N_SPRITE;
		break;
	case 'S':
	case 's':
		key = CHARACTER_KEY_DIR_S_SPRITE;
		break;
	case 'W':
	case 'w':
		key = CHARACTER_KEY_DIR_W_SPRITE;
		break;
	case 'E':
	case 'e':
		key = CHARACTER_KEY_DIR_E_SPRITE;
		break;
	default:
		return RET_NOK;
		break;
	}

	if (entry_write_list_index(CHARACTER_TABLE, id, filename, index, key,
			nullptr) == RET_NOK)
	{
		return RET_NOK;
	}

	return RET_OK;
}

/***********************************
 Write a new filename in a character's moving sprite list
 return RET_NOK on error
 ***********************************/
int character_set_sprite_move(const char * id, const char * dir, int index,
		const char * filename)
{
	const char * key;

	if (id == nullptr || filename == nullptr || dir == nullptr)
	{
		return RET_NOK;
	}

	switch (dir[0])
	{
	case 'N':
	case 'n':
		key = CHARACTER_KEY_MOV_N_SPRITE;
		break;
	case 'S':
	case 's':
		key = CHARACTER_KEY_MOV_S_SPRITE;
		break;
	case 'W':
	case 'w':
		key = CHARACTER_KEY_MOV_W_SPRITE;
		break;
	case 'E':
	case 'e':
		key = CHARACTER_KEY_MOV_E_SPRITE;
		break;
	default:
		return RET_NOK;
		break;
	}

	if (entry_write_list_index(CHARACTER_TABLE, id, filename, index, key,
			nullptr) == RET_NOK)
	{
		return RET_NOK;
	}

	return RET_OK;
}

/***********************************
 Broadcast character file to other context
 ***********************************/
void character_broadcast(const char * character)
{
	context_broadcast_character(character);
}

