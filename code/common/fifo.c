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

#include "common.h"

/****************************************
*****************************************/
void * fifo_pop(fifo_t ** fifo)
{
	void * res;
	fifo_t * f;

	if( *fifo == NULL ) {
		return NULL;
	}

	res = (*fifo)->data;

	f = *fifo;
	*fifo = (*fifo)->next;
	free(f);

	return res;
}
/****************************************
*****************************************/
void fifo_push(fifo_t ** fifo, void * data)
{
	fifo_t * f;

	if(*fifo == NULL) {
		*fifo = malloc(sizeof(fifo_t));
		(*fifo)->data = data;
		(*fifo)->next = NULL;
		return;
	}

	f = *fifo;
	while( f->next != NULL ) {
		f = f->next;
	}

	f->next = malloc(sizeof(fifo_t));
	f = f->next;
	f->data = data;
	f->next = NULL;
}
