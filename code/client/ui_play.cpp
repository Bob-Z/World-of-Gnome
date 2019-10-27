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

#include "anim.h"
#include "common.h"
#include "imageDB.h"
#include "item.h"
#include "network_client.h"
#include "option_client.h"
#include "scr_play.h"
#include "screen.h"
#include "sdl.h"
#include "textview.h"
#include <stack>
#include <vector>
#include <string>
#include "entry.h"
#include "syntax.h"
#include "font.h"
#include "util.h"
#include "log.h"
#include "const.h"
#include "protocol.h"

#define UI_MAIN		0
#define UI_INVENTORY	1
#define UI_POPUP	2

#define FONT "Ubuntu-C.ttf"
#define FONT_SIZE 30
#define TEXT_FONT "Ubuntu-C.ttf"
#define TEXT_FONT_SIZE 15
#define TEXT_TIMEOUT 5000 // Text display timeout
#define ITEM_FONT "Ubuntu-C.ttf"
#define ITEM_FONT_SIZE 15
#define SPEAK_FONT "Ubuntu-C.ttf"
#define SPEAK_FONT_SIZE 32

#define BACKGROUND_COLOR (0xFFFFFF40)

#define POPUP_TAG_IMAGE		"image"
#define POPUP_TAG_TEXT		"text"
#define POPUP_TAG_ACTION	"action"
#define POPUP_TAG_EOL		"eol"
#define POPUP_TAG_EOP		"eop"
#define POPUP_TAG_END		"popup_end"

static int current_ui = UI_MAIN;
static char * last_action = nullptr;
// main ui
static char ** attribute_string = nullptr;
static int action_bar_height;
static int attribute_height;
static constexpr size_t TEXT_BUFFER_SIZE = 2048;
static char text_buffer[TEXT_BUFFER_SIZE];
// inventory ui
static char ** inventory_list = nullptr;
// popup ui
#define MOUSE_WHEEL_SCROLL (20)
static std::stack<std::vector<std::string>> g_PopupFifo;
typedef struct action_param_tag
{
	char * action;
	char * param;
} action_param_t;
static int popup_offset = 0;

static int first_action = 0;
static int num_action = 0;

/**********************************
 **********************************/
static void draw_background(Context * ctx, item_t * item_list)
{
	static anim_t * bg_anim = nullptr;
	int sw;
	int sh;
	item_t * item;

	if (bg_anim)
	{
		si_anim_free(bg_anim);
	}
	SDL_GetRendererOutputSize(ctx->m_render, &sw, &sh);
	bg_anim = anim_create_color(ctx->m_render, sw, sh, BACKGROUND_COLOR);
	item = item_list_add(&item_list);
	item_set_pos(item, 0, 0);
	item_set_anim(item, bg_anim, 0);
	item_set_overlay(item, 1);
}

/**********************************
 **********************************/
void ui_play_set(int ui_type)
{
	current_ui = ui_type;
	screen_compose();
}

/**********************************
 **********************************/
int ui_play_get()
{
	return current_ui;
}

/**********************************
 **********************************/
char * ui_play_get_last_action()
{
	return last_action;
}

/****************************
 ****************************/
static void cb_main_quit(void * arg)
{
	Context * current_ctx;
	Context * next_ctx;

	if (ui_play_get() == UI_MAIN)
	{
		context_set_in_game(context_get_player(), false);
		network_request_stop(context_get_player());
		current_ctx = context_get_first();
		while (current_ctx != nullptr)
		{
			// Save next before freeing the current context
			next_ctx = current_ctx->m_next;
			if (current_ctx != context_get_player())
			{
				context_free(current_ctx);
			}
			current_ctx = next_ctx;
		}

		screen_set_screen(Screen::SELECT);
	}
}

/****************************
 ****************************/
static void key_up(void * arg)
{
	Context * ctx = context_get_player();

	network_send_action(ctx, option_get().action_move_up, nullptr);
}

/**************************************
 **************************************/
static void key_down(void * arg)
{
	Context * ctx = context_get_player();

	network_send_action(ctx, option_get().action_move_down, nullptr);
}

/**************************************
 **************************************/
static void key_left(void * arg)
{
	Context * ctx = context_get_player();

	network_send_action(ctx, option_get().action_move_left, nullptr);
}

/**************************************
 **************************************/
static void key_right(void * arg)
{
	Context * ctx = context_get_player();

	network_send_action(ctx, option_get().action_move_right, nullptr);
}

/**************************************
 **************************************/
