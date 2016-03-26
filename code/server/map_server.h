/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2016 carabobz@gmail.com

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

#ifndef MAP_SERVER_H
#define MAP_SERVER_H

int map_check_tile(context_t * ctx,char * id,const char * map,int layer, int x,int y);
char * map_new(const char * name, int w, int h, int tile_w, int tile_h);
char * map_delete_item(const char * map, int layer, int x, int y);
int map_add_item(const char * map, int layer, const char * item, int x, int y);
int map_check_tile(context_t * ctx,char * id,const char * map,int layer, int x,int y);
int map_set_tile(const char * map,int layer, const char * tile,int x, int y,int network_broadcast);
int map_set_tile_array(const char * map, int layer,const char** tile_array);
int map_set_tile_type(const char * map,int layer, const char * type,int x, int y,int network_broadcast);
void map_broadcast(const char * map);
int map_set_offscreen(const char * map, const char * script);
int map_set_custom_column(const char * map, int layer, int num, int width, int height);
int map_set_custom_row(const char * map, int layer, int num, int width, int height);
char * map_get_tile(const char * map,int layer, int x, int y);
char * map_get_tile_type(const char * map,int layer, int x, int y);
char ** map_get_event(const char * map,int layer, int x, int y);
char ** map_get_character(const char * map, int x, int y);
char ** map_get_item(const char * map,int layer, int x, int y);
char * map_add_event(const char * map, int layer, const char * script, int x, int y);
int map_add_event_param(const char * map, int layer, const char * event_id, const char * param);
int map_delete_event(const char * map, int layer, const char * script, int x, int y);
int map_get_tile_coord(const char * map, int layer, int x, int y, int * tx, int * ty);
char * map_add_scenery(const char * map, int layer, int x, int y, const char * image_name);
int map_add_layer(const char * map_name, int layer, int w, int h, int tile_w, int tile_h, const char * default_tile,const char * default_type);
int map_delete_layer(const char * map_name,int layer);
#endif
