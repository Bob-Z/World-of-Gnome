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

#include <gtk/gtk.h>
#include "config.h"
#include "../common/common.h"
#include "win_game.h"
#include <glib.h>
#include <glib/gstdio.h>
#include "imageDB.h"
#include "file.h"
#include "anim.h"
#include "item.h"

typedef struct {
	char * id;
	char * name;
	char * type;
	anim_t * marquee;
} character_t;

static pthread_mutex_t character_mutex = PTHREAD_MUTEX_INITIALIZER;
static character_t * character_list = NULL;
static int character_num = 0;
static item_t * item_list = NULL;

/**********************************
Compose the character select screen
**********************************/
item_t * scr_select_compose(context_t * context)
{
	const gchar * marquee_name;
	int i = 0;
	anim_t * anim;

	wlog(LOGDEBUG,"Composing select character screen\n");

	if(item_list) {
		free(item_list);
	}

	pthread_mutex_lock(&character_mutex);
	item_list = malloc(character_num*sizeof(item_t));

	for(i=0;i<character_num;i++) {
		item_init(&item_list[i]);
		item_set_string(&item_list[i],character_list[i].name);

		/* Compute the marquee file name */
		if(!read_string(CHARACTER_TABLE,character_list[i].id,&marquee_name,CHARACTER_KEY_MARQUEE,NULL)) {
			continue;
		}
		anim  = imageDB_get_anim(context,marquee_name);
		item_set_anim(&item_list[i],0,0,anim);
	}
	item_set_last(&item_list[i-1],1);

	pthread_mutex_unlock(&character_mutex);

	return item_list;
}

/*************************
 add a character to the list
the data is a list a 3 strings, the first string is the id of the character (its file name) the second one is the type of the character, the third is the name of the character.
the list ends with an empty string
*************************/
void scr_select_add_user_character(context_t * context, gchar * data)
{
	gchar * current_string = data;

	pthread_mutex_lock(&character_mutex);

	character_num++;

	character_list = realloc(character_list,sizeof(character_t)*character_num);

	character_list[character_num-1].id = strdup(current_string);
	current_string += strlen(current_string)+1;
	character_list[character_num-1].type = strdup(current_string);
	current_string += strlen(current_string)+1;
	character_list[character_num-1].name = strdup(current_string);
	character_list[character_num-1].marquee = NULL;

	pthread_mutex_unlock(&character_mutex);

	wlog(LOGDEV,"Received character %s of type %s\n",character_list[character_num-1].name,character_list[character_num-1].type);
}

/*************************
 play button clicked
*************************/
void on_play_clicked (GtkButton *button, gpointer   user_data)
{
	GtkTreeSelection *sel;
	GtkTreeModel*   model;
	GtkTreeIter     iter;
	gchar * id;
	gchar * name;

	context_t * context = (context_t *)user_data;

	/* get selected character */
	sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(character_list));
	/* if no selection */
	if( ! gtk_tree_selection_count_selected_rows( sel ) ) {
		return;
	}
	gtk_tree_selection_get_selected(sel,&model,&iter);
	gtk_tree_model_get (model,&iter,2,&id, -1);
	context_set_id(context, id);
	g_free(id);

	gtk_tree_selection_get_selected(sel,&model,&iter);
	gtk_tree_model_get (model,&iter,0,&name, -1);
	context_set_character_name(context, name);
	g_free(name);

	file_clean(context);
#if 0
	hide_select_character_window();
	show_game_window(context);
#endif
}

/****************
return FALSE on error
****************/

gboolean win_select_character_init(context_t * context)
{
	GtkBuilder * builder = NULL;
	builder = gtk_builder_new ();

	gdk_threads_enter ();
	gchar tmp[SMALL_BUF];
//	g_snprintf(tmp,SMALL_BUF,"./select_character.glade",DATADIR);
	g_snprintf(tmp,SMALL_BUF,"./select_character.glade");
	if( ! gtk_builder_add_from_file (builder, tmp,NULL)) {
		g_snprintf(tmp,SMALL_BUF,"%s/wog/client/select_character.glade",DATADIR);
		if( ! gtk_builder_add_from_file (builder, tmp,NULL)) {
			g_print("Error while loading UI file select_character.glade\n");
			return FALSE;
		}
	}

#if 0
	select_character_window = GTK_WIDGET(gtk_builder_get_object(builder,"select_character"));
	character_list   = GTK_WIDGET(gtk_builder_get_object(builder,"character_list"));
#endif

	gtk_builder_connect_signals (builder, context);
	g_object_unref (G_OBJECT (builder));

#if 0
	gtk_widget_hide_all(select_character_window);
	gtk_widget_realize(select_character_window);
	gdk_threads_leave ();
#endif

	return TRUE;
}

