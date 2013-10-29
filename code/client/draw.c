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
#include "imageDB.h"
#include <glib.h>
#include <glib/gstdio.h>
#include <gdk/gdkkeysyms.h>
#include <dirent.h>
#include <win_game.h>


/* window size */
gint main_window_width;
gint main_window_height;

/* GtkWidget (image) representing the play field */
static GtkWidget ** map_tiles = NULL;
static int map_x = -1;
static int map_y = -1;

/* item list */
static GtkWidget ** item_list = NULL;

/* Save the last graphic element on which the mouse move over 
to detect when it moves on a new graphic element */
static gint	last_mouse_tile_x = -1;
static gint	last_mouse_tile_y = -1;

extern GtkWidget *             selected_character_label;
extern GtkWidget *             selected_character_image;
extern GtkWidget *             selected_tile_label;
extern GtkWidget *             selected_tile_image;
extern GtkWidget *             pointed_label;
extern GtkWidget *             pointed_image;

/* array of coordinate used when mouse button is clicked on a tile */
typedef struct tile_info {
	gint		x;
	gint		y;
	context_t * 	context;
	const gchar *	description;
} tile_info_t;
tile_info_t * map_info = NULL;

/* Event callbacks */
/*********************/
gboolean on_motion_over_sprite_event(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
	gchar * string;
	context_t * context = (context_t *)user_data;
	GdkPixbufAnimation* pixbuf;

	if( last_mouse_tile_x != context->pos_x || last_mouse_tile_y != context->pos_y) {
		last_mouse_tile_x = context->pos_x;
		last_mouse_tile_y = context->pos_y;
		string = g_strconcat( context->avatar_name,"\n", context->type, NULL);

		gtk_label_set_text(GTK_LABEL(pointed_label), string);
                g_free(string);
		pixbuf = gtk_image_get_animation( GTK_IMAGE(gtk_bin_get_child(GTK_BIN(context->sprite_image))) );
		gtk_image_set_from_animation(GTK_IMAGE(pointed_image), pixbuf);
	}

	return FALSE;
}

/*********************/
gboolean on_motion_over_tile_event(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
	GdkPixbufAnimation* pixbuf;
	tile_info_t * tile_info = (tile_info_t *)user_data;

	if( last_mouse_tile_x != tile_info->x || last_mouse_tile_y != tile_info->y) {
		last_mouse_tile_x = tile_info->x;
		last_mouse_tile_y = tile_info->y;

		if( tile_info->description != NULL ) {
			gtk_label_set_text(GTK_LABEL(pointed_label), tile_info->description);
		}
		else {
			gtk_label_set_text(GTK_LABEL(pointed_label), "");
		}
		pixbuf = gtk_image_get_animation( GTK_IMAGE(gtk_bin_get_child(GTK_BIN(widget))) );
		gtk_image_set_from_animation(GTK_IMAGE(pointed_image), pixbuf);
	}

	return FALSE;
}

/************************/
void update_selected_character(context_t * context)
{
	gchar * string;
	GdkPixbufAnimation* pixbuf;
	context_t * selected = NULL;

	selected = context_find(context->selection.id);

	if( selected != NULL ) {
		/* Check if the selected character is still on the same map */
		if( g_strcmp0(selected->map, context->map) == 0 ) {
			string = g_strconcat( selected->avatar_name,"\n", selected->type, NULL);
			gtk_label_set_text(GTK_LABEL(selected_character_label), string);
			g_free(string);
			pixbuf = gtk_image_get_animation( GTK_IMAGE(gtk_bin_get_child(GTK_BIN(selected->sprite_image))) );
			gtk_image_set_from_animation(GTK_IMAGE(selected_character_image), pixbuf);
			return;
		}

		/* selected not on the same map -> reset selected */
		context->selection.id = NULL;
		network_send_context(context);
	}

	gtk_label_set_text(GTK_LABEL(selected_character_label), " ");
	string = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", IMAGE_TABLE, "/", DEFAULT_IMAGE_FILE , NULL);
	gtk_image_set_from_file(GTK_IMAGE(selected_character_image),string);
	g_free(string);
}

