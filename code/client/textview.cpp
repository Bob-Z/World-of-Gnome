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
#include "sdl.h"
#include "textview.h"

history_entry_t * text_history = NULL;

void textview_add_line(char * text)
{
	history_entry_t * new_entry;

	new_entry = (history_entry_t*)malloc(sizeof(history_entry_t));

	new_entry->text = strdup(text);
	new_entry->time = SDL_GetTicks();
	new_entry->next = text_history;

	text_history = new_entry;
}

const history_entry_t * textview_get_history()
{
	return text_history;
}
