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
#include "../common/common.h"
#include "config.h"
#include <glib.h>
#include <glib/gstdio.h>
#include <gdk/gdkkeysyms.h>
#include "draw.h"
#include "imageDB.h"
#include "action.h"
#include "inventory.h"
#include "equipment.h"

static GtkWidget *	game_window;
static GtkWidget *	tile_set;
static GtkWidget *	action_bar;
GtkWidget *		attribute_list;
GtkWidget *		inventory_list;
GtkWidget *		equipment_list;
GtkWidget *		selected_character_label;
GtkWidget *		selected_character_image;
GtkWidget *		selected_tile_label;
GtkWidget *		selected_tile_image;
GtkWidget *		pointed_label;
GtkWidget *		pointed_image;
GtkWidget *		textview;
GtkTextBuffer *		textbuffer;
GtkWidget *		textentry;

void redraw_window()
{
	wlog(LOGDEBUG,"redraw_window");

	gdk_threads_enter();
	gdk_window_invalidate_rect(game_window->window,NULL,TRUE);
	gdk_threads_leave();
}

void on_game_window_destroy(GtkObject *object,gpointer   user_data)
{
	context_t * ctx = (context_t *)user_data;
	g_io_stream_close((GIOStream *)context_get_connection(ctx),NULL,NULL);
	file_dump_all_to_disk();
	gtk_main_quit();
}

/***********************
update_attributes

Write attributes in the game window 
***********************/
void update_attributes(context_t * context)
{
	GtkListStore * attribute_list_store;
	GtkTreeIter     iter;
	gchar ** name_list;
	gint index = 0;
	gint value;

        attribute_list_store=GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(attribute_list)));
        gtk_list_store_clear(attribute_list_store);

	if(!get_group_list(CHARACTER_TABLE,context->id,&name_list,ATTRIBUTE_GROUP,NULL) ) {
		return;
	}

        while( name_list[index] != NULL) {
		if(!read_int(CHARACTER_TABLE,context->id,&value,ATTRIBUTE_GROUP,name_list[index],ATTRIBUTE_CURRENT,NULL)) {
			index++;
			continue;
		}

                gtk_list_store_append (attribute_list_store, &iter);
                gtk_list_store_set (attribute_list_store, &iter,
                                0, name_list[index],
                                1, value,
                                -1);

		index++;
        }

	g_free(name_list);
}

/************************
Update game windows
As win_game_update is called from on_* functions which already have the GDK lock,
you MUST always call win_game_update with the GDK locked (gdk_threads_enter).
And of course you may want to release the lock after the call (gdk_threads_leave).
************************/

void win_game_update(context_t * context)
{
	static gchar * old_map = NULL;
	static gboolean first_time = TRUE;

        /* Do nothing if not visible */
        if( gtk_widget_get_visible(game_window) == FALSE ) {
                return;
        }


	if( context->type == NULL ) {
		if ( context_update_from_file(context) == FALSE ) {
			g_usleep(500*1000);
			return;
		}
	}

	/* Upload the map from the server if needed */
	if( context->map != old_map ) {
		old_map = context->map;
	}

//	gdk_window_freeze_updates(tile_set->window); /* to avoid flickering */

	/* update attributes */
	update_attributes(context);

	/* Draw tiles */
	draw_map(context, tile_set);

	/* Draw sprites */
	draw_all_sprite(tile_set);

	/* Draw item */
	draw_item(context, tile_set);

	/* Draw select cursor */
	draw_cursor(context, tile_set);

	/* update action_bar */
	update_action_bar(context,action_bar);

	/* update inventory */
	fill_inventory(context);

	/* update equipment slots */
	fill_equipment(context);

	gtk_widget_show_all(game_window);

	//gdk_window_thaw_updates(tile_set->window); /* to avoid flickering */

	if( first_time ) {
		/* Fill the context of the server, so that it knows which character we have selected from the select screen */
		network_send_context(context);
		first_time = FALSE;
	}
}

/*************************
 show window
Called from a on_* function so no GDK lock
*************************/
void show_game_window(context_t * context)
{
	gchar title[SMALL_BUF];

	g_snprintf(title,SMALL_BUF,"%s - %s",TITLE_NAME,context->character_name);
	gtk_window_set_title(GTK_WINDOW(game_window),title);
        gtk_widget_show_all(game_window);
}

/*************************
 hide window
*************************/
void hide_game_window(void)
{
        gdk_threads_enter();
        gtk_widget_hide_all(game_window);
        gdk_threads_leave();
}

/************************
Called on configure event (resize...)
************************/
gboolean on_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data)
{
	wlog(LOGDEBUG,"on_configure_event");

	main_window_width = gdk_window_get_width(tile_set->window);
	main_window_height = gdk_window_get_height(tile_set->window);

	return FALSE; /* continue the propagation */
}

/************************
Called on expose event
************************/
gboolean on_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	wlog(LOGDEBUG,"expose event: event = %d, widget = %p, window = %p, send_event = %d",event->type,widget,event->window,event->send_event);
	main_window_width = gdk_window_get_width(tile_set->window);
	main_window_height = gdk_window_get_height(tile_set->window);

	win_game_update((context_t *)user_data);

	return FALSE; /* continue the propagation */
}