/**************/
void update_selected_tile(tile_info_t * tile_info, GtkWidget * tile_image)
{
	GdkPixbufAnimation* pixbuf;

	if( tile_info != NULL && tile_info->description != NULL ) {
		gtk_label_set_text(GTK_LABEL(selected_tile_label), tile_info->description);
	}
	else {
		gtk_label_set_text(GTK_LABEL(selected_tile_label), "");
	}

	if( tile_image == NULL ) {
		gtk_image_set_from_animation(GTK_IMAGE(selected_tile_image), NULL);
	}
	else {
		pixbuf = gtk_image_get_animation( GTK_IMAGE(gtk_bin_get_child(GTK_BIN(tile_image))));
		gtk_image_set_from_animation(GTK_IMAGE(selected_tile_image), pixbuf);
	}
}

/***********/
gboolean on_button_over_sprite_event(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
	context_t * context = (context_t *)user_data;
	context_t * player_context = context_get_list_first();
	tile_info_t  * tile_info;
	GtkWidget * tile_image;

	tile_info = &map_info[(context->pos_y*context->map_x)+context->pos_x];
	tile_image = map_tiles[(context->pos_y*context->map_x)+context->pos_x];

	/* Set selection in player's context */
	player_context->selection.id= context->id;
	player_context->selection.map_coord[0]= tile_info->x;
	player_context->selection.map_coord[1]= tile_info->y;
	if(player_context->selection.map) {
		g_free(player_context->selection.map);
	}
	player_context->selection.map = g_strdup(player_context->map);

	/* Update helper window */
	update_selected_character(player_context);
	update_selected_tile(tile_info,tile_image);

	/* the first context is the player's context */
	network_send_context(player_context);

	return FALSE;
}

/*****************/
gboolean on_button_over_tile_event(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
	tile_info_t * tile_info = (tile_info_t *)user_data;

	/* Set selection in player's context */
	tile_info->context->selection.map_coord[0] = tile_info->x;
	tile_info->context->selection.map_coord[1] = tile_info->y;
	if(tile_info->context->selection.map) {
		g_free(tile_info->context->selection.map);
	}
	tile_info->context->selection.map = g_strdup(tile_info->context->map);

	/* Update helper window */
	update_selected_tile(tile_info,widget);

	/* the first context is the player's context */
	network_send_context(context_get_list_first());

	return FALSE;
}

/***********************
 Load the sprite if needed and return a copy of the sprite image
You MUST have the GDK lock when calling this function 
***********************/

GtkWidget * get_sprite(context_t * context) {

	const gchar * sprite_name = NULL;
	GtkWidget * new_sprite = NULL;
	GtkWidget * new_event_box = NULL;

	/* compute the sprite file name */
	if(!read_string(AVATAR_TABLE,context->type,&sprite_name,AVATAR_KEY_SPRITE,NULL)) {
		g_critical("Can't read sprite name for \"%s\" type",context->type);
		g_assert(1);
	}
		
	/* get the image from the imageDB */
	/* We must use the player's context to have the communication streams */
	new_sprite = imageDB_get_widget(context_get_list_first(),sprite_name);

	/* Add image to an eventbox to be able to catch event */
	new_event_box = gtk_event_box_new();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(new_event_box),FALSE);
	gtk_container_add(GTK_CONTAINER(new_event_box), new_sprite);

	/* Set up mouse motion event */
	gtk_widget_add_events(new_event_box, GDK_POINTER_MOTION_MASK);
	g_signal_connect(new_event_box, "motion-notify-event", G_CALLBACK(on_motion_over_sprite_event), (gpointer)context);
	g_signal_connect(new_event_box, "button-press-event", G_CALLBACK(on_button_over_sprite_event), (gpointer)context);

	return  new_event_box;

}