static void key_up_left(void * arg)
{
	Context * ctx = context_get_player();

	network_send_action(ctx, option_get().action_move_up_left, nullptr);
}

/**************************************
 **************************************/
static void key_up_right(void * arg)
{
	Context * ctx = context_get_player();

	network_send_action(ctx, option_get().action_move_up_right, nullptr);
}

/**************************************
 **************************************/
static void key_down_left(void * arg)
{
	Context * ctx = context_get_player();

	network_send_action(ctx, option_get().action_move_down_left, nullptr);
}

/**************************************
 **************************************/
static void key_down_right(void * arg)
{
	Context * ctx = context_get_player();

	network_send_action(ctx, option_get().action_move_down_right, nullptr);
}

/**********************************
 Compose attribute
 **********************************/
static void compose_attribute(Context * ctx, item_t * item_list)
{
	item_t * item;
	char ** name_list;
	int index = 0;
	int value;
	int y = 0;
	int num_attr = 0;
	char buf[1024];
	int w, h;
	static TTF_Font * font = nullptr;

	font = font_get(ctx, FONT, FONT_SIZE);

	if (attribute_string)
	{
		index = 0;
		while (attribute_string[index])
		{
			free(attribute_string[index]);
			attribute_string[index] = nullptr;
			index++;
		}
		free(attribute_string);
		attribute_string = nullptr;
	}

	if (entry_get_group_list(CHARACTER_TABLE, ctx->m_id, &name_list,
	ATTRIBUTE_GROUP, nullptr) == RET_NOK)
	{
		return;
	}

	index = 0;
	while (name_list[index] != nullptr)
	{
		if (entry_read_int(CHARACTER_TABLE, ctx->m_id, &value, ATTRIBUTE_GROUP, name_list[index], ATTRIBUTE_CURRENT, nullptr) == RET_NOK)
		{
			index++;
			continue;
		}

		num_attr++;
		attribute_string = (char**) realloc(attribute_string, (num_attr + 1) * sizeof(char*));
		sprintf(buf, "%s: %d", name_list[index], value);
		attribute_string[num_attr - 1] = strdup(buf);
		attribute_string[num_attr] = nullptr;

		item = item_list_add(&item_list);

		item_set_overlay(item, 1);
		item_set_string(item, attribute_string[num_attr - 1]);
		item_set_font(item, font);
		sdl_get_string_size(item->font, item->string, &w, &h);
		item_set_anim_shape(item, 0, y, w, h);
		y += h;
		if (attribute_height < y)
		{
			attribute_height = y;
		}

		index++;
	}

	deep_free(name_list);
}

/**********************************
 **********************************/
void ui_play_cb_action(void * arg)
{
	char * action = (char *) arg;

	network_send_action(context_get_player(), action, nullptr);

	if (last_action)
	{
		free(last_action);
		last_action = nullptr;
	}

	if (arg)
	{
		last_action = (char*) strdup(action);
	}
	else
	{
		last_action = nullptr;
	}
}

/**********************************
 **********************************/
static void cb_wheel_up_action(void* not_used)
{
	first_action--;
	if (first_action < 0)
	{
		first_action = 0;
		return;
	}
	screen_compose();
}

/**********************************
 **********************************/
static void cb_wheel_down_action(void* not_used)
{
	first_action++;
	screen_compose();
}

/**********************************
 Compose action icon
 **********************************/
