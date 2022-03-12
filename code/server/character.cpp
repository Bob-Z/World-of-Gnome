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

#include "character.h"

#include "action.h"
#include "client_server.h"
#include "const.h"
#include "context_server.h"
#include "Context.h"
#include "global.h"
#include "log.h"
#include "map_server.h"
#include "network_server.h"
#include "npc.h"
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <entry.h>
#include <file.h>
#include <stdlib.h>
#include <string>
#include <syntax.h>
#include <util.h>
#include <vector>

/*********************************************
 Send playable character templates
 *********************************************/
void character_playable_send_list(Connection &connection)
{
	char *marquee;
	DIR *dir;
	struct dirent *ent;

	// Read all files in character template directory
	const std::string file_path = base_directory + "/"
			+ std::string(CHARACTER_TEMPLATE_TABLE);

	dir = opendir(file_path.c_str());
	if (dir == nullptr)
	{
		return;
	}

	std::vector<std::string> array;

	while ((ent = readdir(dir)) != nullptr)
	{
		// skip hidden file
		if (ent->d_name[0] == '.')
		{
			continue;
		}

		if (entry_read_string(CHARACTER_TEMPLATE_TABLE, ent->d_name, &marquee,
				CHARACTER_KEY_MARQUEE, nullptr) == true)
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
			char **marquee_list = nullptr;
			if (entry_read_list(CHARACTER_TEMPLATE_TABLE, ent->d_name,
					&marquee_list, CHARACTER_KEY_MARQUEE, nullptr) == false)
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
		array.push_back(std::string(ent->d_name));
	}

	closedir(dir);

	network_send_playable_character(connection, array);
}

/*****************************************************************************/
void character_user_send(Connection &connection, const std::string &id)
{
	char *type = nullptr;
	char *name = nullptr;

	if (entry_read_string(CHARACTER_TABLE, id.c_str(), &type,
			CHARACTER_KEY_TYPE, nullptr) == false)
	{
		return;
	}

	if (entry_read_string(CHARACTER_TABLE, id.c_str(), &name,
			CHARACTER_KEY_NAME, nullptr) == false)
	{
		free(type);
		return;
	}

	network_send_user_character(connection, id, std::string(type),
			std::string(name));

	free(type);
	free(name);
}

/*****************************************************************************/
void character_user_send_list(Connection &connection)
{
	std::vector<std::string> characterList;

	try
	{
		characterList = getDataManager().get<std::vector<std::string>>(
				USERS_TABLE, connection.getUserName(),
				{ USERS_CHARACTER_LIST });
	} catch (...)
	{
		LOG_USER("No character available");
		return;
	}

	for (auto &name : characterList)
	{
		character_user_send(connection, name.c_str());
	}
}

/*****************************
 Disconnect a character.
 This kill a NPC AI thread
 return -1 if fails
 *****************************/
