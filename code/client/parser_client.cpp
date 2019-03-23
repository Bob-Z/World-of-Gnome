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

#include "common.h"
#include "EffectManager.h"
#include "file.h"
#include "imageDB.h"
#include "network_client.h"
#include "scr_create.h"
#include "scr_select.h"
#include "screen.h"
#include "textview.h"
#include "ui_play.h"

/***********************************
 Return RET_NOK on error
 ***********************************/
ret_code_t parse_incoming_data(context_t * p_pContext, NetworkFrame & p_rFrame)
{
	uint_fast32_t l_Command = 0U;
	p_rFrame.pop(l_Command);

	switch (l_Command)
	{
	case CMD_SEND_LOGIN_OK:
		wlog(LOGDEVELOPER, "Received CMD_SEND_LOGIN_OK");
		if (network_open_data_connection(p_pContext) == RET_NOK)
		{
			return RET_NOK;
		}
		context_set_connected(p_pContext, true);
		wlog(LOGUSER, "Successfully connected");
		network_request_user_character_list(p_pContext);
		wlog(LOGDEVELOPER, "Character list requested");
		break;
	case CMD_SEND_LOGIN_NOK:
		wlog(LOGDEVELOPER, "Received CMD_SEND_LOGIN_NOK");
		context_set_connected(p_pContext, false);
		werr(LOGUSER,
				"Check your login and password (they are case sensitive)\n");
		exit(-1);
		break;
	case CMD_SEND_PLAYABLE_CHARACTER:
		wlog(LOGDEVELOPER, "Received CMD_SEND_PLAYABLE_CHARACTER");
		scr_create_add_playable_character(p_pContext, p_rFrame);
		screen_compose();
		break;
	case CMD_SEND_FILE:
		wlog(LOGDEVELOPER, "Received CMD_SEND_FILE");
		file_add(p_pContext, p_rFrame);
		screen_compose();
		break;
	case CMD_SEND_USER_CHARACTER:
		wlog(LOGDEVELOPER, "Received CMD_SEND_USER_CHARACTER");
		scr_select_add_user_character(p_pContext, p_rFrame);
		screen_compose();
		break;
	case CMD_SEND_CONTEXT:
		wlog(LOGDEVELOPER, "Received CMD_SEND_CONTEXT");
		context_add_or_update_from_network_frame(p_pContext, p_rFrame);
		screen_compose();
		break;
	case CMD_SEND_TEXT:
	{
		wlog(LOGDEVELOPER, "Received CMD_SEND_TEXT");
		std::string l_Text;
		p_rFrame.pop(l_Text);

		textview_add_line(l_Text);
	}
		break;
	case CMD_SEND_ENTRY:
		wlog(LOGDEVELOPER, "Received CMD_SEND_ENTRY");
		if (entry_update(p_rFrame) != -1)
		{
			screen_compose();
		}
		break;
	case CMD_SEND_POPUP:
		wlog(LOGDEVELOPER, "Received CMD_SEND_POPUP");
		ui_play_popup_add(p_rFrame);
		screen_compose();
		break;
	case CMD_SEND_EFFECT:
		EffectManager::processEffectFrame(p_pContext, p_rFrame);
		wlog(LOGDEVELOPER, "Received CMD_SEND_EFFECT");
		break;
	default:
		werr(LOGDESIGNER, "Unknown request from server");
		return RET_NOK;
		break;
	}

	return RET_OK;
}
