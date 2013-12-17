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
#include "sdl.h"
#include "screen.h"

#define BORDER 20
#define FONT "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-C.ttf"
#define FONT_SIZE 30

typedef struct {
	char * id;
	char * name;
	char * type;
	anim_t * anim;
} character_t;

static pthread_mutex_t character_mutex = PTHREAD_MUTEX_INITIALIZER;
static character_t * character_list = NULL;
static int character_num = 0;
static item_t * item_list = NULL;
static character_t * current_character = NULL;

static void cb_left_click(void * arg)
{
	item_t * item = (item_t*)arg;

	sdl_set_virtual_x(item->rect.x + item->rect.w/2);
	sdl_set_virtual_y(item->rect.y + item->rect.h/2);
}

static void cb_right_click(void * arg)
{
	context_t * ctx = (context_t*)arg;

	ctx->id = strdup(current_character->id);

	file_clean(ctx);

	screen_set_screen(SCREEN_PLAY);

	screen_compose();
}

static void cb_over(void * arg)
{
	character_t * c = (character_t*)arg;

	current_character = c;
}

/**********************************
Compose the character select screen
**********************************/
item_t * scr_select_compose(context_t * context)
{
	int i = 0;
	int x = 0;
	const char * marquee_name;
	static int max_h = 0;
	static int init = 1;
	item_t * item;
	item_t * item_image;

	wlog(LOGDEBUG,"Composing select character screen");

	if(character_num==0) {
		return NULL;
	}

	if(item_list) {
		item_list_free(item_list);
		item_list = NULL;
	}

	pthread_mutex_lock(&character_mutex);

	wlog(LOGDEBUG,"Composing %d characters",character_num);

	/* Load all anim and compute the max height */
	for(i=0;i<character_num;i++) {
		/* Compute the marquee file name */
		if(!read_string(CHARACTER_TABLE,character_list[i].id,&marquee_name,CHARACTER_KEY_MARQUEE,NULL)) {
			continue;
		}
		character_list[i].anim  = imageDB_get_anim(context,marquee_name);
		if(character_list[i].anim->h > max_h) {
			max_h = character_list[i].anim->h;
		}
	}

	/* Create item list */
	for(i=0;i<character_num;i++) {
		if( character_list[i].anim == NULL ) {
			continue;
		}

		/* Character picture */
		item = item_list_add(item_list);
		item_image = item;
		if(item_list == NULL) {
			item_list = item;
		}
		item_set_anim(item,x,max_h/2-character_list[i].anim->h/2,character_list[i].anim);
		item_set_click_left(item,cb_left_click,(void *)item);
		item_set_click_right(item,cb_right_click,(void *)context);
		item_set_over(item,cb_over,(void *)&character_list[i]);

		x += character_list[i].anim->w + BORDER;
		/* character name */
		item = item_list_add(item_list);
		item_set_string(item,character_list[i].name);
		item_set_font(item,TTF_OpenFont(FONT, FONT_SIZE));
		/* display string just above the picture */
		item_set_frame(item,item_image->rect.x + item_image->rect.w/2, item_image->rect.y-FONT_SIZE/2,NULL);

		/* character type */
		item = item_list_add(item_list);
		item_set_string(item,character_list[i].type);
		item_set_font(item,TTF_OpenFont(FONT, FONT_SIZE));
		/* display string just below the picture */
		item_set_frame(item,item_image->rect.x + item_image->rect.w/2, item_image->rect.y+item_image->rect.h+FONT_SIZE/2,NULL);
	}

	if(init && item_list) {
		cb_left_click(&item_list[0]);
		init = 0;
	}

	pthread_mutex_unlock(&character_mutex);

	return item_list;
}

/*************************
 add a character to the list
the data is a list a 3 strings, the first string is the id of the character (its file name) the second one is the type of the character, the third is the name of the character.
the list ends with an empty string
*************************/
void scr_select_add_user_character(context_t * context, char * data)
{
	char * current_string = data;

	pthread_mutex_lock(&character_mutex);

	while(current_string[0] != 0) {

		character_num++;

		character_list = realloc(character_list,sizeof(character_t)*character_num);

		character_list[character_num-1].id = strdup(current_string);
		current_string += strlen(current_string)+1;
		character_list[character_num-1].type = strdup(current_string);
		current_string += strlen(current_string)+1;
		character_list[character_num-1].name = strdup(current_string);
		current_string += strlen(current_string)+1;
		character_list[character_num-1].anim = NULL;

		wlog(LOGDEBUG,"Character %s / %s /%s added",character_list[character_num-1].id,character_list[character_num-1].type,character_list[character_num-1].name);
	}

	pthread_mutex_unlock(&character_mutex);

	wlog(LOGDEV,"Received character %s of type %s",character_list[character_num-1].name,character_list[character_num-1].type);
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

