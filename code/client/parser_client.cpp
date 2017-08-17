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
#include "network_client.h"
#include "scr_select.h"
#include "scr_create.h"
#include "file.h"
#include "imageDB.h"
#include "textview.h"
#include "screen.h"
#include "ui_play.h"

/***********************************
 Return RET_NOK on error
***********************************/
ret_code_t parse_incoming_data(context_t * context, Uint32 command, Uint32 command_size, char * data)
{
	switch(command) {
	case CMD_SEND_LOGIN_OK :
		wlog(LOGDEBUG,"Received CMD_SEND_LOGIN_OK");
		if(network_open_data_connection(context) == RET_NOK ) {
			return RET_NOK;
		}
		context_set_connected(context, true);
		wlog(LOGUSER,"Successfully connected");
		network_request_user_character_list(context);
		wlog(LOGDEBUG,"Character list requested");
		break;
	case CMD_SEND_LOGIN_NOK :
		wlog(LOGDEBUG,"Received CMD_SEND_LOGIN_NOK");
		context_set_connected(context, false);
		werr(LOGUSER,"Check your login and password (they are case sensitive)\n");
		exit(-1);
		break;
	case CMD_SEND_PLAYABLE_CHARACTER :
		wlog(LOGDEBUG,"Received CMD_SEND_PLAYABLE_CHARACTER");
		scr_create_add_playable_character(context,data);
		screen_compose();
		break;
	case CMD_SEND_FILE :
		wlog(LOGDEBUG,"Received CMD_SEND_FILE");
		file_add(context,data,command_size);
		screen_compose();
		break;
	case CMD_SEND_USER_CHARACTER :
		wlog(LOGDEBUG,"Received CMD_SEND_USER_CHARACTER");
		scr_select_add_user_character(context,data);
		screen_compose();
		break;
	case CMD_SEND_CONTEXT :
		wlog(LOGDEBUG,"Received CMD_SEND_CONTEXT");
		context_add_or_update_from_network_frame(context,data);
		screen_compose();
		break;
	case CMD_SEND_TEXT :
		wlog(LOGDEBUG,"Received CMD_SEND_TEXT");
		textview_add_line(data);
		break;
	case CMD_SEND_ENTRY :
		wlog(LOGDEBUG,"Received CMD_SEND_ENTRY");
		if( entry_update(data) != -1 ) {
			screen_compose();
		}
		break;
	case CMD_SEND_POPUP :
		wlog(LOGDEBUG,"Received CMD_SEND_POPUP");
		ui_play_popup_add(data);
		screen_compose();
		break;
	default:
		werr(LOGDEV,"Unknown request from server");
		return RET_NOK;
		break;
	}

	return RET_OK;
}
