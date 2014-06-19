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

#include <gtk/gtk.h>

extern gint main_window_width;
extern gint main_window_height;

void draw_sprite(context_t * context, GtkWidget * tile_set, gint tile_x, gint tile_y);
void draw_all_sprite(GtkWidget * tile_set);
void draw_cleanup(context_t * context);
void draw_map(context_t * context, GtkWidget * tile_set);
void draw_item(context_t * context, GtkWidget * tile_set);
void draw_cursor(context_t * context, GtkWidget * tile_set);
void update_selected_character(context_t * context);
