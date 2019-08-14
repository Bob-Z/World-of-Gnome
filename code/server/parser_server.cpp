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
#include "wog.pb.h"
#include <string.h>

/**************************************
 Return RET_NOK on error
 **************************************/
static ret_code_t manage_login(context_t * context, const pb::Login & login)
{
	wlog(LOGDEVELOPER, "[network] Received login request for user %s",
			login.user().c_str());

	char * password = nullptr;
	if (entry_read_string(PASSWD_TABLE, login.user().c_str(), &password,
	PASSWD_KEY_PASSWORD, nullptr) == RET_NOK)
	{
		return RET_NOK;
	}

	if (strcmp(password, login.password().c_str()) != 0)
	{
		free(password);
		werr(LOGUSER, "[network] Wrong login for %s", login.user().c_str());
		network_send_command_no_data(context, CMD_SEND_LOGIN_NOK, false);
		// force client disconnection
		return RET_NOK;
	}
	else
	{
		free(password);
		if (context_set_username(context, login.user().c_str()) == RET_NOK)
		{
			return RET_NOK;
		}
		context_set_connected(context, true);

		network_send_command_no_data(context, CMD_SEND_LOGIN_OK, false);
		wlog(LOGUSER, "[network] Login successful for user %s",
				context->user_name);

		return RET_OK;
	}
}

/**************************************
 Return RET_NOK on error
 **************************************/
static ret_code_t manage_start(context_t * context, const pb::Start & start)
{
	wlog(LOGDEVELOPER, "[network] Received start");

	if (context->in_game == false)
	{
		context->id = strdup(start.id().c_str());
		context->in_game = true;
		context_update_from_file(context);
		context_spread(context);
		context_request_other_context(context);
	}
	wlog(LOGDEVELOPER, "[network] received start request for ID %s and user %s",
			context->id, context->user_name);

	return RET_OK;
}

/**************************************
 Return RET_NOK on error
 **************************************/
static ret_code_t manage_stop(context_t * context, const pb::Stop & stop)
{
	wlog(LOGDEVELOPER, "[network] Received stop request for ID %s of user %s",
			context->user_name, context->id);

	if (context->in_game == true)
	{
		context->in_game = false;
		if (context->map)
		{
			free(context->map);
		}
		context->map = nullptr;
		if (context->prev_map)
		{
			free(context->prev_map);
		}
		context->prev_map = nullptr;
		if (context->id)
		{
			free(context->id);
		}
		context->id = nullptr;
		context_spread(context);
	}

	return RET_OK;
}

/**************************************
 Return RET_NOK on error
 **************************************/
static ret_code_t manage_playable_character_list(context_t * context,
		const pb::PlayableCharacterList & list)
{
	wlog(LOGDEVELOPER, "[network] Received playable character list request");

	character_playable_send_list(context);

	return RET_OK;
}

/**************************************
 Return RET_NOK on error
 **************************************/
static ret_code_t manage_user_character_list(context_t * context,
		const pb::UserCharacterList & list)
{
	wlog(LOGDEVELOPER,
			"[network] Received user character list request for user %s",
			list.user().c_str());

	character_user_send_list(context);

	return RET_OK;
}

/**************************************
 Return RET_NOK on error
 **************************************/
