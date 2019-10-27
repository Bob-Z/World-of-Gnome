/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2019 carabobz@gmail.com

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

#include "anim.h"
#include "client_server.h"
#include "Context.h"
#include "file.h"
#include "list.h"
#include "log.h"
#include "syntax.h"
#include "mutex.h"
#include <stdlib.h>
#include <SDL_mutex.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_stdinc.h>
#include <string>

static list_t * image_list = nullptr;

static anim_t * def_anim = nullptr;

/**************************
 **************************/
static anim_t * default_anim(Context * ctx)
{
	if (def_anim == nullptr)
	{
		def_anim = (anim_t*) malloc(sizeof(anim_t));

		def_anim->num_frame = 1;
		def_anim->tex = (SDL_Texture**) malloc(sizeof(SDL_Texture*));
		def_anim->tex[0] = SDL_CreateTexture(ctx->m_render, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, 1, 1);
		def_anim->w = 1;
		def_anim->h = 1;
		def_anim->delay = (Uint32*) malloc(sizeof(Uint32));
		;
		def_anim->delay[0] = 0;
		def_anim->total_duration = 0;
	}

	return def_anim;
}

/**************************
 **************************/
static anim_t * image_load(Context * ctx, const std::string & file_name)
{
	const std::string file_path = base_directory + "/" + file_name;

	anim_t * anim = anim_load(ctx->m_render, file_path.c_str());

	return anim;
}

/******************************************************
 Return a pointer to an anim_t object
 image_name is the image file path
 *******************************************************/
anim_t * imageDB_get_anim(Context * context, const char * image_name)
{
	anim_t * anim;

	if (image_name == nullptr)
	{
		return default_anim(context);
	}

	if (image_name[0] == 0)
	{
		return default_anim(context);
	}

	const std::string file_name = std::string(IMAGE_TABLE) + "/" + std::string(image_name);

//	wlog(LOGDEBUG,"Image get: %s",filename);

	SDL_LockMutex(imageDB_mutex);
	// Search for a previously loaded anim
	anim = (anim_t*) list_find(image_list, file_name.c_str());
	if (anim)
	{
//		wlog(LOGDEBUG,"Image find: %s",filename);
		SDL_UnlockMutex(imageDB_mutex);
		return anim;
	}

	// Try to load from a file
	file_lock(file_name.c_str());

	anim = image_load(context, file_name);

	if (anim != nullptr)
	{
//		wlog(LOGDEBUG,"Image loaded: %s",filename);
		list_update(&image_list, file_name.c_str(), anim);
		file_unlock(file_name.c_str());
		SDL_UnlockMutex(imageDB_mutex);
		return anim;
	}

	// Request an update to the server
//	wlog(LOGDEBUG,"Image asked: %s",filename);
	file_update(context, file_name.c_str());

	file_unlock(file_name.c_str());
	SDL_UnlockMutex(imageDB_mutex);

	return default_anim(context);
}

/******************************************************
 Return a pointer to a nullptr terminated array of anim_t object
 image_name is a nullptr terminated array of image files path
 return pointer MUST BE FREED
 *******************************************************/
anim_t ** imageDB_get_anim_array(Context * context, const char ** image_name)
{
	anim_t ** anim_output = nullptr;
	int num_image = 0;
	int current_image = 0;

	anim_output = (anim_t**) malloc(sizeof(anim_t*));
	anim_output[0] = nullptr;

	if (image_name == nullptr)
	{
		return anim_output;
	}

	while (image_name[current_image])
	{
		if (image_name[current_image][0] != 0)
		{
			anim_output = (anim_t**) realloc(anim_output, (num_image + 2) * sizeof(anim_t*));
			anim_output[num_image] = imageDB_get_anim(context, image_name[current_image]);
			anim_output[num_image + 1] = nullptr;
			num_image++;
		}
		current_image++;
	}

	return anim_output;
}

/****************************************************
 Remove an entry from the DB
 ******************************************************/
void image_DB_remove(const char * filename)
{
	anim_t * old_anim = nullptr;

	wlog(LOGDEVELOPER, "Image remove: %s", filename);
	/* Clean-up old anim if any */
	SDL_LockMutex(imageDB_mutex);
	old_anim = (anim_t*) list_find(image_list, filename);
	/* TODO Fix memory leak here */
	/* If we free the anim, it may still be used by the screen renderer and cause a crash */
	/* Try to postpone deletion ?? */
	if (old_anim)
	{
//		si_anim_free(old_anim);
	}

	list_update(&image_list, filename, nullptr);

	SDL_UnlockMutex(imageDB_mutex);
}