static void compose_action(Context * ctx, item_t * item_list)
{
	char ** action_list = nullptr;
	char * text = nullptr;
	char ** icon = nullptr;
	char * icon_array[2] =
	{ nullptr, nullptr };
	char ** icon_over = nullptr;
	char ** icon_click = nullptr;
	char * script = nullptr;
	anim_t ** anim_array;
	item_t * item;
	int sw = 0;
	int sh = 0;
	int x = 0;
	int i;

	SDL_GetRendererOutputSize(ctx->m_render, &sw, &sh);

	action_bar_height = 0;

	// Read action list for current user
	if (entry_read_list(CHARACTER_TABLE, ctx->m_id, &action_list,
	CHARACTER_KEY_ACTION, nullptr) == RET_NOK)
	{
		return;
	}

	i = 0;
	while (action_list[i] != nullptr)
	{
		if (text)
		{
			free(text);
			text = nullptr;
		}
		if (script)
		{
			free(script);
			script = nullptr;
		}

		if (i < first_action)
		{
			if (action_list[i + 1] != nullptr)
			{
				i++;
				continue;
			}
			first_action = i;
		}

		if (entry_read_string(ACTION_TABLE, action_list[i], &text,
		ACTION_KEY_TEXT, nullptr) == RET_NOK)
		{
			i++;
			continue;
		}
		if (entry_read_string(ACTION_TABLE, action_list[i], &icon_array[0],
		ACTION_KEY_ICON, nullptr) == RET_OK)
		{
			icon = icon_array;
		}
		else if (entry_read_list(ACTION_TABLE, action_list[i], &icon,
		ACTION_KEY_ICON, nullptr) == RET_NOK)
		{
			i++;
			continue;
		}

		SiLayout layout = SiLayout::TOP_LEFT;
		entry_read_int(ACTION_TABLE, action_list[i], (int*) &layout,
		ACTION_KEY_ICON_LAYOUT, nullptr);

		item = item_list_add(&item_list);
		item_set_layout(item, layout);
		item_set_overlay(item, 1);
		item_set_click_left(item, ui_play_cb_action, (void*) strdup(action_list[i]), free);
		item_set_wheel_up(item, cb_wheel_up_action, nullptr, nullptr);
		item_set_wheel_down(item, cb_wheel_down_action, nullptr, nullptr);

		// load image
		anim_array = imageDB_get_anim_array(ctx, (const char **) icon);
		if (icon_array[0] == nullptr)
		{
			deep_free(icon);
		}

		item_set_pos(item, x, sh - anim_array[0]->h);
		item_set_anim_array(item, anim_array);

		// calculate next icon start X
		x += anim_array[0]->w;
		if (action_bar_height < anim_array[0]->h)
		{
			action_bar_height = anim_array[0]->h;
		}

		free(anim_array);

		if (entry_read_list(ACTION_TABLE, action_list[i], &icon_over,
		ACTION_KEY_ICON_OVER, nullptr) == RET_OK)
		{
			anim_array = imageDB_get_anim_array(ctx, (const char **) icon_over);
			item_set_anim_over_array(item, anim_array);
			free(anim_array);
			deep_free(icon_over);
		}

		if (entry_read_list(ACTION_TABLE, action_list[i], &icon_click,
		ACTION_KEY_ICON_CLICK, nullptr) == RET_OK)
		{
			anim_array = imageDB_get_anim_array(ctx, (const char **) icon_click);
			item_set_anim_click_array(item, anim_array);
			free(anim_array);
			deep_free(icon_click);
		}

		i++;
	}

	num_action = first_action + i;

	deep_free(action_list);

	if (text)
	{
		free(text);
		text = nullptr;
	}
	if (script)
	{
		free(script);
		script = nullptr;
	}
}

/**********************************
 **********************************/
static void cb_select_slot(void * arg)
{
	char * id = (char*) arg;
	Context * ctx = context_get_player();

	network_send_action(ctx, option_get().action_select_equipment, id, nullptr);
}

/**************************************
 **************************************/
static void show_inventory(void * arg)
{
	ui_play_set(UI_INVENTORY);
}

/**********************************
 Compose equipment icon
 **********************************/
