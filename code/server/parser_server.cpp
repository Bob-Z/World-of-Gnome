/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2017 carabobz@gmail.com

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
#include "character.h"
#include "action.h"
#include <string.h>

/**************************************
 Return RET_NOK on error
 **************************************/
ret_code_t parse_incoming_data(context_t * context, Uint32 command,
		Uint32 command_size, char * data)
{
	char * value = nullptr;
	char * fullname;
	char * elements[512];
	char * cksum;
	int i;
	char * user_name;
	char * password;

	if (!context_get_connected(context)
			&& (command != CMD_REQ_LOGIN && command != CMD_REQ_FILE))
	{
		werr(LOGUSER,
				"Request from not authenticated client, close connection");
		return RET_NOK;
	}

	switch (command)
	{
	case CMD_REQ_LOGIN:
		wlog(LOGDEBUG, "Received CMD_REQ_LOGIN");
		user_name = _strsep(&data, NETWORK_DELIMITER);
		password = _strsep(&data, NETWORK_DELIMITER);

		if (entry_read_string(PASSWD_TABLE, user_name, &value,
				PASSWD_KEY_PASSWORD, nullptr) == RET_NOK)
		{
			return RET_NOK;
		}
		if (strcmp(value, password) != 0)
		{
			free(value);
			werr(LOGUSER, "Wrong login for %s", user_name);
			// send answer
			network_send_command(context, CMD_SEND_LOGIN_NOK, 0, nullptr,
					false);
			// force client disconnection
			return RET_NOK;
		}
		else
		{
			free(value);

			if (context_set_username(context, user_name) == RET_NOK)
			{
				return RET_NOK;
			}
			context_set_connected(context, true);

			// send answer
			network_send_command(context, CMD_SEND_LOGIN_OK, 0, nullptr, false);
			wlog(LOGUSER, "Login successful for user %s", context->user_name);
		}
		break;
	case CMD_REQ_PLAYABLE_CHARACTER_LIST:
		wlog(LOGDEBUG, "Received CMD_REQ_PLAYABLE_CHARACTER_LIST");
		character_playable_send_list(context);
		wlog(LOGDEBUG, "character list sent");
		break;
	case CMD_REQ_FILE:
		i = 0;
		elements[i] = _strsep(&data, NETWORK_DELIMITER);
		while (elements[i])
		{
			i++;
			elements[i] = _strsep(&data, NETWORK_DELIMITER);
		}

		if (elements[0] == nullptr || elements[1] == nullptr)
		{
			werr(LOGDEV, "Received erroneous CMD_REQ_FILE");
			break;
		}
		wlog(LOGDEBUG, "Received CMD_REQ_FILE for %s", elements[0]);
		// compare checksum
		fullname = strconcat(base_directory, "/", elements[0], nullptr);

		cksum = checksum_file(fullname);
		free(fullname);

		if (cksum == nullptr)
		{
			werr(LOGUSER, "Required file %s doesn't exists", elements[0]);
			break;
		}

		if (strcmp(elements[1], cksum) == 0)
		{
			wlog(LOGDEBUG, "Client has already newest %s file", elements[0]);
			free(cksum);
			break;
		}
		free(cksum);

		network_send_file(context, elements[0]);
		wlog(LOGDEBUG, "File %s sent", elements[0]);
		break;
	case CMD_REQ_USER_CHARACTER_LIST:
		wlog(LOGDEBUG, "Received CMD_REQ_USER_CHARACTER_LIST");
		character_user_send_list(context);
		wlog(LOGDEBUG, "user %s's character list sent", context->user_name);
		break;
	case CMD_REQ_START:
		if (context->in_game == false)
		{
			context->id = strdup(data);
			context->in_game = true;
			context_update_from_file(context);
			context_spread(context);
			context_request_other_context(context);
		}
		wlog(LOGDEBUG, "Received CMD_REQ_START for %s /%s", context->user_name,
				context->id);
		break;
	case CMD_REQ_STOP:
		wlog(LOGDEBUG, "Received CMD_REQ_STOP for %s /%s", context->user_name,
				context->id);
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
		i = 0;
		elements[i] = nullptr;
		elements[i] = _strsep(&data, NETWORK_DELIMITER);
		while (elements[i])
		{
			i++;
			elements[i] = _strsep(&data, NETWORK_DELIMITER);
		}
		elements[i + 1] = nullptr;

		wlog(LOGDEBUG, "Received CMD_REQ_ACTION %s from %s /%s", elements[0],
				context->user_name, context->character_name);

		action_execute(context, elements[0], &elements[1]);
		break;
	case CMD_REQ_CREATE:
	{
		char * l_Id = _strsep(&data, NETWORK_DELIMITER);
		char * l_Name = _strsep(&data, NETWORK_DELIMITER);
		wlog(LOGDEBUG, "Received CMD_REQ_CREATE: ID=%s, NAME=%s", l_Id, l_Name);

		char * l_FileName = nullptr;
		l_FileName = file_new(CHARACTER_TABLE, l_Name);
		if (l_FileName == nullptr)
		{
			werr(LOGUSER, "%s already exists", l_Name);
			break;
		}

		if (file_copy(CHARACTER_TEMPLATE_TABLE, l_Id, CHARACTER_TABLE, l_Name)
				== false)
		{
			werr(LOGUSER,
					"Error copying character template %s to character %s (maybe template doesn't exists ?)",
					l_Id, l_Name);
			file_delete(CHARACTER_TABLE, l_Name);
			break;
		}

		if (entry_write_string(CHARACTER_TABLE, l_Name, l_Name,
				CHARACTER_KEY_NAME, nullptr) == RET_NOK)
		{
			werr(LOGUSER, "Error setting character name %s", l_Name);
			file_delete(CHARACTER_TABLE, l_Name);
			break;
		}

		if (entry_add_to_list(USERS_TABLE, context->user_name, l_Name,
				USERS_CHARACTER_LIST, nullptr) == RET_NOK)
		{
			werr(LOGUSER, "Error adding character %s to user %s", l_Name,
					context->user_name);
			file_delete(CHARACTER_TABLE, l_Name);
			break;
		}

		character_user_send(context, l_Name);

		wlog(LOGDEBUG, "Successfully created: ID=%s, NAME=%s", l_Id, l_Name);
	}
		break;
	default:
		werr(LOGDEV, "Unknown request %d from client", command);
		return RET_NOK;
	}

	return RET_OK;
}
