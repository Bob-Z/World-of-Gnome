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

#ifndef MAP_H
#define MAP_H
char * map_new(const char * suggested_name,int w,int h, int tile_w, int tile_h, const char * default_tile,const char * default_type);
char * map_delete_item(const char * map, int x, int y);
int map_add_item(const char * map, const char * item, int x, int y);
int map_check_tile(context_t * ctx,char * id,const char * map,int x,int y);
int map_set_tile(const char * map,const char * tile,int x, int y, int level);
int map_set_tile_type(const char * map,const char * type,int x, int y);
char * map_get_tile(const char * map,int x, int y, int level);
char * map_get_tile_type(const char * map,int x, int y);
char ** map_get_event(const char * map,int x, int y);
char ** map_get_character(const char * map,int x, int y);
char ** map_get_item(const char * map,int x, int y);
char * map_add_event(const char * map, const char * script, int x, int y);
int map_add_event_param(const char * map, const char * event_id, const char * param);
int map_delete_event(const char * map, const char * script, int x, int y);
#endif
