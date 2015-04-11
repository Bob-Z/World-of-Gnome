/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2015 carabobz@gmail.com

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

int attribute_change(context_t * context, const char * table, const char * id, const char * attribute, int value);
int attribute_get(const char * table, const char * id, const char * attribute);
int attribute_set(const char * table, const char * id, const char * attribute, int value);
char * attribute_tag_get(const char * table, const char * id, const char * attribute);
int attribute_tag_set(const char * table, const char * id, const char * attribute, const char * value);

