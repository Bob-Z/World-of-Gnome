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

/* This manage an image data base */
/* When a thread need an image, it create a GtkWidget containing the image, then it adds an entry in the image data base (a hash table) with the name of the file containing the image on the disk (relative to $HOME/.config/wog/client/data) as the key and the pointer to the GtkWidget as the value.
When a file is received from the server, the parser's thread check the image data base and if the received file is an entry in the image data base, it update the corresponding GtkImage*/

#include <gtk/gtk.h>
#include "../common/common.h"
#include "anim.h"

gboolean updated_media = FALSE;

static GHashTable* imageDB;

static anim_t * def_anim = NULL;

static pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;

static void free_key(gpointer data)
{
	char * filename = (char *)data;

	wlog(LOGDEBUG,"Removing image %s",filename);
	g_free(data);
}

static void free_value(gpointer data)
{
	anim_t * a;
	int i;

	a = (anim_t*)data;

	if(a->tex) {
		for(i=0;i<a->num_frame;i++) {
			SDL_DestroyTexture(a->tex[i]);
		}
		free(a->tex);
	}

	g_free(data);
}

static anim_t * default_anim(context_t * ctx)
{
	if(def_anim == NULL) {
		def_anim = malloc(sizeof(anim_t));

		def_anim->num_frame = 1;
		def_anim->tex = malloc(sizeof(SDL_Texture*));
		def_anim->tex[0] = SDL_CreateTexture(ctx->render, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, 1,1);
		def_anim->current_frame = 0;
		def_anim->w = 1;
		def_anim->h = 1;
		def_anim->delay = malloc(sizeof(Uint32));;
		def_anim->delay[0] = 0;
		def_anim->prev_time = 0;
	}

	return def_anim;
}

void imageDB_init()
{
	/* key is a string : it is the filename of the image */
	imageDB = g_hash_table_new_full(g_str_hash,g_str_equal, free_key, free_value);
}

void imageDB_add_file(context_t * context, gchar * filename, anim_t * anim)
{
	if( g_hash_table_lookup(imageDB, filename) ) {
		return;
	}

	wlog(LOGDEBUG,"Adding image %s to the DB",filename);

	gchar * name = g_strdup(filename);
	g_hash_table_replace(imageDB, name, anim);

	/* request an update to the server */
	network_send_req_file(context,filename);
}

/* Return a copy of the image which must be destroyed (gtk_widget_destroy) */
/* image_name is the image file name without any path (e.g. : marquee.jpg) */
anim_t * imageDB_get_anim(context_t * context, const gchar * image_name)
{
	anim_t * anim;
	gchar * tmp;
	gchar * filename;

	filename = g_strconcat( IMAGE_TABLE, "/", image_name , NULL);

	pthread_mutex_lock(&db_mutex);

	/* Lookup DB */
	anim = g_hash_table_lookup(imageDB, filename);

	/* Find image in DB */
	if( anim != NULL ) {
		pthread_mutex_unlock(&db_mutex);
		g_free(filename);
		return anim;
	}

	/* The image was not in the imageDB */
	/* try to read the local file */
	tmp = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", IMAGE_TABLE, "/",  image_name , NULL);
	anim = anim_load(tmp);
	/* Unable to load local file */
	if( anim == NULL ) {
		pthread_mutex_unlock(&db_mutex);
		wlog(LOGDEBUG,"Return default anim for %s",filename);
		g_free(tmp);
		/* request an update to the server */
		network_send_req_file(context,filename);
		/* Use the default image */
		return default_anim(context);
	}
	g_free(tmp);

	/* Local file load OK, save to DB */
	imageDB_add_file(context, filename, anim);
	pthread_mutex_unlock(&db_mutex);

	return anim;
}

/* return NULL if filename is NOT in the DB
   else return the anim coresponding to this
   file (not of copy of it !) */
/* image_name is the image file name without any path (e.g. : marquee.jpg) */
anim_t * imageDB_check_anim(gchar * image_name)
{
	anim_t * anim;
	gchar * name;

	name =  g_strconcat( IMAGE_TABLE, "/", image_name , NULL);

	pthread_mutex_lock(&db_mutex);
	anim = g_hash_table_lookup(imageDB, name);
	pthread_mutex_unlock(&db_mutex);

	g_free(name);
	if( anim != NULL ) {
		return anim;
	}

	return NULL;
}

/* Update the database with a new file */
/* filename is the path relative to base directory ( e.g.: images/marquee.jpg ) */
/* return true if there is an update */
gboolean image_DB_update(context_t * context,gchar * filename)
{
	gchar * full_name;
	gchar * image_name = filename;
	gboolean updated = FALSE;
	anim_t * anim;
	anim_t * old_anim;

	/* search the / chararcter */
	while( image_name[0] != '/' && image_name[0] != 0) {
		image_name++;
	}

	if( image_name[0] == '/' ) {
		image_name++;
	} else {
		werr(LOGDEV,"invalid filename ( %s )",filename);
		return FALSE;
	}

	full_name = g_strconcat( g_getenv("HOME"),"/", base_directory, "/",filename, NULL);

	anim = anim_load(full_name);
	if( anim ) {
		updated = TRUE;
		/* It is actually an image, so try to update the anim DB */
		old_anim = imageDB_check_anim(image_name);
		if( old_anim != NULL ) {
			/* The anim exists in the DB, update the anim */
			imageDB_add_file(context,filename,anim);
		}
	}

	g_free(full_name);

	return updated;
}
