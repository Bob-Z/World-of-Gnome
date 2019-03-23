/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2017 carabobz@gmail.com

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

#include "common.h"
#include "imageDB.h"
#include "file.h"
#include "anim.h"
#include "item.h"
#include "sdl.h"
#include "screen.h"
#include "textview.h"
#include "network_client.h"
#include "ui_play.h"
#include "option_client.h"
#include "Camera.h"
#include <limits.h>

#define ITEM_FONT "Ubuntu-C.ttf"
#define ITEM_FONT_SIZE (15)

#define NORTH (1<<0)
#define SOUTH (1<<1)
#define EAST (1<<2)
#define WEST (1<<3)

#define ALIGN_CENTER	(0)
#define ALIGN_LOWER	(1)

#define MAX_LAYER	(100)

static item_t * item_list = nullptr;
static bool change_map = false;
static int current_map_x = -1;
static int current_map_y = -1;
static layer_t * default_layer = nullptr;
static char * sfx = nullptr;
static bool g_IsMusicPlaying = false;

/**********************************
 **********************************/
int scr_play_get_current_x()
{
	return current_map_x;
}

/**********************************
 **********************************/
int scr_play_get_current_y()
{
	return current_map_y;
}

/**********************************
 **********************************/
static void cb_select_sprite(void *arg)
{
	char * id = (char*) arg;
	context_t * ctx = context_get_player();

	network_send_action(ctx, option_get().action_select_character, id, nullptr);
}

/**********************************
 **********************************/
static void cb_redo_sprite(void *arg)
{
	char * script = nullptr;
	char * last_action = nullptr;

	cb_select_sprite(arg);

	last_action = ui_play_get_last_action();
	if (last_action)
	{
		script = strdup(last_action);
		ui_play_cb_action(script);
		free(script);
	}
}

/**********************************
 **********************************/
static void cb_zoom(Uint32 l_Unused1, Uint32 l_Unused2)
{
	Camera * l_pCamera = screen_get_camera();

	l_pCamera->setZoom(l_pCamera->getZoom() + 1);
}

/**********************************
 **********************************/
static void cb_unzoom(Uint32 l_Unused1, Uint32 l_Unused2)
{
	Camera * l_pCamera = screen_get_camera();

	l_pCamera->setZoom(l_pCamera->getZoom() - 1);
}

/**********************************
 Select sprite image to display
 Return nullptr if no sprite can be found
 Returned anim_t ** must be FREED
 **********************************/
static anim_t ** select_sprite(context_t * ctx)
{
	anim_t ** sprite;
	char * sprite_name = nullptr;
	char ** sprite_list = nullptr;
	const char * sprite_name_array[2] =
	{ nullptr, nullptr };
	context_t * player_context = context_get_player();

	// Try to find a sprite depending on the orientation
	if (ctx->orientation & NORTH)
	{
		entry_read_string(CHARACTER_TABLE, ctx->id, &sprite_name,
		CHARACTER_KEY_DIR_N_SPRITE, nullptr);
	}
	if ((ctx->orientation & SOUTH) && sprite_name == nullptr)
	{
		entry_read_string(CHARACTER_TABLE, ctx->id, &sprite_name,
		CHARACTER_KEY_DIR_S_SPRITE, nullptr);
	}
	if ((ctx->orientation & EAST) && sprite_name == nullptr)
	{
		entry_read_string(CHARACTER_TABLE, ctx->id, &sprite_name,
		CHARACTER_KEY_DIR_E_SPRITE, nullptr);
	}
	if ((ctx->orientation & WEST) && sprite_name == nullptr)
	{
		entry_read_string(CHARACTER_TABLE, ctx->id, &sprite_name,
		CHARACTER_KEY_DIR_W_SPRITE, nullptr);
	}

	if (sprite_name)
	{
		if (sprite_name[0] != 0)
		{
			sprite_name_array[0] = sprite_name;
			sprite = imageDB_get_anim_array(player_context, sprite_name_array);
			free(sprite_name);
			return sprite;
		}
		free(sprite_name);
	}

	// Try sprite lists
	if (ctx->orientation & NORTH)
	{
		entry_read_list(CHARACTER_TABLE, ctx->id, &sprite_list,
		CHARACTER_KEY_DIR_N_SPRITE, nullptr);
	}
	if ((ctx->orientation & SOUTH) && sprite_list == nullptr)
	{
		entry_read_list(CHARACTER_TABLE, ctx->id, &sprite_list,
		CHARACTER_KEY_DIR_S_SPRITE, nullptr);
	}
	if ((ctx->orientation & EAST) && sprite_list == nullptr)
	{
		entry_read_list(CHARACTER_TABLE, ctx->id, &sprite_list,
		CHARACTER_KEY_DIR_E_SPRITE, nullptr);
	}
	if ((ctx->orientation & WEST) && sprite_list == nullptr)
	{
		entry_read_list(CHARACTER_TABLE, ctx->id, &sprite_list,
		CHARACTER_KEY_DIR_W_SPRITE, nullptr);
	}

	if (sprite_list)
	{
		sprite = imageDB_get_anim_array(player_context,
				(const char **) sprite_list);
		deep_free(sprite_list);
		return sprite;
	}

	// try default sprite file
	if (entry_read_string(CHARACTER_TABLE, ctx->id, &sprite_name,
	CHARACTER_KEY_SPRITE, nullptr) == RET_OK)
	{
		if (sprite_name[0] != 0)
		{
			sprite_name_array[0] = sprite_name;
			sprite = imageDB_get_anim_array(player_context, sprite_name_array);
			free(sprite_name);
			return sprite;
		}
		free(sprite_name);
	}
	// try default sprite list
	if (entry_read_list(CHARACTER_TABLE, ctx->id, &sprite_list,
	CHARACTER_KEY_SPRITE, nullptr) == RET_OK)
	{
		sprite = imageDB_get_anim_array(player_context,
				(const char **) sprite_list);
		deep_free(sprite_list);
		return sprite;
	}

	werr(LOGDESIGNER, "Can't read sprite name for \"%s\"", ctx->id);
	return nullptr;
}

