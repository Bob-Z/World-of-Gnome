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

#include "action.h"
#include "character.h"
#include "common.h"
#include <string.h>

/**************************************
 Return RET_NOK on error
 **************************************/
ret_code_t parse_incoming_data(context_t * p_pContext, NetworkFrame & p_rFrame)
{
	uint_fast32_t l_Command = 0U;
	p_rFrame.pop(l_Command);

	if ((context_get_connected(p_pContext) == false)
			&& ((l_Command != CMD_REQ_LOGIN) && (l_Command != CMD_REQ_FILE)))
	{
		werr(LOGUSER,
				"Request from not authenticated client, close connection");
		return RET_NOK;
	}

	switch (l_Command)
	{
	case CMD_REQ_LOGIN:
	{
		wlog(LOGDEBUG, "Received CMD_REQ_LOGIN");

		std::string l_UserName;
		p_rFrame.pop(l_UserName);
		std::string l_PassWord;
		p_rFrame.pop(l_PassWord);

		char * l_pValue = nullptr;
		if (entry_read_string(PASSWD_TABLE, l_UserName.c_str(), &l_pValue,
		PASSWD_KEY_PASSWORD, nullptr) == RET_NOK)
		{
			return RET_NOK;
		}

		if (strcmp(l_pValue, l_PassWord.c_str()) != 0)
		{
			free(l_pValue);
			werr(LOGUSER, "Wrong login for %s", l_UserName.c_str());
			network_send_command_no_data(p_pContext, CMD_SEND_LOGIN_NOK, false);
			// force client disconnection
			return RET_NOK;
		}
		else
		{
			free(l_pValue);
			if (context_set_username(p_pContext, l_UserName.c_str()) == RET_NOK)
			{
				return RET_NOK;
			}
			context_set_connected(p_pContext, true);

			// send answer
			network_send_command_no_data(p_pContext, CMD_SEND_LOGIN_OK, false);
			wlog(LOGUSER, "Login successful for user %s",
					p_pContext->user_name);
		}
		break;
	}
	case CMD_REQ_PLAYABLE_CHARACTER_LIST:
		wlog(LOGDEBUG, "Received CMD_REQ_PLAYABLE_CHARACTER_LIST");
		character_playable_send_list(p_pContext);
		wlog(LOGDEBUG, "character list sent");
		break;
	case CMD_REQ_FILE:
	{
		std::string l_FileName;
		p_rFrame.pop(l_FileName);
		std::string l_CheckSum;
		p_rFrame.pop(l_CheckSum);

		wlog(LOGDEBUG, "Received CMD_REQ_FILE for %s", l_FileName.c_str());
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
			wlog(LOGDEBUG, "Client has already newest %s file",
					l_FileName.c_str());
			free(l_LocalCheckSum);
			break;
		}
		free(l_LocalCheckSum);

		network_send_file(p_pContext, l_FileName.c_str());
		wlog(LOGDEBUG, "File %s sent", l_FileName.c_str());
		break;
	}
	case CMD_REQ_USER_CHARACTER_LIST:
		wlog(LOGDEBUG, "Received CMD_REQ_USER_CHARACTER_LIST");
		character_user_send_list(p_pContext);
		wlog(LOGDEBUG, "user %s's character list sent", p_pContext->user_name);
		break;
	case CMD_REQ_START:
		if (p_pContext->in_game == false)
		{
			char * l_Id = nullptr;
			p_rFrame.pop(l_Id);

			p_pContext->id = strdup(l_Id);
			free(l_Id);

			p_pContext->in_game = true;
			context_update_from_file(p_pContext);
			context_spread(p_pContext);
			context_request_other_context(p_pContext);

		}
		wlog(LOGDEBUG, "Received CMD_REQ_START for %s /%s",
				p_pContext->user_name, p_pContext->id);
		break;
	case CMD_REQ_STOP:
		wlog(LOGDEBUG, "Received CMD_REQ_STOP for %s /%s",
				p_pContext->user_name, p_pContext->id);
		if (p_pContext->in_game == true)
		{
			p_pContext->in_game = false;
			if (p_pContext->map)
			{
				free(p_pContext->map);
			}
			p_pContext->map = nullptr;
			if (p_pContext->prev_map)
			{
				free(p_pContext->prev_map);
			}
			p_pContext->prev_map = nullptr;
			if (p_pContext->id)
			{
				free(p_pContext->id);
			}
			p_pContext->id = nullptr;
			context_spread(p_pContext);
		}
		break;
	case CMD_REQ_ACTION:
		/*
		 i = 0;
		 elements[i] = nullptr;
		 //elements[i] = _strsep(&data, NETWORK_DELIMITER);
		 while (elements[i])
		 {
		 i++;
		 //elements[i] = _strsep(&data, NETWORK_DELIMITER);
		 }
		 elements[i + 1] = nullptr;

		 wlog(LOGDEBUG, "Received CMD_REQ_ACTION %s from %s /%s", elements[0],
		 p_pContext->user_name, p_pContext->character_name);

		 action_execute(p_pContext, elements[0], &elements[1]);
		 */
		break;
	case CMD_REQ_CREATE:
	{
		/*
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
		 */
	}
		break;
	default:
		werr(LOGDEV, "Unknown request %d from client", l_Command);
		return RET_NOK;
	}

	return RET_OK;
}