/************************
Draw  sprite
tile_x and tile_y are the number of tiles from the tile in the center of the view.
(0,0) is the center of the view so it is where the user's sprite appears

You MUST have the GDK lock when calling this function 
************************/
void draw_sprite(context_t * context, GtkWidget * tile_set, gint tile_x, gint tile_y,context_t * player_context)
{
	GtkWidget * image = NULL;
	GdkPixbufAnimation * pixbuf;
	gint x;
	gint y;

	if( context == NULL ) {
		return;
	}

	if( context->sprite_image != NULL ) {
		gtk_widget_destroy(context->sprite_image);
		context->sprite_image = NULL;
	}

	if(g_strcmp0(player_context->map,context->map) != 0) {
		return;
	}

	context->sprite_image = get_sprite(context);
	/* Get the actual GTK image (it's an event box which is stored in context) */
	image = gtk_bin_get_child(GTK_BIN(context->sprite_image));
	

	/* Retrieve sprite size */
	pixbuf = gtk_image_get_animation(GTK_IMAGE(image));
	x = gdk_pixbuf_animation_get_width(pixbuf);
	y = gdk_pixbuf_animation_get_height(pixbuf);

	/* center of the window + (number of tile from the center * size of a tile) - (size of the sprite/2) */
	gtk_layout_put(GTK_LAYOUT(tile_set),context->sprite_image,
				(main_window_width/2) + (tile_x * context->tile_x) - (x/2) ,	
				(main_window_height/2)+ (tile_y * context->tile_y) - (y/2) );
}

/***********************
 Draw all sprite of the list 
You MUST have the GDK lock when calling this function 
***********************/
void draw_all_sprite(GtkWidget * tile_set)
{
        context_t * ctx = NULL;

        context_lock_list();

        context_t * first = context_get_list_first();

        /* The first context is the context of the user, we draw it after the other */
        ctx = first->next;

        while( ctx != NULL ) {
               	draw_sprite(ctx, tile_set, ctx->pos_x - first->pos_x, ctx->pos_y - first->pos_y,first);
                ctx = ctx->next;
        }

        /* Finally draw the user's sprite */
        draw_sprite(first, tile_set, 0,0,first);

        context_unlock_list();
}

/***********************
 Create a widget containg an image for the requested tile
You MUST have the GDK lock when calling this function 
index is the index in the map_cooridnate array for this tile
***********************/
GtkWidget * get_tile(context_t * context, gchar * tile_name, gint index, const gchar ** description)
{
	GtkWidget * new_image = NULL; 
	GtkWidget * new_event_box = NULL;
	const gchar * tile_file_name = NULL;

	*description = NULL;

	/* compute the tile's image file name */
	if(!read_string(TILE_TABLE,tile_name,&tile_file_name,TILE_KEY_IMAGE,NULL)) {
		return NULL;
	}

	/* Save description for caller */
	read_string(TILE_TABLE,tile_name,description,TILE_KEY_TEXT,NULL);

        /* load image from imageDB */
	new_image = imageDB_get_widget(context,tile_file_name);

	/* Add image to an eventbox to be able to catch event */
	new_event_box = gtk_event_box_new();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(new_event_box),FALSE);
	gtk_container_add(GTK_CONTAINER(new_event_box), new_image);

	/* set up mouse motion event */
	gtk_widget_add_events(new_event_box, GDK_POINTER_MOTION_MASK);
	g_signal_connect(new_event_box, "motion-notify-event", G_CALLBACK(on_motion_over_tile_event), (gpointer)&(map_info[index]));
	g_signal_connect(new_event_box, "button-press-event", G_CALLBACK(on_button_over_tile_event), (gpointer)&(map_info[index]));

	return new_event_box;
}

/**********************
 Clean-up map tiles
 You MUST have the GDK lock when calling this function 
***********************/
void draw_cleanup(context_t * context)
{
	int i;

	if(map_tiles != NULL ) {
		for(i=0;i<map_x*map_y;i++){
			if( map_tiles[i] != NULL ) {
				gtk_widget_destroy(map_tiles[i]);
			}
		}
		g_free(map_tiles);
		map_tiles = NULL;
		map_x = -1;
		map_y = -1;
	}
}

