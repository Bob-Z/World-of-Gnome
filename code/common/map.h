/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013 carabobz@gmail.com

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

#ifndef MAP_H
#define MAP_H

gchar * map_new(gint x,gint y, gint tile_x, gint tile_y, gchar * default_tile);
gchar * map_delete_item(const gchar * map, gint x, gint y);
gint map_add_item(const gchar * map, const gchar * item, gint x, gint y);
gboolean map_check_tile(context_t * context,gchar * map,gint x,gint y);
int map_set_tile(const gchar * map,const gchar * tile,gint x, gint y);
gchar * map_get_tile(const gchar * map,gint x, gint y);
const gchar ** map_get_event(const gchar * map,gint x, gint y);
gchar * map_add_event(const gchar * map, const gchar * script, gint x, gint y);
gint map_delete_event(const gchar * map, const gchar * script, gint x, gint y);
#endif