/**********************************
 Draw a single sprite
 if image_file_name is not nullptr, this file is used as an image rather than the normal sprite image
 **********************************/
static void set_up_sprite(context_t * ctx)
{
	anim_t ** sprite_list;
	item_t * item;
	int px;
	int py;
	Uint32 current_time;
	int angle;
	int flip;
	int force_flip;
	int move_status;
	char * zoom_str = nullptr;
	double zoom = 1.0;
	int sprite_align = ALIGN_CENTER;
	int sprite_offset_y = 0;
	bool force_position = false;

	context_t * player_context = context_get_player();

	if (ctx->map == nullptr)
	{
		return;
	}
	if (ctx->in_game == false)
	{
		return;
	}
	if (strcmp(ctx->map, player_context->map))
	{
		return;
	}

	item = item_list_add(&item_list);

	current_time = sdl_get_global_time();

	// Force position when the player has changed map
	if (change_map == true)
	{
		ctx->animation_tick = current_time;
		force_position = true;
	}
	// Force position when this context has changed map
	if (ctx->change_map == true)
	{
		ctx->animation_tick = current_time;
		ctx->change_map = false;
		force_position = true;
	}

	if (ctx->animation_tick == 0)
	{
		ctx->animation_tick = current_time;
	}

	// Detect sprite movement, initiate animation
	if (ctx->pos_changed && force_position == false)
	{
		ctx->pos_changed = false;

		/* flip need to remember previous direction to avoid resetting a
		 east -> west flip when a sprite goes to north for instance.
		 On the contrary rotation must not remember previous state, or
		 the rotation will be wrong.
		 Hence the distinction between orientation (no memory) and
		 direction (memory). */
		ctx->orientation = 0;
		// Compute direction
		if (ctx->pos_tx > ctx->prev_pos_tx)
		{
			ctx->direction &= ~WEST;
			ctx->direction |= EAST;
			ctx->orientation |= EAST;
		}
		if (ctx->pos_tx < ctx->prev_pos_tx)
		{
			ctx->direction &= ~EAST;
			ctx->direction |= WEST;
			ctx->orientation |= WEST;
		}
		if (ctx->pos_ty > ctx->prev_pos_ty)
		{
			ctx->direction &= ~NORTH;
			ctx->direction |= SOUTH;
			ctx->orientation |= SOUTH;
		}
		if (ctx->pos_ty < ctx->prev_pos_ty)
		{
			ctx->direction &= ~SOUTH;
			ctx->direction |= NORTH;
			ctx->orientation |= NORTH;
		}
	}

	// Select sprite to display
	sprite_list = select_sprite(ctx);
	if (sprite_list == nullptr)
	{
		return;
	}
	if (sprite_list[0] == nullptr)
	{
		free(sprite_list);
		return;
	}

	// Get position in pixel
	px = map_t2p_x(ctx->pos_tx, ctx->pos_ty, default_layer);
	py = map_t2p_y(ctx->pos_tx, ctx->pos_ty, default_layer);

	// Get per sprite zoom
	if (entry_read_string(CHARACTER_TABLE, ctx->id, &zoom_str,
	CHARACTER_KEY_ZOOM, nullptr) == RET_OK)
	{
		zoom = atof(zoom_str);
		free(zoom_str);
	}

	// Align sprite on tile
	entry_read_int(CHARACTER_TABLE, ctx->id, &sprite_align, CHARACTER_KEY_ALIGN,
			nullptr);
	if (sprite_align == ALIGN_CENTER)
	{
		px -= ((sprite_list[0]->w * default_layer->map_zoom * zoom)
				- default_layer->tile_width) / 2;
		py -= ((sprite_list[0]->h * default_layer->map_zoom * zoom)
				- default_layer->tile_height) / 2;
	}
	if (sprite_align == ALIGN_LOWER)
	{
		px -= ((sprite_list[0]->w * default_layer->map_zoom * zoom)
				- default_layer->tile_width) / 2;
		py -= (sprite_list[0]->h * default_layer->map_zoom * zoom)
				- default_layer->tile_height;
	}

	// Add Y offset
	entry_read_int(CHARACTER_TABLE, ctx->id, &sprite_offset_y,
	CHARACTER_KEY_OFFSET_Y, nullptr);
	py += sprite_offset_y;

	// Set sprite to item
	item_set_pos(item, px, py);
	item_set_anim_start_tick(item, ctx->animation_tick);
	item_set_anim_array(item, sprite_list);
	free(sprite_list);

	// Get rotation configuration
	angle = 0;
	if ((ctx->orientation & NORTH) && (ctx->orientation & EAST))
	{
		entry_read_int(CHARACTER_TABLE, ctx->id, &angle,
		CHARACTER_KEY_DIR_NE_ROT, nullptr);
		item_set_angle(item, (double) angle);
	}
	else if ((ctx->orientation & SOUTH) && (ctx->orientation & EAST))
	{
		entry_read_int(CHARACTER_TABLE, ctx->id, &angle,
		CHARACTER_KEY_DIR_SE_ROT, nullptr);
		item_set_angle(item, (double) angle);
	}
	else if ((ctx->orientation & SOUTH) && (ctx->orientation & WEST))
	{
		entry_read_int(CHARACTER_TABLE, ctx->id, &angle,
		CHARACTER_KEY_DIR_SW_ROT, nullptr);
		item_set_angle(item, (double) angle);
	}
	else if ((ctx->orientation & NORTH) && (ctx->orientation & WEST))
	{
		entry_read_int(CHARACTER_TABLE, ctx->id, &angle,
		CHARACTER_KEY_DIR_NW_ROT, nullptr);
		item_set_angle(item, (double) angle);
	}
	else if (ctx->orientation & NORTH)
	{
		entry_read_int(CHARACTER_TABLE, ctx->id, &angle,
		CHARACTER_KEY_DIR_N_ROT, nullptr);
		item_set_angle(item, (double) angle);
	}
	else if (ctx->orientation & SOUTH)
	{
		entry_read_int(CHARACTER_TABLE, ctx->id, &angle,
		CHARACTER_KEY_DIR_S_ROT, nullptr);
		item_set_angle(item, (double) angle);
	}
	else if (ctx->orientation & WEST)
	{
		entry_read_int(CHARACTER_TABLE, ctx->id, &angle,
		CHARACTER_KEY_DIR_W_ROT, nullptr);
		item_set_angle(item, (double) angle);
	}
	else if (ctx->orientation & EAST)
	{
		entry_read_int(CHARACTER_TABLE, ctx->id, &angle,
		CHARACTER_KEY_DIR_E_ROT, nullptr);
		item_set_angle(item, (double) angle);
	}

	// Get flip configuration
	force_flip = 0;
	entry_read_int(CHARACTER_TABLE, ctx->id, &force_flip,
	CHARACTER_KEY_FORCE_FLIP, nullptr);
	move_status = ctx->direction;
	if (force_flip == true)
	{
		move_status = ctx->orientation;
	}

	flip = 0;
	if (angle == 0)
	{
		if (move_status & NORTH)
		{
			entry_read_int(CHARACTER_TABLE, ctx->id, &flip,
			CHARACTER_KEY_DIR_N_FLIP, nullptr);
		}
		if (move_status & SOUTH)
		{
			entry_read_int(CHARACTER_TABLE, ctx->id, &flip,
			CHARACTER_KEY_DIR_S_FLIP, nullptr);
		}
		if (move_status & WEST)
		{
			entry_read_int(CHARACTER_TABLE, ctx->id, &flip,
			CHARACTER_KEY_DIR_W_FLIP, nullptr);
		}
		if (move_status & EAST)
		{
			entry_read_int(CHARACTER_TABLE, ctx->id, &flip,
			CHARACTER_KEY_DIR_E_FLIP, nullptr);
		}

		switch (flip)
		{
		case 1:
			item_set_flip(item, SDL_FLIP_HORIZONTAL);
			break;
		case 2:
			item_set_flip(item, SDL_FLIP_VERTICAL);
			break;
		case 3:
			item_set_flip(item, SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
			break;
		default:
			item_set_flip(item, SDL_FLIP_NONE);
		}
	}

	item_set_click_left(item, cb_select_sprite, ctx->id, nullptr);
	item_set_click_right(item, cb_redo_sprite, item, nullptr);

	item_set_zoom_x(item, zoom * default_layer->map_zoom);
	item_set_zoom_y(item, zoom * default_layer->map_zoom);

	item->user_ptr = ctx;
}

/**********************************
 Compose sprites
 **********************************/
static void compose_sprite(int layer_index)
{
	int layer;
	context_t * ctx = context_get_player();

	context_lock_list();

	while (ctx != nullptr)
	{
		layer = 0;
		entry_read_int(CHARACTER_TABLE, ctx->id, &layer, CHARACTER_LAYER,
				nullptr);

		if (layer == layer_index)
		{
			set_up_sprite(ctx);
		}
		ctx = ctx->next;
	}

	context_unlock_list();
}

/**********************************
 Compose item on map
 **********************************/
static void compose_item(int layer_index)
{
	char * sprite_name = nullptr;
	int sprite_align = ALIGN_CENTER;
	int sprite_offset_y = 0;
	anim_t * anim;
	item_t * item;
	int x;
	int y;
	int temp_x;
	int temp_y;
	char ** item_id;
	int i;
	static TTF_Font * font = nullptr;
	char * mytemplate;
	int quantity;
	char buf[SMALL_BUF];
	char layer_name[SMALL_BUF];
	context_t * ctx = context_get_player();

	sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer_index);

	if (entry_get_group_list(MAP_TABLE, ctx->map, &item_id, layer_name,
	MAP_ENTRY_ITEM_LIST, nullptr) == RET_NOK)
	{
		return;
	}

	font = font_get(ctx, ITEM_FONT, ITEM_FONT_SIZE);

	i = 0;
	while (item_id[i] != nullptr)
	{
		sprite_align = ALIGN_CENTER;

		if (entry_read_int(MAP_TABLE, ctx->map, &x, layer_name,
		MAP_ENTRY_ITEM_LIST, item_id[i], MAP_ITEM_POS_X, nullptr) == RET_NOK)
		{
			i++;
			continue;
		}

		if (entry_read_int(MAP_TABLE, ctx->map, &y, layer_name,
		MAP_ENTRY_ITEM_LIST, item_id[i], MAP_ITEM_POS_Y, nullptr) == RET_NOK)
		{
			i++;
			continue;
		}

		mytemplate = item_is_resource(item_id[i]);

		if (mytemplate == nullptr)
		{
			if (entry_read_string(ITEM_TABLE, item_id[i], &sprite_name,
			ITEM_SPRITE, nullptr) == RET_NOK)
			{
				i++;
				continue;
			}
			entry_read_int(ITEM_TABLE, item_id[i], &sprite_align, ITEM_ALIGN,
					nullptr);
			entry_read_int(ITEM_TABLE, item_id[i], &sprite_offset_y,
			ITEM_OFFSET_Y, nullptr);
		}
		else
		{
			if (entry_read_string(ITEM_TEMPLATE_TABLE, mytemplate, &sprite_name,
			ITEM_SPRITE, nullptr) == RET_NOK)
			{
				free(mytemplate);
				i++;
				continue;
			}
			entry_read_int(ITEM_TEMPLATE_TABLE, mytemplate, &sprite_align,
			ITEM_ALIGN, nullptr);
			entry_read_int(ITEM_TEMPLATE_TABLE, mytemplate, &sprite_offset_y,
			ITEM_OFFSET_Y, nullptr);
			free(mytemplate);
		}

		item = item_list_add(&item_list);

		anim = imageDB_get_anim(ctx, sprite_name);
		free(sprite_name);

		temp_x = map_t2p_x(x, y, default_layer);
		temp_y = map_t2p_y(x, y, default_layer);
		x = temp_x;
		y = temp_y;
		// Align on tile
		if (sprite_align == ALIGN_CENTER)
		{
			x -= ((anim->w * default_layer->map_zoom)
					- default_layer->tile_width) / 2;
			y -= ((anim->h * default_layer->map_zoom)
					- default_layer->tile_height) / 2;
		}
		if (sprite_align == ALIGN_LOWER)
		{
			x -= ((anim->w * default_layer->map_zoom)
					- default_layer->tile_width) / 2;
			y -= (anim->h * default_layer->map_zoom)
					- default_layer->tile_height;
		}

		y += sprite_offset_y;

		item_set_pos(item, x, y);
		item_set_anim(item, anim, 0);
		item_set_zoom_x(item, default_layer->map_zoom);
		item_set_zoom_y(item, default_layer->map_zoom);
		if (font)
		{
			quantity = resource_get_quantity(item_id[i]);
			sprintf(buf, "%d", quantity);
			item_set_string(item, buf);
			item_set_font(item, font);
		}

		i++;
	}

	deep_free(item_id);
}