int character_disconnect(const char *id)
{
	Context *ctx;

	werr(LOGDEVELOPER, "Disconnecting %s", id);

	ctx = context_find(id);
	ctx->setInGame(false);
	ctx->getConnection()->setConnected(false);
	context_spread(ctx);

	if (ctx->isNpcActive())
	{
		ctx->wakeUp();
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
int character_out_of_game(const char *id)
{
	Context *ctx;

	werr(LOGDEVELOPER, "Kicking %s out of the game", id);

	ctx = context_find(id);
	ctx->setInGame(false);
	context_spread(ctx);

	if (ctx->isNpcActive())
	{
		ctx->wakeUp();
	}

	return 0;
}

/******************************************************
 Create a new character based on the specified template
 return the id of the newly created character
 the returned string MUST BE FREED by caller
 return nullptr if fails
 *******************************************************/
std::pair<bool, std::string> character_create_from_template(Context *ctx,
		const char *my_template, const char *map, int layer, int x, int y)
{
	const std::pair<bool, std::string> new_id = file_new(
			std::string(CHARACTER_TABLE));

	if (file_copy(CHARACTER_TEMPLATE_TABLE, my_template, CHARACTER_TABLE,
			new_id.second.c_str()) == false)
	{
		file_delete(CHARACTER_TABLE, new_id.second);
		return std::pair<bool, std::string>
		{ false, "" };
	}

	// Check if new character is allowed to be created here
	if (map_check_tile(ctx, new_id.second.c_str(), map, layer, x, y) == 0)
	{
		entry_destroy(CHARACTER_TABLE, new_id.second.c_str());
		file_delete(CHARACTER_TABLE, new_id.second);

		return std::pair<bool, std::string>
		{ false, "" };
	}

	if (entry_write_string(CHARACTER_TABLE, new_id.second.c_str(), map,
			CHARACTER_KEY_MAP, nullptr) == false)
	{
		entry_destroy(CHARACTER_TABLE, new_id.second.c_str());
		file_delete(CHARACTER_TABLE, new_id.second);
		return std::pair<bool, std::string>
		{ false, "" };
	}

	if (entry_write_int(CHARACTER_TABLE, new_id.second.c_str(), x,
			CHARACTER_KEY_TILE_X, nullptr) == false)
	{
		entry_destroy(CHARACTER_TABLE, new_id.second.c_str());
		file_delete(CHARACTER_TABLE, new_id.second);
		return std::pair<bool, std::string>
		{ false, "" };
	}

	if (entry_write_int(CHARACTER_TABLE, new_id.second.c_str(), y,
			CHARACTER_KEY_TILE_Y, nullptr) == false)
	{
		entry_destroy(CHARACTER_TABLE, new_id.second.c_str());
		file_delete(CHARACTER_TABLE, new_id.second);
		return std::pair<bool, std::string>
		{ false, "" };
	}

	if (layer != -1) // FIXME
	{
		if (entry_write_int(CHARACTER_TABLE, new_id.second.c_str(), layer,
				CHARACTER_LAYER, nullptr) == false)
		{
			entry_destroy(CHARACTER_TABLE, new_id.second.c_str());
			file_delete(CHARACTER_TABLE, new_id.second);
			return std::pair<bool, std::string>
			{ false, "" };
		}
	}

	return new_id;
}

/***********************************************************************
 ***********************************************************************/
static void execute_aggro(Context *agressor, const Context &target,
		char *script, int aggro_dist)
{
	int dist;
	const char *param[] =
	{ nullptr, nullptr, nullptr };

	dist = agressor->tileDistance(target);

	if (dist <= aggro_dist)
	{
		param[1] = "1";
	}
	else
	{
		param[1] = "0";
	}

	param[0] = target.getId().c_str();
	param[2] = nullptr;
	action_execute_script(agressor, script, param);

}
/***********************************************************************
 Call aggro script for each context in every NPC context aggro dist
 ***********************************************************************/
void character_update_aggro(Context *agressor)
{
	Context *target = nullptr;
	Context *npc = nullptr;
	int aggro_dist;
	char *aggro_script;

	if (agressor == nullptr)
	{
		return;
	}

	if (agressor->getMap() == "")
	{
		return;
	}

	if (agressor->getId() == "")
	{
		return;
	}

	// If the current context is an NPC it might be an aggressor: compute its aggro
	if (character_get_npc(agressor->getId()))
	{
		if (entry_read_int(CHARACTER_TABLE, agressor->getId().c_str(),
				&aggro_dist, CHARACTER_KEY_AGGRO_DIST, nullptr) == true)
		{
			if (entry_read_string(CHARACTER_TABLE, agressor->getId().c_str(),
					&aggro_script, CHARACTER_KEY_AGGRO_SCRIPT, nullptr) == true)
			{
				target = context_get_first();

				while (target != nullptr)
				{
					// Skip current context
					if (agressor == target)
					{
						target = target->m_next;
						continue;
					}
					if (target->getId() == "")
					{
						target = target->m_next;
						continue;
					}
					if (target->getMap() == "")
					{
						target = target->m_next;
						continue;
					}

					// Skip if not on the same map
					if (agressor->getMap() != target->getMap())
					{
						target = target->m_next;
						continue;
					}
					execute_aggro(agressor, *target, aggro_script, aggro_dist);
					target = target->m_next;
				}
				free(aggro_script);
			}
		}
	}

	// Compute aggro of all other NPC to the current context
	target = agressor;
	npc = context_get_first();

	while (npc != nullptr)
	{
		// Skip current context
		if (target == npc)
		{
			npc = npc->m_next;
			continue;
		}
		if (npc->getId() == "")
		{
			npc = npc->m_next;
			continue;
		}
		if (npc->getMap() == "")
		{
			npc = npc->m_next;
			continue;
		}
		if (npc->isNpc() == false)
		{
			npc = npc->m_next;
			continue;
		}
		// Skip if not on the same map
		if (npc->getMap() != target->getMap())
		{
			npc = npc->m_next;
			continue;
		}

		if (entry_read_int(CHARACTER_TABLE, npc->getId().c_str(), &aggro_dist,
				CHARACTER_KEY_AGGRO_DIST, nullptr) == false)
		{
			npc = npc->m_next;
			continue;
		}
		if (entry_read_string(CHARACTER_TABLE, npc->getId().c_str(),
				&aggro_script, CHARACTER_KEY_AGGRO_SCRIPT, nullptr) == false)
		{
			npc = npc->m_next;
			continue;
		}

		execute_aggro(npc, *target, aggro_script, aggro_dist);

		free(aggro_script);

		npc = npc->m_next;
	}
}
/*************************************************************
 *************************************************************/
static void do_set_pos(Context *ctx, const std::string &map, int x, int y,
		bool change_map)
{
	ctx->setMap(map);
	ctx->setTile(x, y);

	entry_write_string(CHARACTER_TABLE, ctx->getId().c_str(), map.c_str(),
			CHARACTER_KEY_MAP, nullptr);
	entry_write_int(CHARACTER_TABLE, ctx->getId().c_str(), x,
			CHARACTER_KEY_TILE_X, nullptr);
	entry_write_int(CHARACTER_TABLE, ctx->getId().c_str(), y,
			CHARACTER_KEY_TILE_Y, nullptr);

	context_spread(ctx);
	if (change_map == true)
	{
		context_request_other_context(ctx);
	}
}

/*************************************************************
 Move every context on the same coordinate as platform context
 *************************************************************/
static void platform_move(Context *platform, const std::string &map, int x,
		int y, bool change_map)
{
	Context *current = context_get_first();
	int is_platform;

	if (entry_read_int(CHARACTER_TABLE, platform->getId().c_str(), &is_platform,
			CHARACTER_KEY_PLATFORM, nullptr) == false)
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
			current = current->m_next;
			continue;
		}
		if ((platform->getTileX() == current->getTileX())
				&& (platform->getTileY() == current->getTileY())
				&& (platform->getMap() == current->getMap()))
		{
			do_set_pos(current, map, x, y, change_map);
		}
		current = current->m_next;
	}
}