static void compose_equipment(Context * ctx, item_t * item_list)
{
	char ** slot_list = nullptr;
	anim_t * anim;
	anim_t * anim2;
	anim_t * anim3;
	item_t * item;
	int sw = 0;
	int sh = 0;
	int y = 0;
	int x = 0;
	int h1;
	int index;
	char * mytemplate = nullptr;
#if 0
	char * name;
#endif
	char * icon_name = nullptr;
	char * equipped_name = nullptr;
#if 0
	char * equipped_text = nullptr;
#endif
	char * equipped_icon_name = nullptr;
	char * inventory_icon_name = nullptr;
	static anim_t * inventory_icon = nullptr;
	int max_h;
	int max_w;

	SDL_GetRendererOutputSize(ctx->m_render, &sw, &sh);

	entry_get_group_list(CHARACTER_TABLE, ctx->m_id, &slot_list, EQUIPMENT_GROUP, nullptr);

	max_w = 0;
	max_h = 0;
	index = 0;
	while (slot_list && slot_list[index] != nullptr)
	{
#if 0
		// Get the slot name
		if(entry_read_string(CHARACTER_TABLE,ctx->m_id,&item_name,EQUIPMENT_GROUP,slot_list[index],EQUIPMENT_NAME,nullptr) == RET_NOK )
		{
			name = strdup(slot_list[index]);
		}
		else
		{
			name = item_name;
		}
		free(item_name);
#endif
		h1 = 0;
		// Get the slot icon
		if (entry_read_string(CHARACTER_TABLE, ctx->m_id, &icon_name,
		EQUIPMENT_GROUP, slot_list[index], EQUIPMENT_ICON, nullptr) == RET_NOK)
		{
			continue;
		}
		else
		{
			// load image
			anim = imageDB_get_anim(ctx, icon_name);
			free(icon_name);

			item = item_list_add(&item_list);

			x = sw - anim->w;
			h1 = anim->h;
			item_set_overlay(item, 1);
			item_set_pos(item, x, y);
			item_set_anim(item, anim, 0);

			item_set_click_left(item, cb_select_slot, strdup(slot_list[index]), nullptr);

			if (anim->w > max_w)
			{
				max_w = anim->w;
			}
			if (anim->h > max_h)
			{
				max_h = anim->h;
			}
		}

		// Is there an equipped object ?
		if (entry_read_string(CHARACTER_TABLE, ctx->m_id, &equipped_name,
		EQUIPMENT_GROUP, slot_list[index], EQUIPMENT_EQUIPPED, nullptr) == RET_OK && equipped_name[0] != 0)
		{
#if 0
			// Get the equipped object name
			if(entry_read_string(ITEM_TABLE,equipped_name,&equipped_text,ITEM_NAME,nullptr) == RET_NOK )
			{
				werr(LOGDESIGNER,"Can't read object %s name in equipment slot %s",equipped_name,slot_list[index]);
			}
			free(equipped_text);
#endif
			// Get its icon
			mytemplate = item_is_resource(equipped_name);

			if (mytemplate == nullptr)
			{
				if (entry_read_string(ITEM_TABLE, equipped_name, &equipped_icon_name, ITEM_ICON, nullptr) == RET_NOK)
				{
					werr(LOGDESIGNER, "Can't read object %s icon in equipment slot %s", equipped_name, slot_list[index]);
				}
			}
			else
			{
				if (entry_read_string(ITEM_TEMPLATE_TABLE, mytemplate, &equipped_icon_name, ITEM_ICON, nullptr) == RET_NOK)
				{
					werr(LOGDESIGNER, "Can't read item %s icon name (template: %s)", equipped_name, mytemplate);
				}
				free(mytemplate);
			}

			if (equipped_icon_name)
			{
				item = item_list_add(&item_list);

				anim2 = imageDB_get_anim(ctx, equipped_icon_name);
				free(equipped_icon_name);

				item_set_overlay(item, 1);
				item_set_pos(item, x - anim->w, y);
				item_set_anim(item, anim2, 0);
				item_set_click_left(item, cb_select_slot, strdup(slot_list[index]), nullptr);
				if (h1 < anim->h)
				{
					h1 = anim->h;
				}
			}
			free(equipped_name);
		}

		// Draw selection cursor
		if (ctx->m_selection.getEquipment() != "")
		{
			if (option_get().cursor_equipment)
			{
				if (strcmp(ctx->m_selection.getEquipment().c_str(), slot_list[index]) == 0)
				{
					anim3 = imageDB_get_anim(ctx, option_get().cursor_equipment);

					item = item_list_add(&item_list);

					// Center on icon
					item_set_overlay(item, 1);
					item_set_pos(item, x - (anim3->w - anim->w) / 2, y - (anim3->h - anim->w) / 2);
					item_set_anim(item, anim3, 0);
				}
			}
		}

		if (h1 > anim->h)
		{
			y += h1;
		}
		else
		{
			y += anim->h;
		}

		index++;
	}
	deep_free(slot_list);

	// Draw selected item
	if (ctx->m_selection.getInventory() != "")
	{
		mytemplate = item_is_resource(ctx->m_selection.getInventory().c_str());

		if (mytemplate == nullptr)
		{
			if (entry_read_string(ITEM_TABLE, ctx->m_selection.getInventory().c_str(), &inventory_icon_name, ITEM_ICON, nullptr) == RET_NOK)
			{
				werr(LOGDESIGNER, "Can't read item %s icon name", ctx->m_selection.getInventory().c_str());
			}
		}
		else
		{
			if (entry_read_string(ITEM_TEMPLATE_TABLE, mytemplate, &inventory_icon_name, ITEM_ICON, nullptr) == RET_NOK)
			{
				werr(LOGDESIGNER, "Can't read item %s icon name (template: %s)", ctx->m_selection.getInventory().c_str(), mytemplate);
			}
			free(mytemplate);
		}

		if (inventory_icon_name)
		{
			item = item_list_add(&item_list);

			anim = imageDB_get_anim(ctx, inventory_icon_name);
			free(inventory_icon_name);

			item_set_overlay(item, 1);
			item_set_pos(item, sw - anim->w, y);
			item_set_anim(item, anim, 0);
			item_set_click_left(item, show_inventory, nullptr, nullptr);
		}
	}
	else
	{
		if (max_w == 0)
		{
			max_w = 32;
		}
		if (max_h == 0)
		{
			max_h = 32;
		}

		if (inventory_icon == nullptr)
		{
			inventory_icon = anim_create_color(ctx->m_render, max_w, max_h, 0x7f7f7f7f);
		}

		item = item_list_add(&item_list);

		item_set_overlay(item, 1);
		item_set_pos(item, sw - inventory_icon->w, y);
		item_set_anim(item, inventory_icon, 0);
		item_set_click_left(item, show_inventory, nullptr, nullptr);
	}
}

