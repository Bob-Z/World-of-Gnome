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

#ifndef ITEM_H
#define ITEM_H

char * item_create_empty();
char * item_create_from_template(const char * template);
int item_destroy(const char * item_id);
char * item_resource_new(const char * template, int quantity);
const char * item_is_resource(const char * item_id);
int item_get_quantity(const char * item_id);
int item_set_quantity(const char * item_id, int quantity);
const char * item_get_name(const char * item_id);

#endif