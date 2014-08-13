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

/*****************************************************************
Delete the selected item from the character's equipment slot
return -1 if fails
*****************************************************************/
int equipment_delete(const char *id, const char * slot)
{
	context_t * context = context_find(id);
	if( context == NULL ) {
		werr(LOGDEV,"Could not find context %s",id);
		return -1;
	}

	if( remove_group(CHARACTER_TABLE, context->id, EQUIPMENT_EQUIPPED, EQUIPMENT_GROUP, slot, NULL)) {
		/* update client */
		network_send_character_file(context);
		return 0;
	}

	return -1;
}

/**********************************************************************
Add the passed item to the character's equipment slot
return -1 if fails
**********************************************************************/
int equipment_add(const char *id, const char * slot, const char * item)
{
	context_t * context = context_find(id);
	if( context == NULL ) {
		werr(LOGDEV,"Could not find context %s",id);
		return -1;
	}

	if( write_string(CHARACTER_TABLE, context->id, item, EQUIPMENT_GROUP, slot, EQUIPMENT_EQUIPPED, NULL)) {
		/* update client */
		network_send_character_file(context);
		return 0;
	}

	return -1;
}

/***********************************************************************
Return the name of the item in character's specified equipment slot
Returned string MUST BE FREED
return NULL if fails
***********************************************************************/
char * equipment_get_item_id(const char *id, const char * slot)
{
	char * item;
	context_t * context = context_find(id);
	if( context == NULL ) {
		werr(LOGDEV,"Could not find context %s",id);
		return NULL;
	}

	if(!entry_read_string(CHARACTER_TABLE, context->id, &item, EQUIPMENT_GROUP, slot, EQUIPMENT_EQUIPPED, NULL)) {
		return NULL;
	}

	return item;
}
