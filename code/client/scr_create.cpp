/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2017-2020 carabobz@gmail.com

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
#include "Camera.h"
#include "entry.h"
#include "file.h"
#include "font.h"
#include "imageDB.h"
#include "item.h"
#include "log.h"
#include "mutex.h"
#include "network_client.h"
#include "screen.h"
#include "sdl.h"
#include "sfx.h"
#include "syntax.h"
#include "util.h"

static const constexpr int BORDER = 20;
static const constexpr int FONT_SIZE = 30;
static const std::string FONT = "Ubuntu-C.ttf";
static constexpr unsigned long BACKGROUND_COLOR = 0xFFFFFF40U;
static bool g_IsMusicPlaying = false;

typedef struct
{
	char * id;
	char * name;
	char * type;
	anim_t ** anim;
	item_t * item;
	int width;
} character_t;

static character_t * character_list = nullptr;
static int character_num = 0;
static item_t * g_itemList = nullptr;
static long current_character = -1;
static long selected_character = -1;
static char * sfx_filename = nullptr;
static constexpr size_t TEXT_BUFFER_SIZE = 2048;
static char text_buffer[TEXT_BUFFER_SIZE];

/****************************
 Keyboard callback
 ****************************/
static void cb_quit(void * arg)
{
	text_buffer[0] = '\0';

	if (sfx_filename != nullptr)
	{
		sfx_stop(MUSIC_CHANNEL);
		g_IsMusicPlaying = false;
	}

	screen_set_screen(Screen::SELECT);
}

/**********************************
 **********************************/
static void cb_show_item(void * arg)
{
	item_t * item = (item_t*) arg;

	if (item == nullptr)
	{
		return;
	}

	Camera * l_Camera = screen_get_camera();
	l_Camera->setX(item->rect.x + item->rect.w / 2);
	l_Camera->setY(item->rect.y + item->rect.h / 2);

	selected_character = current_character;
}

/**********************************
 **********************************/
static void cb_over(void * arg, int x, int y)
{
	current_character = (long) arg;
}

/**********************************
 **********************************/
static void cb_next_character(void * arg)
{
	if (current_character == -1)
	{
		current_character = 0;
	}

	current_character++;
	if (current_character >= character_num)
	{
		current_character = character_num - 1;
	}

	cb_show_item(character_list[current_character].item);
}

/**********************************
 **********************************/
static void cb_previous_character(void * arg)
{
	if (current_character == -1)
	{
		current_character = 0;
	}

	current_character--;
	if (current_character <= 0)
	{
		current_character = 0;
	}

	cb_show_item(character_list[current_character].item);
}

/**********************************
 **********************************/
static void cb_wheel_up(Uint32 y, Uint32 unused)
{
	cb_previous_character(nullptr);
}

/**********************************
 **********************************/
static void cb_wheel_down(Uint32 y, Uint32 unused)
{
	cb_next_character(nullptr);
}

/**************************************
 **************************************/
static void cb_keyboard_text(void * arg)
{
	const char * text = (const char*) arg;

	if (text[0] == '\0')
	{
		werr(LOGUSER, "Cannot create character with no name");
		return;
	}

	if (selected_character == -1)
	{
		werr(LOGUSER, "Character not selected");
		return;
	}

	network_request_character_creation(*(context_get_player()->getConnection()), character_list[selected_character].id, text);

	if (sfx_filename != nullptr)
	{
		sfx_stop(MUSIC_CHANNEL);
		g_IsMusicPlaying = false;
	}

	screen_set_screen(Screen::SELECT);

	text_buffer[0] = '\0';
}

/**********************************
 **********************************/
void scr_create_frame_start(Context * context)
{
}

/**********************************
 **********************************/
void scr_create_init()
{
	current_character = -1;

	item_list_free(g_itemList);
	g_itemList = nullptr;

	if (character_list != nullptr)
	{
		free(character_list);
		character_list = nullptr;
	}

	character_num = 0;

	network_request_playable_character_list(*(context_get_player()->getConnection()));
}

/*************************
 *************************/
int sort_character(const void * p_pArg1, const void * p_pArg2)
{
	const character_t * l_pChar1 = static_cast<const character_t*>(p_pArg1);
	const character_t * l_pChar2 = static_cast<const character_t*>(p_pArg2);

	if (l_pChar1->type == nullptr || l_pChar2->type == nullptr)
	{
		return 0;
	}

	int l_Compare = strcmp(l_pChar1->type, l_pChar2->type);

	if (l_Compare == 0 && l_pChar1->name != nullptr && l_pChar2->name != nullptr)
	{
		l_Compare = strcmp(l_pChar1->name, l_pChar2->name);
	}

	return l_Compare;
}

