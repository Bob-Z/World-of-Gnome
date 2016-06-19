/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2015 carabobz@gmail.com

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

#include "../common/common.h"
#include "../sdl_item/anim.h"

static list_t * image_list = NULL;

static anim_t * def_anim = NULL;

/**************************
**************************/
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

/**************************
**************************/
static anim_t * image_load(context_t * ctx, char * filename)
{
	char * fullname;
	anim_t * anim;

	fullname = strconcat(base_directory,"/",filename,NULL);

	anim = anim_load(ctx->render, fullname);

	free(fullname);

	return anim;
}

/******************************************************
Return a pointer to an anim_t object
image_name is the image file path
*******************************************************/
anim_t * imageDB_get_anim(context_t * context, const char * image_name)
{
	anim_t * anim;
	char * filename;

	if( image_name == NULL ){
		return default_anim(context);
	}

	if( image_name[0] == 0 ){
		return default_anim(context);
	}

	filename = strconcat(IMAGE_TABLE,"/",image_name,NULL);

//	wlog(LOGDEBUG,"Image get: %s",filename);

	SDL_LockMutex(imageDB_mutex);
	/* Search for a previously loaded anim */
	anim = list_find(image_list,filename);
	if(anim) {
//		wlog(LOGDEBUG,"Image find: %s",filename);
		free(filename);
		SDL_UnlockMutex(imageDB_mutex);
		return anim;
	}

	/* Try to load from a file */
	file_lock(filename);

	anim = image_load(context,filename);

	if(anim) {
//		wlog(LOGDEBUG,"Image loaded: %s",filename);
		list_update(&image_list,filename,anim);
		file_unlock(filename);
		free(filename);
		SDL_UnlockMutex(imageDB_mutex);
		return anim;
	}

	/* Request an update to the server */
//	wlog(LOGDEBUG,"Image asked: %s",filename);
	file_update(context,filename);

	file_unlock(filename);
	free(filename);
	SDL_UnlockMutex(imageDB_mutex);

	return default_anim(context);
}

/******************************************************
Return a pointer to a NULL terminated array of anim_t object
image_name is a NULL terminated array of image files path
return pointer MUST BE FREED
*******************************************************/
anim_t ** imageDB_get_anim_array(context_t * context, const char ** image_name)
{
	anim_t ** anim_output = NULL;
	int num_image = 0;
	int current_image = 0;

	anim_output = malloc( sizeof(anim_t*) );
	anim_output[0] = NULL;

	if( image_name == NULL ) {
		return anim_output;
	}

	while( image_name[current_image] ) {
		if( image_name[current_image][0] != 0 ){
			anim_output = realloc(anim_output,(num_image+2)*sizeof(anim_t*));
			anim_output[num_image] = imageDB_get_anim(context,image_name[current_image]);
			anim_output[num_image+1] = NULL;
			num_image++;
		}
		current_image++;
	}

	return anim_output;
}

/****************************************************
Remove an entry from the DB
******************************************************/
void image_DB_remove(char * filename)
{
	anim_t * old_anim;

	wlog(LOGDEBUG,"Image remove: %s",filename);
	/* Clean-up old anim if any */
	SDL_LockMutex(imageDB_mutex);
	old_anim = list_find(image_list,filename);
	/* TODO Fix memory leak here */
	/* If we free the anim, it may still be used by the screen renderer and cause a crash */
	/* Try to postpone deletion ?? */
	if( old_anim ) {
//		si_anim_free(old_anim);
	}

	list_update(&image_list,filename,NULL);

	SDL_UnlockMutex(imageDB_mutex);
}
