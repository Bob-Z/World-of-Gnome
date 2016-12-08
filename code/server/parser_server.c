/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2016 carabobz@gmail.com

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

#include "../common/common.h"
#include "character.h"
#include "action.h"
#include <string.h>

/**************************************
Return FALSE on error, TRUE if OK
**************************************/
int parse_incoming_data(context_t * context, Uint32 command, Uint32 command_size, char * data)
{
	char * value = NULL;
	char * fullname;
	char * elements[512];
	char * cksum;
	int i;
	char * user_name;
	char * password;

	if( !context_get_connected(context) && ( command != CMD_REQ_LOGIN  && command != CMD_REQ_FILE) ) {
		werr(LOGUSER,"Request from not authenticated client, close connection");
		return FALSE;
	}

	switch(command) {
	case CMD_REQ_LOGIN:
		wlog(LOGDEBUG,"Received CMD_REQ_LOGIN");
		user_name = _strsep(&data,NETWORK_DELIMITER);
		password = _strsep(&data,NETWORK_DELIMITER);

		if(!entry_read_string(PASSWD_TABLE, user_name, &value, PASSWD_KEY_PASSWORD,NULL)) {
			return FALSE;
		}
		if( strcmp(value, password) != 0) {
			free(value);
			werr(LOGUSER,"Wrong login for %s",user_name);
			// send answer
			network_send_command(context, CMD_SEND_LOGIN_NOK, 0, NULL, false);
			// force client disconnection
			return FALSE;
		} else {
			free(value);

			if( context_set_username(context, user_name) == RET_NOK ) {
				return FALSE;
			}
			context_set_connected(context, true);

			// send answer
			network_send_command(context, CMD_SEND_LOGIN_OK, 0, NULL, false);
			wlog(LOGUSER,"Login successful for user %s",context->user_name);
		}
		break;
	case CMD_REQ_CHARACTER_LIST :
		wlog(LOGDEBUG,"Received CMD_REQ_CHARACTER_LIST");
		character_send_list(context);
		wlog(LOGDEBUG,"character list sent");
		break;
	case CMD_REQ_FILE :
		i = 0;
		elements[i] = _strsep(&data,NETWORK_DELIMITER);
		while(elements[i]) {
			i++;
			elements[i] = _strsep(&data,NETWORK_DELIMITER);
		}

		if(elements[0]==NULL || elements[1]==NULL) {
			werr(LOGDEV,"Received erroneous CMD_REQ_FILE");
			break;
		}
		wlog(LOGDEBUG,"Received CMD_REQ_FILE for %s",elements[0]);
		/* compare checksum */
		fullname = strconcat(base_directory,"/",elements[0],NULL);

		cksum = checksum_file(fullname);
		free(fullname);

		if( cksum == NULL) {
			werr(LOGUSER,"Required file %s doesn't exists",elements[0]);
			break;
		}

		if( strcmp(elements[1],cksum) == 0 ) {
			wlog(LOGDEBUG,"Client has already newest %s file",elements[0]);
			free(cksum);
			break;
		}
		free(cksum);

		network_send_file(context,elements[0]);
		wlog(LOGDEBUG,"File %s sent",elements[0]);
		break;
	case CMD_REQ_USER_CHARACTER_LIST :
		wlog(LOGDEBUG,"Received CMD_REQ_USER_CHARACTER_LIST");
		character_user_send_list(context);
		wlog(LOGDEBUG,"user %s's character list sent",context->user_name);
		break;
	case CMD_REQ_START :
		if( context->in_game == false ) {
			context->id = strdup(data);
			context->in_game = true;
			context_update_from_file(context);
			context_spread(context);
			context_request_other_context(context);
		}
		wlog(LOGDEBUG,"Received CMD_REQ_START for %s /%s",context->user_name,context->id);
		break;
	case CMD_REQ_STOP :
		wlog(LOGDEBUG,"Received CMD_REQ_STOP for %s /%s",context->user_name,context->id);
		if( context->in_game == true ) {
			context->in_game = false;
			if( context->map ) {
				free(context->map);
			}
			context->map = NULL;
			if( context->prev_map ) {
				free(context->prev_map);
			}
			context->prev_map = NULL;
			if( context->id ) {
				free(context->id);
			}
			context->id = NULL;
			context_spread(context);
		}
		break;
	case CMD_REQ_ACTION :
		i = 0;
		elements[i] = NULL;
		elements[i] = _strsep(&data,NETWORK_DELIMITER);
		while(elements[i]) {
			i++;
			elements[i] = _strsep(&data,NETWORK_DELIMITER);
		}
		elements[i+1] = NULL;

		wlog(LOGDEBUG,"Received CMD_REQ_ACTION %s from %s /%s",elements[0],context->user_name,context->character_name);

		action_execute(context,elements[0],&elements[1]);
		break;
	default:
		werr(LOGDEV,"Unknown request %d from client",command);
		return FALSE;
	}

	return TRUE;
}