/************************************
Manage keyboard on text entry
*************************************/
gboolean on_textentry_key_press_event (GtkWidget   *widget, GdkEventKey *event, gpointer     user_data)
{
	GtkEntryBuffer * buffer;
	const gchar * buff;

	if( event->type == GDK_KEY_PRESS ) {
		switch ( event->keyval ) {
			case GDK_KEY_Return :
				buffer=gtk_entry_get_buffer(GTK_ENTRY(widget));
				buff = gtk_entry_buffer_get_text(buffer);
				network_send_action(context_get_list_first(),WOG_CHAT,buff,NULL);
				gtk_entry_buffer_delete_text(buffer,0,-1);
				break;
			default:
				return FALSE;
				break;
		}
	}

	return FALSE;
}

/************************************
Manage keyboard on main window
*************************************/
gboolean on_game_window_key_press_event (GtkWidget   *widget, GdkEventKey *event, gpointer     user_data)
{
	context_t * ctx = (context_t *)user_data;

	if( event->type == GDK_KEY_PRESS ) {
		switch ( event->keyval ) {
			case GDK_KEY_Up :
				network_send_action(ctx,"move_up.lua",NULL);
				break;
			case GDK_KEY_Down :
				network_send_action(ctx,"move_down.lua",NULL);
				break;
			case GDK_KEY_Left :
				network_send_action(ctx,"move_left.lua",NULL);
				break;
			case GDK_KEY_Right :
				network_send_action(ctx,"move_right.lua",NULL);
				break;
			default:
				return TRUE;
				break;
		}
	}
	else {
		return FALSE;
	}

	/* Return true to avoid interacting with last selected "tree views" */
	return TRUE;
}

/******************************
 Called on click on inventory or equipment window
******************************/
gboolean update_selection (GtkWidget *widget,GdkEvent *event,gpointer user_data)
{
	context_t * context = (context_t *)user_data;

	/* update inventory */
	fill_inventory(context);

	/* update equipment slots */
	fill_equipment(context);

	return FALSE; /* continue the propagation */
}

/****************
return FALSE on error
****************/

gboolean win_game_init(context_t * context)
{
        GtkBuilder * builder = NULL;
        builder = gtk_builder_new ();

        gchar tmp[SMALL_BUF];
        gdk_threads_enter();
	//g_snprintf(tmp,SMALL_BUF,"./game.glade",DATADIR);
	g_snprintf(tmp,SMALL_BUF,"./game.glade");
	if( ! gtk_builder_add_from_file (builder, tmp,NULL)) {
		g_snprintf(tmp,SMALL_BUF,"%s/wog/client/game.glade",DATADIR);
		if( ! gtk_builder_add_from_file (builder, tmp,NULL)) {
			g_print("Error while loading UI file select_character.glade\n");
			gdk_threads_leave();
			return FALSE;
		}
	}

        game_window = GTK_WIDGET(gtk_builder_get_object(builder,"game_window"));
        tile_set = GTK_WIDGET(gtk_builder_get_object(builder,"tile_set"));
        action_bar = GTK_WIDGET(gtk_builder_get_object(builder,"action_bar"));
        attribute_list = GTK_WIDGET(gtk_builder_get_object(builder,"attribute_list"));
        inventory_list = GTK_WIDGET(gtk_builder_get_object(builder,"inventory_list"));
        equipment_list = GTK_WIDGET(gtk_builder_get_object(builder,"equipment_list"));
        selected_character_label = GTK_WIDGET(gtk_builder_get_object(builder,"selected_character_label"));
        selected_character_image = GTK_WIDGET(gtk_builder_get_object(builder,"selected_character_image"));
        selected_tile_label = GTK_WIDGET(gtk_builder_get_object(builder,"selected_tile_label"));
        selected_tile_image = GTK_WIDGET(gtk_builder_get_object(builder,"selected_tile_image"));
        pointed_label = GTK_WIDGET(gtk_builder_get_object(builder,"pointed_label"));
        pointed_image = GTK_WIDGET(gtk_builder_get_object(builder,"pointed_image"));
        textview = GTK_WIDGET(gtk_builder_get_object(builder,"textview"));
        textbuffer = GTK_TEXT_BUFFER(gtk_builder_get_object(builder,"textbuffer"));
        textentry = GTK_WIDGET(gtk_builder_get_object(builder,"textentry"));

        gtk_builder_connect_signals (builder, context);
        g_object_unref (G_OBJECT (builder));

        g_signal_connect(game_window,"configure-event",G_CALLBACK(on_configure_event),context);
        g_signal_connect(game_window,"expose-event",G_CALLBACK(on_expose_event),context);
        g_signal_connect(game_window,"key-press-event",G_CALLBACK(on_game_window_key_press_event),context);
        g_signal_connect(game_window,"destroy",G_CALLBACK(on_game_window_destroy),context);
        g_signal_connect(inventory_list,"button-release-event",G_CALLBACK(update_selection),context);
        g_signal_connect(equipment_list,"button-release-event",G_CALLBACK(update_selection),context);
        g_signal_connect(textentry,"key-press-event",G_CALLBACK(on_textentry_key_press_event),context);

        gtk_widget_hide_all(game_window);
        gtk_widget_realize(game_window);
       	gdk_threads_leave();

        return TRUE;
}

