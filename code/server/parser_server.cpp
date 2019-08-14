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
	wlog(LOGDEVELOPER, "[network] Received login");

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
		wlog(LOGUSER, "Login successful for user %s", context->user_name);

		return RET_OK;
	}
}

/**************************************
 Return RET_NOK on error
 **************************************/
ret_code_t parse_incoming_data(context_t * context, NetworkFrame & frame)
{
	uint_fast32_t l_Command = 0U;
	frame.pop(l_Command);

	if ((context_get_connected(context) == false)
			&& ((l_Command != CMD_REQ_FILE) && (l_Command != CMD_PB)))
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
		pb::NetworkMessage message;
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
			else
			{
				werr(LOGUSER, "Unknown message received");
			}
		}

		break;
	}
	case CMD_REQ_PLAYABLE_CHARACTER_LIST:
		wlog(LOGDEVELOPER, "Received CMD_REQ_PLAYABLE_CHARACTER_LIST");
		character_playable_send_list(context);
		wlog(LOGDEVELOPER, "character list sent");
		break;
	case CMD_REQ_FILE:
	{
		std::string l_FileName;
		frame.pop(l_FileName);
		std::string l_CheckSum;
		frame.pop(l_CheckSum);

		wlog(LOGDEVELOPER, "Received CMD_REQ_FILE for %s", l_FileName.c_str());
		// compare checksum
		char * l_pFullName = strconcat(base_directory, "/", l_FileName.c_str(),
				nullptr);

		char * l_LocalCheckSum = checksum_file(l_pFullName);
		free(l_pFullName);

		if (l_LocalCheckSum == nullptr)
		{
			werr(LOGUSER, "Required file %s doesn't exists",
					l_FileName.c_str());
			break;
		}

		if (strcmp(l_CheckSum.c_str(), l_LocalCheckSum) == 0)
		{
			wlog(LOGDEVELOPER, "Client has already newest %s file",
					l_FileName.c_str());
			free(l_LocalCheckSum);
			break;
		}
		free(l_LocalCheckSum);

		network_send_file(context, l_FileName.c_str());
		wlog(LOGDEVELOPER, "File %s sent", l_FileName.c_str());
		break;
	}
	case CMD_REQ_USER_CHARACTER_LIST:
		wlog(LOGDEVELOPER, "Received CMD_REQ_USER_CHARACTER_LIST");
		character_user_send_list(context);
		wlog(LOGDEVELOPER, "user %s's character list sent", context->user_name);
		break;
	case CMD_REQ_START:
		if (context->in_game == false)
		{
			char * l_Id = nullptr;
			frame.pop(l_Id);

			context->id = strdup(l_Id);
			free(l_Id);

			context->in_game = true;
			context_update_from_file(context);
			context_spread(context);
			context_request_other_context(context);

		}
		wlog(LOGDEVELOPER, "Received CMD_REQ_START for %s /%s",
				context->user_name, context->id);
		break;
	case CMD_REQ_STOP:
		wlog(LOGDEVELOPER, "Received CMD_REQ_STOP for %s /%s",
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
		break;
	case CMD_REQ_ACTION:
	{
		std::string l_ActionName;
		frame.pop(l_ActionName);
		std::vector<std::string> l_Param;
		frame.pop(l_Param);

		wlog(LOGDEVELOPER, "Received CMD_REQ_ACTION %s from %s /%s",
				l_ActionName.c_str(), context->user_name,
				context->character_name);

		action_execute(context, l_ActionName, l_Param);
	}
		break;
	case CMD_REQ_CREATE:
	{
		std::string l_Id;
		frame.pop(l_Id);
		std::string l_Name;
		frame.pop(l_Name);

		wlog(LOGDEVELOPER, "Received CMD_REQ_CREATE: ID=%s, NAME=%s",
				l_Id.c_str(), l_Name.c_str());

		char * l_FileName = nullptr;
		l_FileName = file_new(CHARACTER_TABLE, l_Name.c_str());
		if (l_FileName == nullptr)
		{
			werr(LOGUSER, "%s already exists", l_Name.c_str());
			break;
		}

		if (file_copy(CHARACTER_TEMPLATE_TABLE, l_Id.c_str(), CHARACTER_TABLE,
				l_Name.c_str()) == false)
		{
			werr(LOGUSER,
					"Error copying character template %s to character %s (maybe template doesn't exists ?)",
					l_Id, l_Name);
			file_delete(CHARACTER_TABLE, l_Name.c_str());
			break;
		}

		if (entry_write_string(CHARACTER_TABLE, l_Name.c_str(), l_Name.c_str(),
		CHARACTER_KEY_NAME, nullptr) == RET_NOK)
		{
			werr(LOGUSER, "Error setting character name %s", l_Name.c_str());
			file_delete(CHARACTER_TABLE, l_Name.c_str());
			break;
		}

		if (entry_add_to_list(USERS_TABLE, context->user_name, l_Name.c_str(),
		USERS_CHARACTER_LIST, nullptr) == RET_NOK)
		{
			werr(LOGUSER, "Error adding character %s to user %s",
					l_Name.c_str(), context->user_name);
			file_delete(CHARACTER_TABLE, l_Name.c_str());
			break;
		}

		character_user_send(context, l_Name.c_str());

		wlog(LOGDEVELOPER, "Successfully created: ID=%s, NAME=%s", l_Id.c_str(),
				l_Name.c_str());
	}
		break;
	default:
		werr(LOGDESIGNER, "Unknown request %d from client", l_Command);
		return RET_NOK;
	}

	return RET_OK;
}
