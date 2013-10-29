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

#include <glib.h>
#include <gio/gio.h>
#include <common.h>


/* Return a string representing the checksum of the file or NULL on error */
/* filename is the directory + name */
/* The returned string MUST be FREED */

gchar * checksum_file(const gchar * filename)
{
        gchar * file_data = NULL;
        gsize file_length = 0;
        gboolean res = file_get_contents(filename,&file_data,&file_length,NULL);

        if( res == FALSE) {
                return NULL;
        }

	gchar * checksum = NULL;

	checksum = g_compute_checksum_for_data ( G_CHECKSUM_MD5,(guchar *)file_data,file_length);

	g_free(file_data);

	return checksum;
}