/**************************************
 **************************************/
static void cb_select_map(void *arg)
{
	item_t * item = (item_t*) arg;
	context_t * ctx = context_get_player();
	char x[SMALL_BUF];
	char y[SMALL_BUF];

	sprintf(x, "%d", item->user1);
	sprintf(y, "%d", item->user2);

	network_send_action(ctx, option_get().action_select_tile, ctx->map, x, y,
			nullptr);
}

/**************************************
 **************************************/
static void cb_redo_map(void *arg)
{
	char * script = nullptr;
	char * last_action = nullptr;

	cb_select_map(arg);

	last_action = ui_play_get_last_action();
	if (last_action)
	{
		script = strdup(last_action);
		ui_play_cb_action(script);
		free(script);
	}
}

/**************************************
 **************************************/
static void cb_over(void *arg, int x, int y)
{
	item_t * item = (item_t*) arg;

	current_map_x = item->user1;
	current_map_y = item->user2;
}

/**********************************
 Set sdl_item item for mouse button callback
 **********************************/
static void compose_map_button()
{
	int x = 0;
	int y = 0;
	item_t * item;
	anim_t * anim = nullptr;

	context_t * ctx = context_get_player();

	if (option_get().cursor_over_tile)
	{
		anim = imageDB_get_anim(ctx, option_get().cursor_over_tile);
	}

	for (y = 0; y < default_layer->map_h; y++)
	{
		for (x = 0; x < default_layer->map_w; x++)
		{
			item = item_list_add(&item_list);
			item_set_anim_shape(item, map_t2p_x(x, y, default_layer),
					map_t2p_y(x, y, default_layer), default_layer->tile_width,
					default_layer->tile_height);
			item_set_user(item, x, y);
			item_set_click_left(item, cb_select_map, item, nullptr);
			item_set_click_right(item, cb_redo_map, item, nullptr);
			item_set_over(item, cb_over, item, nullptr);
			item_set_anim_over(item, anim, 0);
		}
	}
}

