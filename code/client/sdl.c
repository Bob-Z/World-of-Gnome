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

#include "sdl.h"

int fullscreen = 0;

int mouse_x = 0;
int mouse_y = 0;

static char keyboard_buf[2048];
static unsigned int keyboard_index = 0;
static void (*keyboard_cb)(void * arg) = NULL;

//You must SDL_LockSurface(surface); then SDL_UnlockSurface(surface); before calling this function
void sdl_set_pixel(SDL_Surface *surface, int x, int y, Uint32 R, Uint32 G, Uint32 B, Uint32 A)
{
	Uint32 color = (A << 24) + (R << 16) + (G << 8) + (B);
	Uint8 *target_pixel = (Uint8 *)surface->pixels + y * surface->pitch + x * sizeof(color);
	*(Uint32 *)target_pixel = color;
}

void sdl_cleanup()
{
	SDL_Quit();
}

void sdl_init(context_t * context)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0) {
		werr(LOGUSER,"SDL init failed: %s.\n",SDL_GetError());
		exit(EXIT_FAILURE);
	}

	atexit(sdl_cleanup);

	context->window = SDL_CreateWindow("World of Gnome",
								 SDL_WINDOWPOS_UNDEFINED,
								 SDL_WINDOWPOS_UNDEFINED,
								 DEFAULT_SCREEN_W, DEFAULT_SCREEN_H,
								 SDL_WINDOW_RESIZABLE);
	if( context->window == NULL) {
		printf("SDL window init failed: %s.\n",SDL_GetError());
		exit(EXIT_FAILURE);
	}

	context->render = SDL_CreateRenderer(context->window, -1, 0);
	if( context->render == NULL) {
		printf("SDL renderer init failed: %s\n",SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_RenderSetLogicalSize(context->render,DEFAULT_SCREEN_W,DEFAULT_SCREEN_H);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
}

void sdl_mouse_manager(SDL_Event * event, item_t * item_list)
{
	SDL_Rect rect;
	int i;

	if(item_list == NULL) {
		return;
	}

	switch (event->type) {
	case SDL_MOUSEMOTION:
#if 0
		printf("Mouse moved by %d,%d to (%d,%d)\n",
			   event->motion.xrel, event->motion.yrel,
			   event->motion.x, event->motion.y);
#endif
		rect.x = event->motion.x;
		rect.y = event->motion.y;
		mouse_x = rect.x;
		mouse_y = rect.y;
//			printf("orig coord = %d,%d \n",rect.x,rect.y);
		i=0;
		do {
			item_list[i].current_frame = item_list[i].frame_normal;
			if( (item_list[i].rect.x < rect.x) &&
					((item_list[i].rect.x+item_list[i].rect.w) > rect.x) &&
					(item_list[i].rect.y < rect.y) &&
					((item_list[i].rect.y+item_list[i].rect.h) > rect.y) ) {
				item_list[i].current_frame = item_list[i].frame_over;
				if( item_list[i].over ) {
					item_list[i].over(item_list[i].over_arg);
				}
			}
			if(item_list[i].clicked) {
				item_list[i].current_frame = item_list[i].frame_click;
			}
			i++;
		}
		while(!item_list[i-1].last);

		break;
	case SDL_MOUSEBUTTONDOWN:
#if 0
		printf("Mouse button %d pressed at (%d,%d)\n",
			   event->button.button, event->button.x, event->button.y);
#endif
		rect.x = event->button.x;
		rect.y = event->button.y;
//			printf("orig coord = %d,%d \n",rect.x,rect.y);
		i=0;
		do {
			if( (item_list[i].rect.x < rect.x) &&
					((item_list[i].rect.x+item_list[i].rect.w) > rect.x) &&
					(item_list[i].rect.y < rect.y) &&
					((item_list[i].rect.y+item_list[i].rect.h) > rect.y) ) {
				if( item_list[i].click_left && event->button.button == SDL_BUTTON_LEFT) {
					item_list[i].current_frame=item_list[i].frame_click;
					item_list[i].clicked=1;
				}
				if( item_list[i].click_right && event->button.button == SDL_BUTTON_RIGHT) {
					item_list[i].current_frame=item_list[i].frame_click;
					item_list[i].clicked=1;
				}
			}
			i++;
		}
		while(!item_list[i-1].last);

		break;
	case SDL_MOUSEBUTTONUP:
		rect.x = event->button.x;
		rect.y = event->button.y;
		i=0;
		do {
			item_list[i].clicked=0;
			item_list[i].current_frame = item_list[i].frame_normal;
			if( (item_list[i].rect.x < rect.x) &&
					((item_list[i].rect.x+item_list[i].rect.w) > rect.x) &&
					(item_list[i].rect.y < rect.y) &&
					((item_list[i].rect.y+item_list[i].rect.h) > rect.y) ) {
				if( item_list[i].click_left && event->button.button == SDL_BUTTON_LEFT) {
					item_list[i].click_left(item_list[i].click_left_arg);
				}
				if( item_list[i].click_right && event->button.button == SDL_BUTTON_RIGHT) {
					item_list[i].click_right(item_list[i].click_right_arg);
				}
			}
			i++;
		}
		while(!item_list[i-1].last);

		break;
	}
}

/* Take care of system's windowing event */
void sdl_screen_manager(context_t * ctx,SDL_Event * event)
{
	const Uint8 *keystate;

	switch (event->type) {
        case SDL_WINDOWEVENT:
		switch(event->window.event) {
		case SDL_WINDOWEVENT_RESIZED:
			SDL_RenderSetLogicalSize(ctx->render,event->window.data1,event->window.data2);
			break;
		}
	break;
	case SDL_KEYDOWN:
		switch (event->key.keysym.sym) {
		case SDLK_RETURN:
			keystate = SDL_GetKeyboardState(NULL);

			if( keystate[SDL_SCANCODE_RALT] || keystate[SDL_SCANCODE_LALT] ) {
				if(!fullscreen) {
					fullscreen = SDL_WINDOW_FULLSCREEN_DESKTOP;
				} else {
					fullscreen = 0;
				}
				SDL_SetWindowFullscreen(ctx->window,fullscreen);
				break;
			}
			break;
		default:
			break;
		}
		break;
	case SDL_QUIT:
		exit(EXIT_SUCCESS);
		break;
	default:
		break;
	}
}

void sdl_loop_manager()
{
	static Uint32 old_timer = 0;
	Uint32 timer;

	if( old_timer == 0 ) {
		old_timer = SDL_GetTicks();
	}

	timer = SDL_GetTicks();

	if( timer < old_timer + FRAME_DELAY ) {
		SDL_Delay(old_timer + FRAME_DELAY - timer);
	}
}

void sdl_blit_frame(context_t * ctx,anim_t * anim, SDL_Rect * rect, int frame_num,int x, int y)
{
	SDL_Rect r;

	r.w = rect->w;
	r.h = rect->h;
	r.x = rect->x + x;
	r.y = rect->y + y;

	if( anim ) {
		if( SDL_RenderCopy(ctx->render,anim->tex[frame_num],NULL,&r) < 0) {
			werr(LOGDEV,"SDL_RenderCopy error\n");
		}
	}
}

int sdl_blit_anim(context_t * ctx,anim_t * anim, SDL_Rect * rect, int start, int end,int x, int y)
{
	Uint32 time = SDL_GetTicks();

	sdl_blit_frame(ctx,anim,rect,anim->current_frame,x,y);

	if( anim->prev_time == 0 ) {
		anim->prev_time = time;
	}
	if( time >= anim->prev_time + anim->delay) {
		(anim->current_frame)++;
		anim->prev_time = time;
		if( end != -1 ) {
			if(anim->current_frame >= end) {
				anim->current_frame = start;
				return 1;
			}
		} else {
			if(anim->current_frame >= anim->num_frame) {
				anim->current_frame = 0;
				return 1;
			}
		}
	}

	return 0;
}

int sdl_blit_item(context_t * ctx,item_t * item,int x, int y)
{

	if( item->frame_normal == -1 ) {
		return sdl_blit_anim(ctx,item->anim,&item->rect,item->anim_start,item->anim_end,x,y);
	} else {
		sdl_blit_frame(ctx,item->anim,&item->rect,item->current_frame,x,y);
	}

	return 0;
}

void sdl_blit_item_list(context_t * ctx,item_t * list,int x, int y)
{
	int i = 0;

	if(list == NULL) {
		return;
	}

	do {
		sdl_blit_item(ctx,&list[i],x,y);
		i++;
	}
	while(!list[i-1].last);
}

void sdl_keyboard_init(char * string, void (*cb)(void*arg))
{
	keyboard_index=0;
	if( string ) {
		strcpy(keyboard_buf,string);
		keyboard_index=strlen(keyboard_buf);
	}
	keyboard_buf[keyboard_index]=0;
	keyboard_cb=cb;
}

char * sdl_keyboard_get_buf()
{
	if(keyboard_cb) {
		return keyboard_buf;
	} else {
		return NULL;
	}
}

void sdl_keyboard_manager(SDL_Event * event)
{
	const Uint8 *keystate;

	switch (event->type) {
	case SDL_KEYDOWN:
		if( event->key.keysym.sym == SDLK_RETURN ) {
			if( keyboard_cb ) {
				keyboard_cb(NULL);
				keyboard_cb=NULL;
			}
		}

		if( event->key.keysym.sym == SDLK_DELETE ||
				event->key.keysym.sym == SDLK_BACKSPACE) {
			if(keyboard_index > 0 ) {
				keyboard_index--;
			}
			keyboard_buf[keyboard_index]=0;
		}

		if( event->key.keysym.sym >= SDLK_SPACE &&
				event->key.keysym.sym < SDLK_DELETE ) {

			/* Uppercase */
			keystate = SDL_GetKeyboardState(NULL);
			if( (keystate[SDL_SCANCODE_RSHIFT] ||
					keystate[SDL_SCANCODE_LSHIFT] ) &&
					(event->key.keysym.sym >=SDL_SCANCODE_A &&
					 event->key.keysym.sym <=SDL_SCANCODE_Z) ) {
				event->key.keysym.sym = (SDL_Scancode)(event->key.keysym.sym-32);
			}
			keyboard_buf[keyboard_index]=event->key.keysym.sym;
			if( keyboard_index < sizeof(keyboard_buf)) {
				keyboard_index++;
			}
			keyboard_buf[keyboard_index]=0;
		}
		break;
	default:
		break;
	}
}

void sdl_blit_to_screen(context_t * ctx)
{
	SDL_RenderPresent(ctx->render);
}