/**************************************
 **************************************/
static void keyboard_text(void * arg)
{
	const char * text = (const char*) arg;

	network_send_action(context_get_player(), WOG_CHAT, text, nullptr);
	text_buffer[0] = 0;
	screen_compose();
}

/**********************************
 Compose text
 **********************************/
static void compose_text(Context * ctx, item_t * item_list)
{
	const history_entry_t * history;
	history_entry_t * hist;
	Uint32 time = SDL_GetTicks();
	int sw;
	int sh;
	int current_y;
	static TTF_Font * font = nullptr;
	item_t * item;
	int w;
	int h;
	int x;
	int y;

	font = font_get(ctx, TEXT_FONT, TEXT_FONT_SIZE);

	SDL_GetRendererOutputSize(ctx->m_render, &sw, &sh);
	current_y = sh - action_bar_height;

	// Draw edit box
	item = item_list_add(&item_list);

	item_set_overlay(item, 1);
	item_set_buffer(item, text_buffer, TEXT_BUFFER_SIZE);
	item_set_string_bg(item, BACKGROUND_COLOR);
	item_set_font(item, font);
	item_set_editable(item, 1);
	item_set_edit_cb(item, keyboard_text);
	sdl_get_string_size(item->font, item->string, &w, &h);
	x = w;
	if (w < 100)
	{
		x = 100;
	}
	y = h;
	if (y < TEXT_FONT_SIZE)
	{
		y = TEXT_FONT_SIZE;
	}
	item_set_anim_shape(item, 0, current_y - y, x, y);
	current_y -= y;
	if (attribute_height > current_y)
	{
		return;
	}

	/* Draw text history */
	history = textview_get_history();

	if (history == nullptr)
	{
		return;
	}

	hist = (history_entry_t*) history;

	while (hist)
	{
		if (time > hist->time + TEXT_TIMEOUT)
		{
			return;
		}

		item = item_list_add(&item_list);

		item_set_overlay(item, 1);
		item_set_string(item, hist->text);
		item_set_string_bg(item, BACKGROUND_COLOR);
		item_set_font(item, font);
		sdl_get_string_size(item->font, item->string, &w, &h);
		item_set_anim_shape(item, 0, current_y - h, w, h);
		current_y -= h;
		if (attribute_height > current_y)
		{
			return;
		}

		hist = hist->next;
	}
}

/****************************
 ****************************/
static void cb_print_coord(void * arg)
{
	char buf[SMALL_BUF];
	int map_w;
	Context * ctx = context_get_player();

	entry_read_int(MAP_TABLE, ctx->m_map, &map_w, MAP_KEY_WIDTH, nullptr);

//TODO: take layer into account
#if 0
	entry_read_list_index(MAP_TABLE,ctx->m_map,&type,scr_play_get_current_x()+scr_play_get_current_y()*map_w,layer_name,MAP_KEY_TYPE,nullptr);
	sprintf(buf,"x=%d y=%d type=%s",scr_play_get_current_x(),scr_play_get_current_y(),type);
	free(type);
#endif
	sprintf(buf, "x=%d y=%d", scr_play_get_current_x(), scr_play_get_current_y());
	textview_add_line(std::string(buf));

	screen_compose();
}

/**********************************
 **********************************/
