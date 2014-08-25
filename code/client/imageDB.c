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

#include "../common/common.h"
#include "../sdl_item/anim.h"

static list_t * image_list = NULL;

static anim_t * def_anim = NULL;

/* TODO : move this to sdl_item */
/**************************
**************************/
static void free_anim(anim_t * anim)
{
	int i;

	if(anim->tex) {
		for(i=0; i<anim->num_frame; i++) {
			SDL_DestroyTexture(anim->tex[i]);
		}
		free(anim->tex);
	}

	free(anim);
}

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
image_name is the image file path and name in the IMAGE_TABLE directory
*******************************************************/
anim_t * imageDB_get_anim(context_t * context, const char * image_name)
{
	anim_t * anim;
	char * filename;

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
	if( old_anim ) {
		free_anim(old_anim);
	}

	list_update(&image_list,filename,NULL);

	SDL_UnlockMutex(imageDB_mutex);
}