/**********************************
 Draw the "set" keyword of a layer
 **********************************/
static void compose_map_set(int layer_index)
{
	int i = 0;
	int x = 0;
	int y = 0;
	anim_t * anim;
	item_t * item;
	char ** tile_set = nullptr;
	char layer_name[SMALL_BUF];
	layer_t * layer;
	context_t * ctx = context_get_player();

	snprintf(layer_name, sizeof(layer_name), "%s%d", MAP_KEY_LAYER,
			layer_index);
	if (entry_read_list(MAP_TABLE, ctx->map, &tile_set, layer_name, MAP_KEY_SET,
			nullptr) == RET_NOK)
	{
		return;
	}

	layer = map_layer_new(ctx->map, layer_index, default_layer);
	if (layer == nullptr)
	{
		return;
	}

	while (tile_set[i] != nullptr)
	{
		// Skip empty tile
		if (tile_set[i][0] != 0)
		{
			item = item_list_add(&item_list);
			anim = imageDB_get_anim(ctx, tile_set[i]);
			item_set_pos(item, map_t2p_x(x, y, layer), map_t2p_y(x, y, layer));
			item_set_anim(item, anim, 0);
		}

		x++;
		if (x >= layer->map_w)
		{
			x = 0;
			y++;
		}
		i++;
	}

	deep_free(tile_set);

	map_layer_delete(layer);
}