static void main_compose(Context * ctx, item_t * item_list)
{
	compose_attribute(ctx, item_list);
	compose_action(ctx, item_list);
	compose_equipment(ctx, item_list);
	compose_text(ctx, item_list);

	sdl_add_keycb(SDL_SCANCODE_I, show_inventory, nullptr, nullptr);
	sdl_add_keycb(SDL_SCANCODE_UP, key_up, nullptr, nullptr);
	sdl_add_keycb(SDL_SCANCODE_KP_8, key_up, nullptr, nullptr);
	sdl_add_keycb(SDL_SCANCODE_DOWN, key_down, nullptr, nullptr);
	sdl_add_keycb(SDL_SCANCODE_KP_2, key_down, nullptr, nullptr);
	sdl_add_keycb(SDL_SCANCODE_LEFT, key_left, nullptr, nullptr);
	sdl_add_keycb(SDL_SCANCODE_KP_4, key_left, nullptr, nullptr);
	sdl_add_keycb(SDL_SCANCODE_RIGHT, key_right, nullptr, nullptr);
	sdl_add_keycb(SDL_SCANCODE_KP_6, key_right, nullptr, nullptr);
	sdl_add_keycb(SDL_SCANCODE_KP_7, key_up_left, nullptr, nullptr);
	sdl_add_keycb(SDL_SCANCODE_KP_9, key_up_right, nullptr, nullptr);
	sdl_add_keycb(SDL_SCANCODE_KP_1, key_down_left, nullptr, nullptr);
	sdl_add_keycb(SDL_SCANCODE_KP_3, key_down_right, nullptr, nullptr);
	sdl_add_keycb(SDL_SCANCODE_ESCAPE, cb_main_quit, nullptr, nullptr);
	sdl_add_keycb(SDL_SCANCODE_SCROLLLOCK, cb_print_coord, nullptr, nullptr);
}

/****************************
 ****************************/
static void cb_inventory_quit(void * arg)
{
	ui_play_set(UI_MAIN);
}

/****************************
 ****************************/
void cb_inventory_select(void * arg)
{
	char * item_id = (char *) arg;
	Context * ctx = context_get_player();

	network_send_action(ctx, option_get().action_select_inventory, item_id, nullptr);
}

/**********************************
 Compose inventory
 **********************************/
static void compose_inventory(Context * ctx, item_t * item_list)
{
	char * value = nullptr;
	char * label;
	char * description;
	anim_t * anim;
	item_t * item;
	int x = 0;
	int i = 0;
	static TTF_Font * font = nullptr;
	char * mytemplate;
	int quantity;
	char buf[1024];
	int w;
	int h;

	font = font_get(ctx, ITEM_FONT, ITEM_FONT_SIZE);

	deep_free(inventory_list);

	draw_background(ctx, item_list);

	// read data from file
	if (entry_read_list(CHARACTER_TABLE, ctx->m_id, &inventory_list,
	CHARACTER_KEY_INVENTORY, nullptr) == RET_NOK)
	{
		return;
	}

	while (inventory_list[i] != nullptr)
	{
		mytemplate = item_is_resource(inventory_list[i]);

		if (mytemplate == nullptr)
		{
			// Icon is mandatory for now
			if (entry_read_string(ITEM_TABLE, inventory_list[i], &value,
			ITEM_ICON, nullptr) == RET_NOK)
			{
				i++;
				continue;
			}
			// load image
			anim = imageDB_get_anim(ctx, value);
			free(value);

			if (entry_read_string(ITEM_TABLE, inventory_list[i], &value,
			ITEM_NAME, nullptr) == RET_NOK)
			{
				label = strdup(inventory_list[i]);
			}
			else
			{
				label = value;
			}

			if (entry_read_string(ITEM_TABLE, inventory_list[i], &value,
			ITEM_DESC, nullptr) == RET_NOK)
			{
				description = strdup("");
				;
			}
			else
			{
				description = value;
			}
		}
		else
		{
			// Icon is mandatory for now
			if (entry_read_string(ITEM_TEMPLATE_TABLE, mytemplate, &value,
			ITEM_ICON, nullptr) == RET_NOK)
			{
				i++;
				free(mytemplate);
				continue;
			}
			// load image
			anim = imageDB_get_anim(ctx, value);
			free(value);

			if (entry_read_string(ITEM_TEMPLATE_TABLE, mytemplate, &value,
			ITEM_NAME, nullptr) == RET_NOK)
			{
				label = strdup(inventory_list[i]);
			}
			else
			{
				label = value;
			}

			if (entry_read_string(ITEM_TEMPLATE_TABLE, mytemplate, &value,
			ITEM_DESC, nullptr) == RET_NOK)
			{
				description = strdup("");
				;
			}
			else
			{
				description = value;
			}
			free(mytemplate);

		}

		quantity = resource_get_quantity(inventory_list[i]);

		if (quantity > 0)
		{
			w = 0;
			item = item_list_add(&item_list);
			item_set_pos(item, x, 0);
			item_set_anim(item, anim, 0);
			if (quantity > 1)
			{
				sprintf(buf, "%d", quantity);
				item_set_string(item, buf);
				item_set_string_bg(item, BACKGROUND_COLOR);
				item_set_font(item, font);
				sdl_get_string_size(item->font, item->string, &w, &h);
			}
			item_set_overlay(item, 1);
			if (w > anim->w)
			{
				x += w;
			}
			else
			{
				x += anim->w;
			}
			item_set_click_left(item, cb_inventory_select, (void*) strdup(inventory_list[i]), free);
		}

		free(description);
		free(label);
		i++;
	}
}

