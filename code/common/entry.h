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

#ifndef ENTRY_H
#define ENTRY_H

#include <glib.h>

gchar * file_new(gchar * table);

int read_int(const gchar * table, const gchar * file, int * res, ...);
int read_string(const gchar * table, const gchar * file, const gchar ** res, ...);
int _read_int(const gchar * table, const gchar * file, int * res, ...);
int _read_string(const gchar * table, const gchar * file, const gchar ** res, ...);
int read_list_index(const gchar * table, const gchar * file, const gchar ** res,gint index, ...);
int read_list(const gchar * table, const gchar * file, gchar *** res, ...);
int _read_list(const gchar * table, const gchar * file, gchar *** res, ...);

int write_int(const gchar * table, const gchar * file, int data, ...);
int write_string(const gchar * table, const gchar * file,const char * data, ...);
int write_list_index(const gchar * table, const gchar * file, const char * data,gint index, ...);
int write_list(const gchar * table, const gchar * file, char ** data, ...);

gchar * get_unused_list_entry(const gchar * table, const gchar * file, ...);
gchar * get_unused_group(const gchar * table, const gchar * file, ...);
int get_group_list(const gchar * table, const gchar * file, gchar *** res, ...);

gboolean file_get_contents(const gchar *filename,gchar **contents,gsize *length,GError **error);
gboolean file_set_contents(const gchar *filename,const gchar *contents,gssize length,GError **error);

gboolean add_to_list(const gchar * table, const gchar * file, const gchar * to_be_added, ...);
int remove_group(const gchar * table, const gchar * file, const gchar * group, ...);

void file_dump_all_to_disk(void);

gboolean remove_from_list(const gchar * table, const gchar * file, const gchar * to_be_removed, ...);
int list_create(const gchar * table, const gchar * file, ...);
int group_create(const gchar * table, const gchar * file, ...);
gchar * copy_group(const gchar * src_table, const gchar * src_file, const gchar * dst_table, const gchar * dst_file, const gchar * group_name, ...);
gint entry_update(gchar * data);
gint entry_destroy(const gchar * id);
#endif
