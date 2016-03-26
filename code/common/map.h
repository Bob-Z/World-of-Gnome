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

#ifndef MAP_H
#define MAP_H

#define DEFAULT_LAYER      (-1)

// Max number of consecutive tiles with custom alignement
#define MAX_COL         16
#define MAX_ROW         16

typedef struct layer {
        int active;
        int tile_width;
        int tile_height;
        int map_w;
        int map_h;
        double map_zoom;
        int col_width[MAX_COL];
        int col_height[MAX_COL];
        int col_num;
        int col_width_total;
        int col_height_total;
        int row_width[MAX_ROW];
        int row_height[MAX_ROW];
        int row_num;
        int row_width_total;
        int row_height_total;
} layer_t;

layer_t * map_layer_new(const char * map, int layer_index, layer_t * default_layer);
void map_layer_delete(layer_t * default_layer);
int map_t2p_x(int x, int y,layer_t * layer);
int map_t2p_y(int x, int y,layer_t * layer);
#endif