/**********************************
 Draw the "list" keyword of a layer
 **********************************/
static void compose_map_scenery(int layer_index)
{
	int i = 0;
	int x = 0;
	int y = 0;
	char * image_name = nullptr;
	anim_t * anim;
	item_t * item;
	char ** scenery_list = nullptr;
	char layer_name[SMALL_BUF];
	context_t * ctx = context_get_player();

	sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer_index);
	if (entry_get_group_list(MAP_TABLE, ctx->map, &scenery_list, layer_name,
	MAP_KEY_SCENERY, nullptr) == RET_NOK)
	{
		return;
	}

	while (scenery_list[i] != nullptr)
	{
		if (entry_read_int(MAP_TABLE, ctx->map, &x, layer_name, MAP_KEY_SCENERY,
				scenery_list[i], MAP_KEY_SCENERY_X, nullptr) == RET_NOK)
		{
			i++;
			continue;
		}
		if (entry_read_int(MAP_TABLE, ctx->map, &y, layer_name, MAP_KEY_SCENERY,
				scenery_list[i], MAP_KEY_SCENERY_Y, nullptr) == RET_NOK)
		{
			i++;
			continue;
		}
		if (entry_read_string(MAP_TABLE, ctx->map, &image_name, layer_name,
		MAP_KEY_SCENERY, scenery_list[i], MAP_KEY_SCENERY_IMAGE,
				nullptr) == RET_NOK)
		{
			i++;
			continue;
		}

		anim = imageDB_get_anim(ctx, image_name);

		item = item_list_add(&item_list);
		item_set_pos(item, x, y);
		item_set_anim(item, anim, 0);
		//item_set_anim(item, x*ctx->tile_width, y*ctx->tile_height, anim,0);

		i++;
	}

	deep_free(scenery_list);
}