/**********************************
 Compose the character create screen
 **********************************/
item_t * scr_create_compose(Context * context)
{
	long i = 0;
	int x = 0;
	static int max_h = 0;
	item_t * item;
	item_t * item_image;
	int w;
	int h;
	static TTF_Font * font_name = nullptr;
	static TTF_Font * font_type = nullptr;

	if (character_num == 0)
	{
		return nullptr;
	}

	if (sfx_filename == nullptr)
	{
		entry_read_string(nullptr, CLIENT_CONF_FILE, &sfx_filename, CLIENT_KEY_CREATE_CHARACTER_SFX, nullptr);
	}

	if (sfx_filename != nullptr)
	{
		if (g_IsMusicPlaying == false)
		{
			if (sfx_play(*(context->getConnection()), std::string(sfx_filename), MUSIC_CHANNEL, LOOP) != -1)
			{
				g_IsMusicPlaying = true;
			}

			int sfx_volume = 100; // 100%
			entry_read_int(nullptr, CLIENT_CONF_FILE, &sfx_volume, CLIENT_KEY_CREATE_CHARACTER_SFX_VOLUME, nullptr);
			sfx_set_volume(MUSIC_CHANNEL, sfx_volume);
		}
	}

	if (g_itemList != nullptr)
	{
		item_list_free(g_itemList);
		g_itemList = nullptr;
	}

	font_name = font_get(context, FONT, FONT_SIZE);
	font_type = font_get(context, FONT, FONT_SIZE);

	sdl_free_mousecb();
	sdl_add_mousecb(MOUSE_WHEEL_UP, cb_wheel_up);
	sdl_add_mousecb(MOUSE_WHEEL_DOWN, cb_wheel_down);

	// Name box
	static TTF_Font * font = nullptr;
	font = font_get(context, FONT, FONT_SIZE);
	if (font == nullptr)
	{
		werr(LOGDESIGNER, "Can't open TTF font %s", FONT);
	}

	int sw = 0;
	int sh = 0;

	sdl_get_output_size(&sw, &sh);

	item = item_list_add(&g_itemList);

	item_set_overlay(item, 1);
	item_set_buffer(item, text_buffer, TEXT_BUFFER_SIZE);
	item_set_string_bg(item, BACKGROUND_COLOR);
	item_set_font(item, font);
	item_set_editable(item, 1);
	item_set_edit_cb(item, cb_keyboard_text);
	sdl_get_string_size(item->font, "111111111122222222223333333333", &w, &h);
	item_set_anim_shape(item, sw / 2 - w / 2, sh - FONT_SIZE - BORDER, w, h);

	SDL_LockMutex(character_create_mutex);

	// Load all anim compute max height and width of anim + string
	for (i = 0; i < character_num; i++)
	{
		// Compute the marquee file name
		char * marquee_name = nullptr;
		if (entry_read_string(CHARACTER_TEMPLATE_TABLE, character_list[i].id, &marquee_name, CHARACTER_KEY_MARQUEE, nullptr) == true)
		{
			const char * name_array[2] =
			{ nullptr, nullptr };
			name_array[0] = marquee_name;
			character_list[i].anim = imageDB_get_anim_array(context, name_array);
			free(marquee_name);
		}
		else
		{
			char ** marquee_list = nullptr;
			if (entry_read_list(CHARACTER_TEMPLATE_TABLE, character_list[i].id, &marquee_list, CHARACTER_KEY_MARQUEE, nullptr) == false)
			{
				wlog(LOGDESIGNER, "%s has no marquee", character_list[i].id);
				continue;
			}

			character_list[i].anim = imageDB_get_anim_array(context, (const char **) marquee_list);
			deep_free(marquee_list);
		}

		entry_read_string(CHARACTER_TEMPLATE_TABLE, character_list[i].id, &character_list[i].name, CHARACTER_KEY_NAME, nullptr);
		entry_read_string(CHARACTER_TEMPLATE_TABLE, character_list[i].id, &character_list[i].type, CHARACTER_KEY_TYPE, nullptr);

		if (character_list[i].anim[0] == nullptr)
		{
			continue;
		}

		if (character_list[i].anim[0]->h > max_h)
		{
			max_h = character_list[i].anim[0]->h;
		}

		if (font_name != nullptr && character_list[i].name != nullptr)
		{
			sdl_get_string_size(font_name, character_list[i].name, &w, &h);
			character_list[i].width = w;
		}
		if (font_type != nullptr && character_list[i].type != nullptr)
		{
			sdl_get_string_size(font_type, character_list[i].type, &w, &h);
			if (w > character_list[i].width)
			{
				character_list[i].width = w;
			}
		}
		if (character_list[i].anim[0]->w > character_list[i].width)
		{
			character_list[i].width = character_list[i].anim[0]->w;
		}
	}

	qsort(character_list, character_num, sizeof(character_t), sort_character);

	// Create item list
	for (i = 0; i < character_num; i++)
	{
		if (character_list[i].anim == nullptr)
		{
			continue;
		}

		// Character picture
		item = item_list_add(&g_itemList);
		item_image = item;
		character_list[i].item = item;

		item_set_pos(item, x + character_list[i].width / 2 - character_list[i].anim[0]->w / 2, max_h / 2 - character_list[i].anim[0]->h / 2);
		item_set_anim_array(item, character_list[i].anim);
		item_set_click_left(item, cb_show_item, (void *) item, nullptr);
		//item_set_click_right(item,cb_select,(void *)context,nullptr);
		//item_set_double_click_left(item,cb_select,(void *)context,nullptr);
		item_set_over(item, cb_over, (void *) i, nullptr);

		x += character_list[i].width + BORDER;
		// character name
		if (font_name == nullptr)
		{
			werr(LOGDESIGNER, "Can't open TTF font %s", FONT);
		}
		else
		{
			if (character_list[i].name != nullptr)
			{
				item = item_list_add(&g_itemList);
				item_set_string(item, character_list[i].name);
				item_set_font(item, font_name);
				// display string just above the picture
				sdl_get_string_size(item->font, item->string, &w, &h);
				item_set_anim_shape(item, item_image->rect.x + item_image->rect.w / 2 - w / 2, item_image->rect.y - h + (item_image->rect.h / 2) - (max_h / 2),
						w, h);
			}
		}

		// character type
		if (font_type == nullptr)
		{
			werr(LOGDESIGNER, "Can't open TTF font %s", FONT);
		}
		else
		{
			item = item_list_add(&g_itemList);
			item_set_string(item, character_list[i].type);
			item_set_font(item, font_type);
			// display string just below the picture
			sdl_get_string_size(item->font, item->string, &w, &h);
			item_set_anim_shape(item, item_image->rect.x + item_image->rect.w / 2 - w / 2, item_image->rect.y + (item_image->rect.h / 2) + (max_h / 2), w, h);
		}
	}

	SDL_UnlockMutex(character_create_mutex);

	if (current_character == -1)
	{
		cb_show_item(character_list[0].item);
	}

	sdl_free_keycb();
	sdl_add_keycb(SDL_SCANCODE_ESCAPE, cb_quit, nullptr, nullptr);
	sdl_add_keycb(SDL_SCANCODE_RIGHT, cb_next_character, nullptr, nullptr);
	sdl_add_keycb(SDL_SCANCODE_LEFT, cb_previous_character, nullptr, nullptr);
	//sdl_add_keycb(SDL_SCANCODE_RETURN,cb_select,nullptr,(void *)context);

	return g_itemList;
}

/*************************
 Add a character to the list
 *************************/
void scr_create_add_playable_character(const std::vector<std::string> & id_list)
{
	SDL_LockMutex(character_create_mutex);

	for (auto l_CharacterName : id_list)
	{
		character_num++;

		character_list = (character_t*) realloc(character_list, sizeof(character_t) * character_num);

		int l_Index = character_num - 1;
		character_list[l_Index].id = strdup(l_CharacterName.c_str());
		character_list[l_Index].name = nullptr;
		character_list[l_Index].type = nullptr;
		character_list[l_Index].anim = nullptr;
		character_list[l_Index].item = nullptr;
		character_list[l_Index].width = 0;

		entry_read_string(CHARACTER_TEMPLATE_TABLE, character_list[l_Index].id, &character_list[l_Index].name, CHARACTER_KEY_NAME, nullptr);
		entry_read_string(CHARACTER_TEMPLATE_TABLE, character_list[l_Index].id, &character_list[l_Index].type, CHARACTER_KEY_TYPE, nullptr);

		wlog(LOGDEVELOPER, "Character %s added", character_list[l_Index].id);
	}

	qsort(character_list, character_num, sizeof(character_t), sort_character);

	SDL_UnlockMutex(character_create_mutex);
}