static ret_code_t manage_create(context_t * context, const pb::Create& create)
{
	wlog(LOGDEVELOPER, "[network] Received create ID %s with name %s",
			create.id().c_str(), create.name().c_str());

	char * file_name = nullptr;
	file_name = file_new(CHARACTER_TABLE, create.name().c_str());
	if (file_name == nullptr)
	{
		werr(LOGUSER, "%s already exists", create.name().c_str());
		return RET_NOK;
	}

	if (file_copy(CHARACTER_TEMPLATE_TABLE, create.id().c_str(),
	CHARACTER_TABLE, create.name().c_str()) == false)
	{
		werr(LOGUSER,
				"Error copying character template %s to character %s (maybe template doesn't exists ?)",
				create.id(), create.name());
		file_delete(CHARACTER_TABLE, create.name().c_str());
		return RET_NOK;
	}

	if (entry_write_string(CHARACTER_TABLE, create.name().c_str(),
			create.name().c_str(),
			CHARACTER_KEY_NAME, nullptr) == RET_NOK)
	{
		werr(LOGUSER, "Error setting character name %s", create.name().c_str());
		file_delete(CHARACTER_TABLE, create.name().c_str());
		return RET_NOK;
	}

	if (entry_add_to_list(USERS_TABLE, context->user_name,
			create.name().c_str(),
			USERS_CHARACTER_LIST, nullptr) == RET_NOK)
	{
		werr(LOGUSER, "Error adding character %s to user %s",
				create.name().c_str(), context->user_name);
		file_delete(CHARACTER_TABLE, create.name().c_str());
		return RET_NOK;
	}

	character_user_send(context, create.name().c_str());

	wlog(LOGDEVELOPER, "Successfully created: ID=%s, NAME=%s",
			create.id().c_str(), create.name().c_str());

	return RET_OK;
}

/**************************************
 Return RET_NOK on error
 **************************************/
static ret_code_t manage_action(context_t * context, const pb::Action& action)
{
	wlog(LOGDEVELOPER, "[network] Received action script %s",
			action.script().c_str());

	std::vector<std::string> params;
	for (int i = 0; i < action.params_size(); i++)
	{
		params.push_back(action.params(i));
	}

	action_execute(context, action.script().c_str(), params);

	return RET_OK;
}

/**************************************
 Return RET_NOK on error
 **************************************/
static ret_code_t manage_file(context_t * context, const pb::File& file)
{
	wlog(LOGDEVELOPER, "[network] Received file request for %s",
			file.name().c_str());

	char * file_path = strconcat(base_directory, "/", file.name().c_str(),
			nullptr);

	char * crc = checksum_file(file_path);
	free(file_path);

	if (crc == nullptr)
	{
		werr(LOGUSER, "Required file %s doesn't exists", file_path);
		return RET_NOK;
	}

	if (strcmp(file.crc().c_str(), crc) == 0)
	{
		wlog(LOGDEVELOPER, "Client has already newest %s file", file.name());
		free(crc);
		return RET_NOK;
	}
	free(crc);

	network_send_file(context, file.name().c_str());
	wlog(LOGDEVELOPER, "File %s sent", file.name());

	return RET_OK;
}

/**************************************
 Return RET_NOK on error
 **************************************/
ret_code_t parse_incoming_data(context_t * context, NetworkFrame & frame)
{
	uint_fast32_t l_Command = 0U;
	frame.pop(l_Command);

	if ((context_get_connected(context) == false) && (l_Command != CMD_PB))
	{
		werr(LOGUSER,
				"Request from not authenticated client, close connection");
		return RET_NOK;
	}

	switch (l_Command)
	{
	case CMD_PB:
	{
		std::string serialized_data;
		frame.pop(serialized_data);
		pb::ClientMessage message;
		if (message.ParseFromString(serialized_data) == false)
		{
			werr(LOGUSER, "Parsing failed");
		}
		else
		{
			if (message.has_login())
			{
				manage_login(context, message.login());
			}
			else if (message.has_start())
			{
				manage_start(context, message.start());
			}
			else if (message.has_stop())
			{
				manage_stop(context, message.stop());
			}
			else if (message.has_playable_character_list())
			{
				manage_playable_character_list(context,
						message.playable_character_list());
			}
			else if (message.has_user_character_list())
			{
				manage_user_character_list(context,
						message.user_character_list());
			}
			else if (message.has_create())
			{
				manage_create(context, message.create());
			}
			else if (message.has_action())
			{
				manage_action(context, message.action());
			}
			else if (message.has_file())
			{
				manage_file(context, message.file());
			}
			else
			{
				werr(LOGUSER, "Unknown message received");
			}
		}

		break;
	}
	default:
		werr(LOGDESIGNER, "Unknown request %d from client", l_Command);
		return RET_NOK;
	}

	return RET_OK;
}
