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
#include "network_server.h"
#include <dirent.h>
#include <string.h>
#include "action.h"

/***************************************************************************
Add the specified value to the specified attribute
Check on max and min are done and call to on_* scripts are done if set
ctx is the context of the source of the attribute change request
id is the id of the target of the change
return -1 if fails
***************************************************************************/
int attribute_change(context_t * context, const char * id, const char * attribute, int value)
{
	int current;
	int old;
	int min;
	int max;
	char buf[SMALL_BUF];
	int do_min_action = FALSE;
	int do_down_action = FALSE;
	int do_max_action = FALSE;
	int do_up_action = FALSE;
	char * action;
	char * min_action = NULL;
	char * down_action = NULL;
	char * max_action = NULL;
	char * up_action = NULL;

	SDL_LockMutex(attribute_mutex);

	if(!entry_read_int(CHARACTER_TABLE,id,&current,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_CURRENT, NULL)) {
		SDL_UnlockMutex(attribute_mutex);
		return -1;
	}

	if(!entry_read_int(CHARACTER_TABLE,id,&min,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_MIN, NULL)) {
		min = -1;
	}

	if(!entry_read_int(CHARACTER_TABLE,id,&max,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_MAX, NULL)) {
		max = -1;
	}

	old = current;
	current = current + value;
	if( min != -1 ) {
		if(current <= min) {
			do_min_action = TRUE;
			current = min;
		}
	}

	if( max != -1 ) {
		if(current >= max) {
			do_max_action = TRUE;
			current = max;
		}
	}

	if(!entry_write_int(CHARACTER_TABLE,id,current,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_CURRENT, NULL)) {
		SDL_UnlockMutex(attribute_mutex);
		return -1;
	}
	if(!entry_write_int(CHARACTER_TABLE,id,old,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_PREVIOUS, NULL)) {
		SDL_UnlockMutex(attribute_mutex);
		return -1;
	}

	/* Check automatic actions */
	if( value < 0 ) {
		if( do_min_action ) {
			if(!entry_read_string(CHARACTER_TABLE,id,&action,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_ON_MIN, NULL)) {
				do_min_action = FALSE;
			} else {
				min_action = action;
			}
		}

		if(entry_read_string(CHARACTER_TABLE,id,&action,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_ON_DOWN, NULL)) {
			do_down_action = TRUE;
			down_action = action;
		}
	}

	if( value > 0 ) {
		if( do_max_action ) {
			if(!entry_read_string(CHARACTER_TABLE,id,&action,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_ON_MAX, NULL)) {
				do_max_action = FALSE;
			} else {
				max_action = action;
			}
		}

		if(entry_read_string(CHARACTER_TABLE,id,&action,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_ON_UP, NULL)) {
			do_up_action = TRUE;
			up_action = action;
		}
	}

	SDL_UnlockMutex(attribute_mutex);

	/* do automatic actions */
	if( do_down_action && down_action) {
		action_execute_script(context,down_action,NULL);
	}
	if( down_action ) {
		free(down_action);
	}

	if( do_min_action && min_action) {
		action_execute_script(context,min_action,NULL);
	}
	if( min_action ) {
		free(min_action);
	}

	if( do_up_action && up_action) {
		action_execute_script(context,up_action,NULL);
	}
	if( min_action ) {
		free(up_action);
	}

	if( do_max_action && max_action) {
		action_execute_script(context,max_action,NULL);
		free(max_action);
	}
	if( max_action ) {
		free(max_action);
	}

	sprintf(buf,"%s.%s.%s",ATTRIBUTE_GROUP,attribute,ATTRIBUTE_CURRENT);
	network_broadcast_entry_int(CHARACTER_TABLE,id,buf,current,TRUE);
	sprintf(buf,"%s.%s.%s",ATTRIBUTE_GROUP,attribute,ATTRIBUTE_PREVIOUS);
	network_broadcast_entry_int(CHARACTER_TABLE,id,buf,old,TRUE);
	return 0;
}

/****************************************
get the specified attribute's value
return -1 if fails
****************************************/
int attribute_get(const char *id, const char * attribute)
{
	int current;
	context_t * context = context_find(id);
	if( context == NULL ) {
		werr(LOGDEV,"%s: Could not find context %s",id);
		return -1;
	}

	SDL_LockMutex(attribute_mutex);

	if(!entry_read_int(CHARACTER_TABLE,id,&current,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_CURRENT, NULL)) {
		SDL_UnlockMutex(attribute_mutex);
		return -1;
	}

	SDL_UnlockMutex(attribute_mutex);

	return current;
}

/*********************************************************************
set the specified attribute's value without check of min and max
return -1 if fails
*********************************************************************/
int attribute_set(const char * id, const char * attribute, int value)
{
	context_t * context = context_find(id);
	if( context == NULL ) {
		werr(LOGDEV,"Could not find context %s",id);
		return -1;
	}

	SDL_LockMutex(attribute_mutex);

	if(!entry_write_int(CHARACTER_TABLE,id,value,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_CURRENT, NULL)) {
		SDL_UnlockMutex(attribute_mutex);
		return -1;
	}

	SDL_UnlockMutex(attribute_mutex);

	return 0;
}