/***********************
 Load map if needed
You MUST have the GDK lock when calling this function 
***********************/
void draw_map(context_t * context, GtkWidget * tile_set) {
	gchar ** value = NULL;
	gint i,j;

	draw_cleanup(context);

	/* description of the map */
	if( context->map_x == -1 ) {
		if(!read_int(MAP_TABLE, context->map, &i,MAP_KEY_SIZE_X,NULL)) {
			return;
		}
		context_set_map_x(context, i);
	}

	if( context->map_y == -1 ) {
		if(!read_int(MAP_TABLE, context->map, &i,MAP_KEY_SIZE_Y,NULL)) {
			return;
		}
		context_set_map_y( context,i);
	}

	if( context->tile_x == -1 ) {
		if(!read_int(MAP_TABLE, context->map, &i,MAP_KEY_TILE_SIZE_X,NULL)) {
			return;
		}
		context_set_tile_x( context, i);
	}

	if( context->tile_y == -1 ) {
		if(!read_int(MAP_TABLE, context->map,&i,MAP_KEY_TILE_SIZE_Y,NULL)) {
			return;
		}
		context_set_tile_y( context, i);
	}

	if( map_tiles == NULL ) {
		map_x = context->map_x;
		map_y = context->map_y;
		map_tiles = g_malloc0( map_x * map_y * sizeof(GtkWidget *) );
	}

	if(!read_list(MAP_TABLE, context->map, &value,MAP_KEY_SET,NULL)) {
		return;
	}

	/* destroy old map info */
	if( map_info != NULL ) {
		g_free(map_info);
	}

	/* Create map info */
	map_info = g_malloc0( sizeof(tile_info_t) * context->map_x*context->map_y );
	for( j=0 ; j< context->map_y; j++) {
		for( i=0 ; i< context->map_x; i++) {
			map_info[j*context->map_x + i].x = i;
			map_info[j*context->map_x + i].y = j;
			map_info[j*context->map_x + i].context = context;
			map_info[j*context->map_x + i].description = NULL;
		}
	}

	/* Parse map string */
	gint x = 0;
	gint y = 0;
	const gchar * description;
	gint pos_right;
	gint pos_left;
	gint pos_top;
	gint pos_bottom;

	i=0;
	while(value[i] != NULL ) {
		pos_left = main_window_width/2 - context->tile_x/2 + (x - context->pos_x ) * context->tile_x;
		pos_right = pos_left + context->tile_x;

		pos_top = main_window_height/2 - context->tile_y/2 + (y - context->pos_y ) * context->tile_y;
		pos_bottom = pos_top + context->tile_y;

		/* Clipping */
		if(pos_right < 0 || pos_left > main_window_width ||
		pos_top > main_window_height || pos_bottom < 0 ) {
			goto next_tile;
		}


		/* load new tile */
		map_tiles[y*context->map_x + x] = get_tile(context, value[i], y*context->map_x + x, &description);
		if( map_tiles[y*context->map_x + x] == NULL ) {
			goto next_tile;
		}

		/* update description of this tile */
		map_info[y*context->map_x + x].description = description;

		gtk_layout_put(GTK_LAYOUT(tile_set),map_tiles[y*context->map_x + x],pos_left,pos_top);

		/* next */
		next_tile:
		x++;
		if( x >= context->map_x ) {
			y++;
			x = 0;
		}
		i++;
	}

	g_free(value);
}

/***********************
Create the cursor image if not existing.
Draw the cursor. 
You MUST have the GDK lock when calling this function 
***********************/
void draw_cursor(context_t * context, GtkWidget * tile_set) {
	gint x_coord;
	gint y_coord;
	GdkPixbufAnimation *Pixbuf;
	gint Width, Height;
	context_t * ctx;
	/* Cursor widget */
	static GtkWidget * sprite_cursor = NULL;
	static GtkWidget * tile_cursor = NULL;


	if( sprite_cursor != NULL ) {
		gtk_widget_destroy(sprite_cursor);
	}
	if( tile_cursor != NULL ) {
		gtk_widget_destroy(tile_cursor);
	}

	/* calculate coordinate of the cursor regarding it's position relative to the center of the screen (where the player sprite is) */

	/* cursor for character */
	if(context->selection.id != NULL) {
		/* get a copy of the cursor image for sprite */
		sprite_cursor = imageDB_get_widget(context,CURSOR_SPRITE_FILE);

		/* Adjust the coordinate depending on the size of the cursor image */
		/* get the GdkPixbuf */
		Pixbuf = gtk_image_get_animation(GTK_IMAGE(sprite_cursor));
		/* Size of the image */
		Width = gdk_pixbuf_animation_get_width(Pixbuf);
		Height = gdk_pixbuf_animation_get_height(Pixbuf);

		ctx = context_find(context->selection.id);
		x_coord = main_window_width/2 + (ctx->pos_x - context->pos_x ) * context->tile_x;
		y_coord = main_window_height/2 + (ctx->pos_y - context->pos_y ) * context->tile_y;

		x_coord = x_coord - Width/2;
		y_coord = y_coord - Height/2;

		gtk_layout_put(GTK_LAYOUT(tile_set),sprite_cursor,x_coord,y_coord);
	}

	/* cursor for tile */
	if( context->selection.map_coord[0] != -1 && context->selection.map_coord[1] != -1 && context->selection.map && !g_strcmp0(context->selection.map,context->map)) {
		/* get a copy of the cursor image for tile */
		tile_cursor = imageDB_get_widget(context,CURSOR_TILE_FILE);

		/* Adjust the coordinate depending on the size of the cursor image */
		/* get the GdkPixbuf */
		Pixbuf = gtk_image_get_animation(GTK_IMAGE(tile_cursor));
		/* Size of the image */
		Width = gdk_pixbuf_animation_get_width(Pixbuf);
		Height = gdk_pixbuf_animation_get_height(Pixbuf);

		x_coord = main_window_width/2 + (context->selection.map_coord[0] - context->pos_x ) * context->tile_x;
		y_coord = main_window_height/2 + (context->selection.map_coord[1] - context->pos_y ) * context->tile_y;
		x_coord = x_coord - Width/2;
		y_coord = y_coord - Height/2;

		gtk_layout_put(GTK_LAYOUT(tile_set),tile_cursor,x_coord,y_coord);
	}
}