/**********************************
 Compose select cursor
 **********************************/
static void compose_inventory_select(Context * ctx, item_t * item_list)
{
	item_t * item = nullptr;
	int x = -1;
	int i = 0;
	char * icon_name = nullptr;
	anim_t * anim = nullptr;
	anim_t * icon_anim = nullptr;
	char * mytemplate = nullptr;

	if (ctx->m_selection.getInventory() == "")
	{
		return;
	}

	if (item_list == nullptr)
	{
		return;
	}

	if (option_get().cursor_inventory == nullptr)
	{
		return;
	}

	anim = imageDB_get_anim(ctx, option_get().cursor_inventory);

	deep_free(inventory_list);

	// read data from file
	if (entry_read_list(CHARACTER_TABLE, ctx->m_id, &inventory_list,
	CHARACTER_KEY_INVENTORY, nullptr) == RET_NOK)
	{
		return;
	}

	i = 0;
	x = 0;
	while (inventory_list[i] && strcmp(inventory_list[i], ctx->m_selection.getInventory().c_str()))
	{
		mytemplate = item_is_resource(inventory_list[i]);

		if (mytemplate == nullptr)
		{
			if (entry_read_string(ITEM_TABLE, inventory_list[i], &icon_name,
			ITEM_ICON, nullptr) == RET_NOK)
			{
				werr(LOGDESIGNER, "Can't read item %s icon name", inventory_list[i]);
			}
		}
		else
		{
			if (entry_read_string(ITEM_TEMPLATE_TABLE, mytemplate, &icon_name,
			ITEM_ICON, nullptr) == RET_NOK)
			{
				werr(LOGDESIGNER, "Can't read item %s icon name (template: %s)", inventory_list[i], mytemplate);
			}
			free(mytemplate);
		}

		icon_anim = imageDB_get_anim(ctx, icon_name);
		free(icon_name);

		x += icon_anim->w;
		i++;
	}

	if (inventory_list[i])
	{
		item = item_list_add(&item_list);
		item_set_pos(item, x, 0);
		item_set_anim(item, anim, 0);
		item_set_overlay(item, 1);
	}
}

/**********************************
 **********************************/
static void inventory_compose(Context * ctx, item_t * item_list)
{
	compose_inventory(ctx, item_list);
	compose_inventory_select(ctx, item_list);

	sdl_add_keycb(SDL_SCANCODE_I, cb_inventory_quit, nullptr, nullptr);
	sdl_add_keycb(SDL_SCANCODE_ESCAPE, cb_inventory_quit, nullptr, nullptr);
	sdl_add_keycb(SDL_SCANCODE_SPACE, cb_inventory_quit, nullptr, nullptr);
}

/****************************
 ****************************/
static void cb_popup_quit(void * arg)
{
	popup_offset = 0;

	g_PopupFifo.pop();

	if (g_PopupFifo.size() == 0)
	{
		ui_play_set(UI_MAIN);
	}
}

/****************************
 ****************************/
void cb_popup(void * arg)
{
	Context * player = context_get_player();
	action_param_t * action_param = (action_param_t *) arg;

	cb_popup_quit(nullptr);

	network_send_action(player, action_param->action, action_param->param, nullptr);

	screen_compose();
}

/****************************
 ****************************/
void cb_free_action_param(void * arg)
{
	action_param_t * action_param = (action_param_t *) arg;

	free(action_param->action);
	free(action_param->param);
	free(action_param);
}

/**********************************
 **********************************/
static void cb_wheel_up(Uint32 y, Uint32 unused)
{
	popup_offset -= MOUSE_WHEEL_SCROLL;
	if (popup_offset < 0)
	{
		popup_offset = 0;
	}
	screen_compose();
}

/**********************************
 **********************************/
static void cb_wheel_down(Uint32 y, Uint32 unused)
{
	popup_offset += MOUSE_WHEEL_SCROLL;
	screen_compose();
}

