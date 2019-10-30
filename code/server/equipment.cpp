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
#include "Context.h"
#include "entry.h"
#include "syntax.h"
#include "log.h"
#include <string.h>

#include "network_server.h"

/**********************************************************************
 Set the passed item to the character's equipment slot
 return -1 if fails
 **********************************************************************/
int equipment_set_item(const char *id, const char * slot, const char * item)
{
	Context * context = context_find(id);
	if (context == nullptr)
	{
		werr(LOGDESIGNER, "Could not find context %s", id);
		return -1;
	}

	if (entry_write_string(CHARACTER_TABLE, context->getId().c_str(), item, EQUIPMENT_GROUP, slot, EQUIPMENT_EQUIPPED, nullptr) == RET_OK)
	{
		// update client
		network_send_character_file(context);
		return 0;
	}

	return -1;
}

/***********************************************************************
 Return the name of the item in character's specified equipment slot
 Returned string MUST BE FREED
 return nullptr if fails
 ***********************************************************************/
char * equipment_get_item(const char *id, const char * slot)
{
	char * item;
	Context * context = context_find(id);
	if (context == nullptr)
	{
		werr(LOGDESIGNER, "Could not find context %s", id);
		return nullptr;
	}

	if (entry_read_string(CHARACTER_TABLE, context->getId().c_str(), &item, EQUIPMENT_GROUP, slot, EQUIPMENT_EQUIPPED, nullptr) == RET_NOK)
	{
		return nullptr;
	}

	return item;
}
