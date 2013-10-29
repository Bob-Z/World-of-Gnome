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

#ifndef LOG_C
#define LOG_C

#include <glib.h>
#include <glib/gprintf.h>

static gint level;

void my_log_handler(const gchar *log_domain,
                    GLogLevelFlags log_level,
                    const gchar *message,
                    gpointer user_data) {

	if( level == 0 ) {
		if( log_level <= G_LOG_LEVEL_CRITICAL ) {
//			g_log_default_handler(log_domain,log_level,message,user_data);
			printf("%04X | %s\n",log_level,message);
		}
		return;
	}
	if( level == 1 ) {
		if( log_level <=  G_LOG_LEVEL_WARNING ) {
//			g_log_default_handler(log_domain,log_level,message,user_data);
			printf("%04X | %s\n",log_level,message);
		}
		return;
	}

	if( level == 2 ) {
		if( log_level <=  G_LOG_LEVEL_MESSAGE ) {
//			g_log_default_handler(log_domain,log_level,message,user_data);
			printf("%04X | %s\n",log_level,message);
		}
		return;
	}

	if( level >= 3 ) {
		if( log_level <=  G_LOG_LEVEL_DEBUG ) {
//			g_log_default_handler(log_domain,log_level,message,user_data);
			printf("%04X | %s\n",log_level,message);
		}
		return;
	}

	g_printf("LOG ERROR\n");
}

void init_log(gint log_level)
{
	level = log_level;
	g_log_set_handler (NULL, G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL
                   | G_LOG_FLAG_RECURSION, my_log_handler, NULL);
}
#endif
