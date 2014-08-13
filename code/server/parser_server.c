/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2014 carabobz@gmail.com

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
	char *saveptr1;
	char *saveptr2;

	if( !context_get_connected(context) && command != CMD_LOGIN_USER && command != CMD_LOGIN_PASSWORD ) {
		werr(LOGUSER,"Request from not authenticated client, close connection");
		return FALSE;
	}

	switch(command) {
	case CMD_LOGIN_USER :
		wlog(LOGDEBUG,"Received CMD_LOGIN_USER");
		if( !context_set_username(context, data) ) {
			return FALSE;
		}
		wlog(LOGDEBUG,"Successfully set username to %s",data);
		break;
	case CMD_LOGIN_PASSWORD :
		wlog(LOGDEBUG,"Received CMD_LOGIN_PASSWORD");
		/* Read username / password pairs from file : FILE_USER_CONF */
		/* Read password for username */
		if(!entry_read_string(PASSWD_TABLE, context->user_name, &value, PASSWD_KEY_PASSWORD,NULL)) {
			return FALSE;
		}
		if( strcmp(value, data) != 0) {
			free(value);
			werr(LOGUSER,"Wrong login for %s",context->user_name);
			/* send answer */
			network_send_command(context, CMD_LOGIN_NOK, 0, NULL, FALSE);
			/* force client disconnection*/
			return FALSE;
		} else {
			free(value);
			/* send answer */
			network_send_command(context, CMD_LOGIN_OK, 0, NULL, FALSE);
			wlog(LOGUSER,"%s successfully login",context->user_name);
			context_set_connected(context, TRUE);
		}
		break;
	case CMD_REQ_CHARACTER_LIST :
		wlog(LOGDEBUG,"Received CMD_REQ_CHARACTER_LIST");
		character_send_list(context);
		wlog(LOGDEBUG,"character list sent");
		break;
	case CMD_REQ_FILE :
		i = 0;
		elements[i] = strtok_r(data,NETWORK_DELIMITER,&saveptr1);
		while(elements[i]) {
			i++;
			elements[i] = strtok_r(NULL,NETWORK_DELIMITER,&saveptr1);
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
	case CMD_SEND_CONTEXT :
		if( context->type == NULL ) { /* First time a context send its data */
			context_update_from_network_frame(context,data);
			context_spread(context);
			context_request_other_context(context);
		} else {
			context_update_from_network_frame(context,data);
		}
		context_write_to_file(context);
		wlog(LOGDEBUG,"Received CMD_SEND_CONTEXT for %s /%s",context->user_name,context->character_name);
		break;
	case CMD_SEND_ACTION :
		i = 0;
		elements[i] = strtok_r(data,NETWORK_DELIMITER,&saveptr2);
		while(elements[i]) {
			i++;
			elements[i] = strtok_r(NULL,NETWORK_DELIMITER,&saveptr2);
		}

		wlog(LOGDEBUG,"Received CMD_SEND_ACTION %s from %s /%s",elements[0],context->user_name,context->character_name);

		action_execute_script(context,elements[0],&elements[1]);
		break;
	default:
		werr(LOGDEV,"Unknown request %d from client",command);
		return FALSE;
	}

	return TRUE;
}