/**********************************
 Compose screen
 **********************************/
static void compose_popup(Context * ctx, item_t * item_list)
{
	if (g_PopupFifo.size() == 0)
	{
		return;
	}

	draw_background(ctx, item_list);

	static TTF_Font *l_pFont = font_get(ctx, SPEAK_FONT, SPEAK_FONT_SIZE);

	sdl_free_mousecb();
	sdl_add_mousecb(MOUSE_WHEEL_UP, cb_wheel_up);
	sdl_add_mousecb(MOUSE_WHEEL_DOWN, cb_wheel_down);

	std::vector<std::string> l_PopupData = g_PopupFifo.top();

	item_t * l_pItem = nullptr;
	int l_X = 0;
	int l_Y = 0;
	int l_Width = 0;
	int l_Height = 0;
	int l_MaxHeight = 0;
	action_param_t * l_pActionParam = nullptr;

	for (auto l_It = l_PopupData.cbegin(); l_It != l_PopupData.cend(); ++l_It)
	{
		if (*l_It == POPUP_TAG_IMAGE)
		{
			++l_It;
			anim_t * l_pAnim = imageDB_get_anim(ctx, l_It->c_str());

			l_pItem = item_list_add(&item_list);
			item_set_pos(l_pItem, l_X, l_Y - popup_offset);
			item_set_anim(l_pItem, l_pAnim, 0);
			item_set_overlay(l_pItem, 1);
			if (l_pActionParam != nullptr)
			{
				item_set_click_left(l_pItem, cb_popup, l_pActionParam, cb_free_action_param);
				l_pActionParam = nullptr;
			}
			l_X += l_pAnim->w;
			if (l_MaxHeight < l_pAnim->h)
			{
				l_MaxHeight = l_pAnim->h;
			}
			continue;
		}
		if (*l_It == POPUP_TAG_TEXT)
		{
			++l_It;
			l_pItem = item_list_add(&item_list);
			item_set_string(l_pItem, l_It->c_str());
			item_set_font(l_pItem, l_pFont);
			sdl_get_string_size(l_pItem->font, l_pItem->string, &l_Width, &l_Height);
			item_set_anim_shape(l_pItem, l_X, l_Y - popup_offset, l_Width, l_Height);
			item_set_overlay(l_pItem, 1);
			if (l_pActionParam != nullptr)
			{
				item_set_click_left(l_pItem, cb_popup, l_pActionParam, cb_free_action_param);
				l_pActionParam = nullptr;
			}
			l_X += l_Width;
			if (l_MaxHeight < l_Height)
			{
				l_MaxHeight = l_Height;
			}
			continue;
		}
		if (*l_It == POPUP_TAG_ACTION)
		{
			l_pActionParam = (action_param_t*) malloc(sizeof(action_param_t));
			// get action
			++l_It;
			l_pActionParam->action = strdup(l_It->c_str());
			// get param
			++l_It;
			l_pActionParam->param = strdup(l_It->c_str());
			continue;
		}
		if (*l_It == POPUP_TAG_EOL)
		{
			l_Y += l_MaxHeight;
			l_MaxHeight = 0;
			l_X = 0;
		}
		if (*l_It == POPUP_TAG_EOP)
		{
			l_Y += l_MaxHeight;
			l_MaxHeight = 0;
			l_X = 0;
		}
	}
}

/**********************************
 **********************************/
void ui_play_popup_add(const std::vector<std::string> & data)
{
	g_PopupFifo.push(data);

	ui_play_set(UI_POPUP);
}

/**********************************
 **********************************/
static void popup_compose(Context * ctx, item_t * item_list)
{
	compose_popup(ctx, item_list);

	sdl_add_keycb(SDL_SCANCODE_ESCAPE, cb_popup_quit, nullptr, nullptr);
	sdl_add_keycb(SDL_SCANCODE_SPACE, cb_popup_quit, nullptr, nullptr);
}
/**********************************
 **********************************/
void ui_play_compose(Context * ctx, item_t * item_list)
{
	switch (current_ui)
	{
	case UI_MAIN:
		main_compose(ctx, item_list);
		break;
	case UI_INVENTORY:
		inventory_compose(ctx, item_list);
		break;
	case UI_POPUP:
		popup_compose(ctx, item_list);
		break;
	default:
		werr(LOGUSER, "Wrong UI type");
		break;
	}
}
/**********************************
 Called once
 **********************************/
void ui_play_init()
{
	// Empty text buffer
	text_buffer[0] = 0;
}