/**********************************
 Show tiles types
 **********************************/
static void compose_type(int layer_index)
{
	int x = 0;
	int y = 0;
	item_t * item;
	char * type;
	static TTF_Font * font = nullptr;
	int w;
	int h;
	char layer_name[SMALL_BUF];
	context_t * ctx = context_get_player();

	if (option_get().show_tile_type == false)
	{
		return;
	}

	sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer_index);
	if (entry_exist(MAP_TABLE, ctx->map, layer_name, MAP_KEY_TYPE, nullptr)
			== false)
	{
		return;
	}

	font = font_get(ctx, ITEM_FONT, ITEM_FONT_SIZE);

	for (x = 0; x < default_layer->map_w; x++)
	{
		for (y = 0; y < default_layer->map_h; y++)
		{
			if (entry_read_list_index(MAP_TABLE, ctx->map, &type,
					x + y * default_layer->map_w, layer_name, MAP_KEY_TYPE,
					nullptr) == RET_NOK)
			{
				continue;
			}

			if (type[0] == 0)
			{
				continue;
			}

			item = item_list_add(&item_list);

			item_set_string(item, type);
			item_set_font(item, font);
			sdl_get_string_size(item->font, item->string, &w, &h);
			item_set_anim_shape(item, map_t2p_x(x, y, default_layer),
					map_t2p_y(x, y, default_layer), w, h);
		}
	}
}

/**********************************
 Compose select cursor
 **********************************/