/**************************
Put the requested item on the current map
**************************/
void put_item(context_t * context,GtkWidget * tile_set, const gchar * item_name, gint x,gint y) {
	const gchar * sprite_name = NULL;
	gint i=0;
	gint x_coord = 0;
	gint y_coord = 0;

	if(!read_string(ITEM_TABLE,item_name,&sprite_name,ITEM_SPRITE,NULL)) {
		return;
	}

	/* search next entry in item_list */
	i=0;
	while(item_list[i] != NULL) {
		i++;
	}

	item_list=g_realloc(item_list,(i+2)*sizeof(GtkWidget *));
	item_list[i] = imageDB_get_widget(context,sprite_name);
	item_list[i+1] = NULL;

	/* calculate coordinate of the item regarding it's position relative to the center of the screen (where the player sprite is) */
	x_coord = main_window_width/2 + (x - context->pos_x ) * context->tile_x;
	y_coord = main_window_height/2 + (y - context->pos_y ) * context->tile_y;

	/* Adjust the coordinate depending of the size of the cursor image */
	GdkPixbufAnimation *Pixbuf;
	gint Width, Height;

	/* get the GdkPixbuf */
	Pixbuf = gtk_image_get_animation(GTK_IMAGE(item_list[i]));
	/* Size of the image */
	Width = gdk_pixbuf_animation_get_width(Pixbuf);
	Height = gdk_pixbuf_animation_get_height(Pixbuf);

	x_coord = x_coord - Width/2;
	y_coord = y_coord - Height/2;

	gtk_layout_put(GTK_LAYOUT(tile_set),item_list[i],x_coord,y_coord);
}

/***********************
Draw all items on the current map.
You MUST have the GDK lock when calling this function 
***********************/
void draw_item(context_t * context, GtkWidget * tile_set) {
	gint x;
	gint y;
	gint i=0;
	gchar ** items;

	i = 0;
	if(item_list != NULL ) {
		while(item_list[i] != NULL ) {
			gtk_widget_destroy(item_list[i]);
			i++;
		}
		g_free(item_list);
	}
	item_list = NULL;

	if(!get_group_list(MAP_TABLE,context->map,&items,MAP_ENTRY_ITEM_LIST,NULL)) {
		return;
	}

	item_list = g_malloc0(sizeof(GtkWidget *));
	item_list[0] = NULL;

	i = 0;
	while( items[i] != NULL ) {
		if(!read_int(MAP_TABLE,context->map,&x,MAP_ENTRY_ITEM_LIST,items[i],MAP_ITEM_POS_X,NULL)) {
			i++;
			continue;
		}

		if(!read_int(MAP_TABLE,context->map,&y,MAP_ENTRY_ITEM_LIST,items[i],MAP_ITEM_POS_Y,NULL)) {
			i++;
			continue;
		}

		put_item(context,tile_set,items[i],x,y);

		i++;
	}
	g_free(items);
}
