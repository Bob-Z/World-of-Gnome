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
#include <glib/gprintf.h>
#include <gio/gio.h>
#include "../common/common.h"
#include <dirent.h>
#include <string.h>
#include "action.h"

static GStaticMutex attribute_mutex = G_STATIC_MUTEX_INIT;


/*****************************/
/* Add the specified value to the specified attribute */
/* Check on max and min are done and call to on_* scripts are done if set */
/* ctx is the context of the source of the attribute change request */
/* id is the id of the target of the change */
/* return -1 if fails */
gint attribute_change(context_t * context, const gchar * id, const gchar * attribute, gint value)
{
	gint current;
	gint old;
	gint min;
	gint max;
	gchar buf[SMALL_BUF];
	gboolean do_min_action = FALSE;
	gboolean do_down_action = FALSE;
	gboolean do_max_action = FALSE;
	gboolean do_up_action = FALSE;
	const gchar * action;
	gchar * min_action = NULL;
	gchar * down_action = NULL;
	gchar * max_action = NULL;
	gchar * up_action = NULL;

	g_static_mutex_lock(&attribute_mutex);

	if(!read_int(CHARACTER_TABLE,id,&current,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_CURRENT, NULL)) {
		g_static_mutex_unlock(&attribute_mutex);
		return -1;
	}

	if(!read_int(CHARACTER_TABLE,id,&min,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_MIN, NULL)) {
		min = -1;
	}

	if(!read_int(CHARACTER_TABLE,id,&max,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_MAX, NULL)) {
		max = -1;
	}

	old = current;
	current = current + value;
	if( min != -1 ) {
		if(current <= min) {
			do_min_action = TRUE;
			current = min;
		}
	}

	if( max != -1 ) {
		if(current >= max) {
			do_max_action = TRUE;
			current = max;
		}
	}

	if(!write_int(CHARACTER_TABLE,id,current,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_CURRENT, NULL)) {
		g_static_mutex_unlock(&attribute_mutex);
		return -1;
	}
	if(!write_int(CHARACTER_TABLE,id,old,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_PREVIOUS, NULL)) {
		g_static_mutex_unlock(&attribute_mutex);
		return -1;
	}

	/* Check automatic actions */
	if( value < 0 ) {
		if( do_min_action ) {
			if(!read_string(CHARACTER_TABLE,id,&action,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_ON_MIN, NULL)) {
				do_min_action = FALSE;
			} else {
				min_action = g_strdup(action);
			}
		}

		if(read_string(CHARACTER_TABLE,id,&action,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_ON_DOWN, NULL)) {
			do_down_action = TRUE;
			down_action = g_strdup(action);
		}
	}

	if( value > 0 ) {
		if( do_max_action ) {
			if(!read_string(CHARACTER_TABLE,id,&action,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_ON_MAX, NULL)) {
				do_max_action = FALSE;
			} else {
				max_action = g_strdup(action);
			}
		}

		if(read_string(CHARACTER_TABLE,id,&action,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_ON_UP, NULL)) {
			do_up_action = TRUE;
			up_action = g_strdup(action);
		}
	}

	g_static_mutex_unlock(&attribute_mutex);

	/* do automatic actions */
	if( do_down_action && down_action) {
		action_execute_script(context,down_action,NULL);
	}
	if( down_action ) {
		g_free(down_action);
	}

	if( do_min_action && min_action) {
		action_execute_script(context,min_action,NULL);
	}
	if( min_action ) {
		g_free(min_action);
	}

	if( do_up_action && up_action) {
		action_execute_script(context,up_action,NULL);
	}
	if( min_action ) {
		g_free(up_action);
	}

	if( do_max_action && max_action) {
		action_execute_script(context,max_action,NULL);
		g_free(max_action);
	}
	if( max_action ) {
		g_free(max_action);
	}

	g_sprintf(buf,"%s.%s.%s",ATTRIBUTE_GROUP,attribute,ATTRIBUTE_CURRENT);
	network_broadcast_entry_int(CHARACTER_TABLE,id,buf,current,TRUE);
	g_sprintf(buf,"%s.%s.%s",ATTRIBUTE_GROUP,attribute,ATTRIBUTE_PREVIOUS);
	network_broadcast_entry_int(CHARACTER_TABLE,id,buf,old,TRUE);
	return 0;
}

/*****************************/
/* get the specified attribute's value */
/* return -1 if fails */
gint attribute_get(const gchar *id, const gchar * attribute)
{
	gint current;
	context_t * context = context_find(id);
	if( context == NULL ) {
		werr(LOGDEV,"%s: Could not find context %s",id);
		return -1;
	}

	g_static_mutex_lock(&attribute_mutex);

	if(!read_int(CHARACTER_TABLE,id,&current,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_CURRENT, NULL)) {
		g_static_mutex_unlock(&attribute_mutex);
		return -1;
	}

	g_static_mutex_unlock(&attribute_mutex);

	return current;
}

/*****************************/
/* set the specified attribute's value without check of min and max */
/* return -1 if fails */
gint attribute_set(const gchar * id, const gchar * attribute, gint value)
{
	context_t * context = context_find(id);
	if( context == NULL ) {
		werr(LOGDEV,"Could not find context %s",id);
		return -1;
	}

	g_static_mutex_lock(&attribute_mutex);

	if(!write_int(CHARACTER_TABLE,id,value,ATTRIBUTE_GROUP,attribute, ATTRIBUTE_CURRENT, NULL)) {
		g_static_mutex_unlock(&attribute_mutex);
		return -1;
	}

	g_static_mutex_unlock(&attribute_mutex);

	return 0;
}