/******************************************************
 return 0 if new position OK or if position has not changed.
 return -1 if the position was not set (because tile not allowed or out of bound)
 ******************************************************/
int character_set_pos(Context *ctx, const std::string &map, int x, int y)
{
	char **event_id;
	char *script;
	char **param = nullptr;
	int i;
	bool change_map = false;
	int width = x + 1;
	int height = y + 1;
	int warpx = 0;
	int warpy = 0;
	int ctx_layer = 0;
	char layer_name[SMALL_BUF];
	char buf[SMALL_BUF];
	char *coord[3];
	int ret_value;
	int layer;

	if (ctx == nullptr)
	{
		return -1;
	}

	// Do nothing if no move
	if ((ctx->getMap() == map) && (ctx->getTileX() == x)
			&& (ctx->getTileY() == y))
	{
		return 0;
	}

	ctx_layer = 0;
	entry_read_int(CHARACTER_TABLE, ctx->getId().c_str(), &ctx_layer,
			CHARACTER_LAYER, nullptr);
	sprintf(layer_name, "%s%d", MAP_KEY_LAYER.c_str(), ctx_layer);

	entry_read_int(MAP_TABLE, map.c_str(), &width, MAP_KEY_WIDTH, nullptr);
	entry_read_int(MAP_TABLE, map.c_str(), &height, MAP_KEY_HEIGHT, nullptr);
	entry_read_int(MAP_TABLE, map.c_str(), &warpx, MAP_KEY_WARP_X, nullptr);
	entry_read_int(MAP_TABLE, map.c_str(), &warpy, MAP_KEY_WARP_Y, nullptr);

	// Offscreen script
	entry_read_string(MAP_TABLE, map.c_str(), &script, MAP_OFFSCREEN, nullptr);
	if (script != nullptr && (x < 0 || y < 0 || x >= width || y >= height))
	{
		snprintf(buf, SMALL_BUF, "%d", x);
		coord[0] = strdup(buf);
		snprintf(buf, SMALL_BUF, "%d", y);
		coord[1] = strdup(buf);
		coord[2] = nullptr;

		ret_value = action_execute_script(ctx, script, (const char**) coord);

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
		ret_value = map_check_tile(ctx, ctx->getId().c_str(), map, layer, x, y);
		// not allowed
		if (ret_value == 0)
		{
			return -1;
		}
		// allowed
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

	if (ctx->getMap() != map)
	{
		change_map = true;
	}

	// If this character is a platform, move all characters on it
	platform_move(ctx, map, x, y, change_map);

	do_set_pos(ctx, map, x, y, change_map);

	event_id = map_get_event(map, ctx_layer, x, y);

	if (event_id)
	{
		i = 0;
		while (event_id[i])
		{
			script = nullptr;
			if (entry_read_string(MAP_TABLE, map.c_str(), &script, layer_name,
					MAP_ENTRY_EVENT_LIST, event_id[i], MAP_EVENT_SCRIPT,
					nullptr) == true)
			{
				entry_read_list(MAP_TABLE, map.c_str(), &param, layer_name,
						MAP_ENTRY_EVENT_LIST, event_id[i], MAP_EVENT_PARAM,
						nullptr);
			}
			else if (entry_read_string(MAP_TABLE, map.c_str(), &script,
					MAP_ENTRY_EVENT_LIST, event_id[i], MAP_EVENT_SCRIPT,
					nullptr) == true)
			{
				entry_read_list(MAP_TABLE, map.c_str(), &param,
						MAP_ENTRY_EVENT_LIST, event_id[i], MAP_EVENT_PARAM,
						nullptr);
			}

			if (script == nullptr)
			{
				i++;
				continue;
			}

			action_execute_script(ctx, script, (const char**) param);

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
 If the value is != 0 , the NPC is instantiated
 return -1 on error
 *********************************************************/
int character_set_npc(const char *id, int npc)
{
	if (entry_write_int(CHARACTER_TABLE, id, npc, CHARACTER_KEY_NPC,
			nullptr) == false)
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
int character_get_npc(const std::string &id)
{
	int npc;

	if (entry_read_int(CHARACTER_TABLE, id.c_str(), &npc, CHARACTER_KEY_NPC,
			nullptr) == false)
	{
		return 0;
	}

	return npc;
}

/*********************************************************
 return -1 if error
 *********************************************************/
int character_set_portrait(const char *id, const char *portrait)
{
	Context *ctx;

	if (entry_write_string(CHARACTER_TABLE, id, portrait,
			CHARACTER_KEY_PORTRAIT, nullptr) == false)
	{
		return -1;
	}

	ctx = context_find(id);
	network_send_character_file(ctx);

	return true;
}

/*********************************************************
 Get character portrait.
 must be freed
 *********************************************************/
char* character_get_portrait(const char *id)
{
	char *portrait;

	if (entry_read_string(CHARACTER_TABLE, id, &portrait,
			CHARACTER_KEY_PORTRAIT, nullptr) == false)
	{
		return nullptr;
	}

	return portrait;
}

/*********************************************************
 Set AI script name
 return -1 on error
 *********************************************************/
int character_set_ai_script(const char *id, const char *script_name)
{
	if (entry_write_string(CHARACTER_TABLE, id, script_name, CHARACTER_KEY_AI,
			nullptr) == false)
	{
		return -1;
	}

	return 0;
}

/*********************************************************
 Wake-up NPC. Execute it's AI script immediately
 return -1 on error
 *********************************************************/
int character_wake_up(const char *id)
{
	Context *ctx = nullptr;

	ctx = context_find(id);

	// Wake up NPC
	ctx->setNextExecutionTick(0);
	ctx->wakeUp();

	return 0;
}

/***********************************
 Write a new filename in a character's sprite list
 return false on error
 ***********************************/
int character_set_sprite(const char *id, int index, const char *filename)
{
	if (id == nullptr || filename == nullptr)
	{
		return false;
	}

	if (entry_write_list_index(CHARACTER_TABLE, id, filename, index,
			CHARACTER_KEY_SPRITE, nullptr) == false)
	{
		return false;
	}

	return true;
}

/******************************************************************************
 Write a new filename in a character's sprite direction list
 return false on error
 *****************************************************************************/
int character_set_sprite_dir(const std::string &id, const std::string &dir,
		int index, const std::string &filename)
{
	std::string key;

	if (id.size() == 0 || filename.size() == 0 || dir.size() == 0)
	{
		return false;
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
		return false;
		break;
	}

	if (entry_write_list_index(CHARACTER_TABLE, id.c_str(), filename.c_str(),
			index, key.c_str(), nullptr) == false)
	{
		return false;
	}

	return true;
}

/***********************************
 Write a new filename in a character's moving sprite list
 return false on error
 ***********************************/
int character_set_sprite_move(const char *id, const char *dir, int index,
		const char *filename)
{
	std::string key;

	if (id == nullptr || filename == nullptr || dir == nullptr)
	{
		return false;
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
		return false;
		break;
	}

	if (entry_write_list_index(CHARACTER_TABLE, id, filename, index, key,
			nullptr) == false)
	{
		return false;
	}

	return true;
}

/***********************************
 Broadcast character file to other context
 ***********************************/
void character_broadcast(const char *character)
{
	context_broadcast_character(character);
}
