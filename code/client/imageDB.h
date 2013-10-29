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

void imageDB_init();
void imageDB_add_file(context_t * context, gchar * filename, GtkWidget * widget);
GtkWidget * imageDB_get_widget(context_t * context, const gchar * image_name);
GtkWidget * imageDB_check_widget(gchar * file_name);
gboolean image_DB_update(context_t * context,gchar * filename);
extern gboolean updated_media;