static void compose_select()
{
	item_t * item;
	anim_t * anim;
	int pos_tx;
	int pos_ty;
	int x;
	int y;
	context_t * ctx = context_get_player();

	// Tile selection
	if (option_get().cursor_tile)
	{
		if (ctx->selection.map[0] != 0)
		{
			if (!strcmp(ctx->selection.map, ctx->map))
			{
				pos_tx = ctx->selection.map_coord[0];
				pos_ty = ctx->selection.map_coord[1];

				if (pos_tx != -1 && pos_ty != -1)
				{
					anim = imageDB_get_anim(ctx, option_get().cursor_tile);

					item = item_list_add(&item_list);

					// get pixel coordinate from tile coordinate
					x = map_t2p_x(pos_tx, pos_ty, default_layer);
					y = map_t2p_y(pos_tx, pos_ty, default_layer);

					item_set_pos(item, x, y);
					item_set_anim(item, anim, 0);
				}
			}
		}
	}

	// Sprite selection
	if (option_get().cursor_character_draw_script)
	{
		if (ctx->selection.id[0] != 0)
		{
			context_t * selected_context = nullptr;
			selected_context = context_find(ctx->selection.id);
			if (selected_context == nullptr)
			{
				return;
			}

			item = item_list_add(&item_list);
			item->user_ptr = selected_context;
			item->user1_ptr = option_get().cursor_character_draw_script;
		}
	}
}

/**********************************
 **********************************/
void scr_play_frame_start(context_t * context)
{
}

/**********************************
 **********************************/
void scr_play_init()
{
	// Register this character to receive server notifications
	context_t * l_pCtx = context_get_player();
	network_request_start(l_pCtx, l_pCtx->id);
	ui_play_init();
}

/**********************************
 Compose the character select screen
 **********************************/
item_t * scr_play_compose(context_t * ctx)
{
	int bg_red = 0;
	int bg_blue = 0;
	int bg_green = 0;
	char * map_filename;
	int layer_index = 0;
	char * old_sfx = nullptr;

	if (item_list)
	{
		item_list_free(item_list);
		item_list = nullptr;
	}

	if (ctx->map == nullptr)
	{
		if (context_update_from_file(ctx) == RET_NOK)
		{
			return nullptr;
		}
	}

	sdl_free_keycb();
	sdl_free_mousecb();
	sdl_add_mousecb(MOUSE_WHEEL_UP, cb_zoom);
	sdl_add_mousecb(MOUSE_WHEEL_DOWN, cb_unzoom);

	change_map = ctx->change_map;

	if (change_map == true)
	{
		map_filename = strconcat(MAP_TABLE, "/", ctx->map, nullptr);
		network_send_req_file(ctx, map_filename);
		free(map_filename);
		if (default_layer != nullptr)
		{
			map_layer_delete(default_layer);
		}
		default_layer = map_layer_new(ctx->map, DEFAULT_LAYER, nullptr);
	}

	if (default_layer && default_layer->active)
	{ // Make sure map data are available
		for (layer_index = 0; layer_index < MAX_LAYER; layer_index++)
		{
			compose_map_set(layer_index);
			compose_map_scenery(layer_index);
			compose_item(layer_index);
			compose_sprite(layer_index);
			compose_type(layer_index);
		}
		compose_map_button();
		compose_select();

		ui_play_compose(ctx, item_list);
	}

	entry_read_int(MAP_TABLE, ctx->map, &bg_red, MAP_KEY_BG_RED, nullptr);
	entry_read_int(MAP_TABLE, ctx->map, &bg_blue, MAP_KEY_BG_BLUE, nullptr);
	entry_read_int(MAP_TABLE, ctx->map, &bg_green, MAP_KEY_BG_GREEN, nullptr);
	SDL_SetRenderDrawColor(ctx->render, bg_red, bg_blue, bg_green, 255);

	old_sfx = sfx;
	sfx = nullptr;

	entry_read_string(MAP_TABLE, ctx->map, &sfx, MAP_SFX, nullptr);

	if (old_sfx)
	{
		if (sfx)
		{
			if (strcmp(old_sfx, sfx))
			{
				sfx_stop(MUSIC_CHANNEL);
				g_IsMusicPlaying = false;
			}
		}
		else
		{ // sfx == nullptr
			sfx_stop(MUSIC_CHANNEL);
			g_IsMusicPlaying = false;
		}
		free(old_sfx);
	}

	if (sfx && sfx[0] != 0)
	{
		if (g_IsMusicPlaying == false)
		{
			if (sfx_play(ctx, std::string(sfx), MUSIC_CHANNEL, LOOP) != -1)
			{
				g_IsMusicPlaying = true;
			}
			int sfx_volume = 100; // 100%
			entry_read_int(MAP_TABLE, ctx->map, &sfx_volume, MAP_SFX_VOLUME,
					nullptr);
			sfx_set_volume(MUSIC_CHANNEL, sfx_volume);
		}
	}

	return item_list;
}

