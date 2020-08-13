/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2020 carabobz@gmail.com

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

#include "Camera.h"
#include "CharacterMarquee.h"
#include "entry.h"
#include "file_client.h"
#include "file.h"
#include "font.h"
#include "imageDB.h"
#include "log.h"
#include "mutex.h"
#include "network_client.h"
#include "screen.h"
#include "SdlItemCore.h"
#include "sfx.h"
#include "syntax.h"
#include "util.h"

static const constexpr int BORDER = 20;
static const constexpr int FONT_SIZE = 30;
static const std::string FONT = "Ubuntu-C.ttf";

static std::vector<CharacterMarquee> characterMarqueeArray;
static long centeredCharacter = -1;
static std::string sfxFileName;
static bool isMusicPlaying = false;

/*****************************************************************************/
static void cb_quit()
{
	screen_quit();
}

/*****************************************************************************/
static void center_item()
{
	if (centeredCharacter == -1)
	{
		return;
	}

	int x = characterMarqueeArray[centeredCharacter].getX();
	int y = characterMarqueeArray[centeredCharacter].getY();

	if ((x == CharacterMarquee::NO_COORD) || (y == CharacterMarquee::NO_COORD))
	{
		return;
	}

	Camera * camera = screen_get_camera();
	camera->setX(x + (characterMarqueeArray[centeredCharacter].getWidth() / 2));
	camera->setY(y + (characterMarqueeArray[centeredCharacter].getHeight() / 2));

	return;
}

/*****************************************************************************/
static void cb_select(Context * ctx, long characterIndex)
{
	if (characterIndex == -1)
	{
		return;
	}

	ctx->setId(characterMarqueeArray[characterIndex].getId());
	ctx->setCharacterName(characterMarqueeArray[characterIndex].getName());
	ctx->setInGame(true);

	file_request_from_network(*(ctx->getConnection()), CHARACTER_TABLE, ctx->getId());

	sdl_free_mousecb();

	if (sfxFileName.size() != 0)
	{
		sfx_stop(MUSIC_CHANNEL);
		isMusicPlaying = false;
	}

	screen_set_screen(Screen::PLAY);
}

/*****************************************************************************/
static void cb_select_current(Context * ctx)
{
	if (centeredCharacter == -1)
	{
		return;
	}

	cb_select(ctx, centeredCharacter);
}

/*****************************************************************************/
static void cb_next_character()
{
	if (centeredCharacter == -1)
	{
		centeredCharacter = 0;
	}

	centeredCharacter++;

	if (centeredCharacter >= static_cast<int>(characterMarqueeArray.size()))
	{
		centeredCharacter = characterMarqueeArray.size() - 1;
	}

	if (characterMarqueeArray.size() != 0)
	{
		center_item();
	}
}

/*****************************************************************************/
static void cb_previous_character()
{
	centeredCharacter--;
	if (centeredCharacter < 0)
	{
		centeredCharacter = 0;
	}

	if (characterMarqueeArray.size() != 0)
	{
		center_item();
	}
}

/*****************************************************************************/
static void cb_wheel_up()
{
	cb_previous_character();
}

/*****************************************************************************/
static void cb_wheel_down()
{
	cb_next_character();
}

/*****************************************************************************/
static void cb_icon_add_clicked()
{
	if (sfxFileName.size() != 0)
	{
		sfx_stop(MUSIC_CHANNEL);
		isMusicPlaying = false;
	}

	screen_set_screen(Screen::CREATE);
}

/*****************************************************************************/
void scr_select_frame_start(Context * context)
{
}

/*****************************************************************************/
void scr_select_init()
{
	centeredCharacter = -1;
}

/*****************************************************************************/
static void init_sfx(Connection & connection)
{
	if (sfxFileName.size() == 0)
	{
		char * sfx = nullptr;
		entry_read_string(nullptr, CLIENT_CONF_FILE, &sfx, CLIENT_KEY_SELECT_CHARACTER_SFX, nullptr);
		if (sfx != nullptr)
		{
			sfxFileName = std::string(sfx);
		}
	}

	if (sfxFileName.size() != 0)
	{
		if (isMusicPlaying == false)
		{
			if (sfx_play(connection, sfxFileName, MUSIC_CHANNEL, LOOP) != -1)
			{
				isMusicPlaying = true;
			}

			int sfxVolume = 100; // 100%
			entry_read_int(nullptr, CLIENT_CONF_FILE, &sfxVolume, CLIENT_KEY_SELECT_CHARACTER_SFX_VOLUME, nullptr);
			sfx_set_volume(MUSIC_CHANNEL, sfxVolume);
		}
	}
}

/*****************************************************************************/
static void compose_add_icon(Context * context, std::vector<SdlItem *> &itemArray)
{
	char * iconAddImageName = nullptr;
	entry_read_string(nullptr, CLIENT_CONF_FILE, &iconAddImageName, CLIENT_KEY_SELECT_CHARACTER_ADD_ICON, nullptr);

	if (iconAddImageName != nullptr)
	{
		int sw = 0;
		int sh = 0;
		sdl_get_output_size(&sw, &sh);

		SdlItem * item;
		item = new SdlItem;

		SiAnim *anim = imageDB_get_anim(context, std::string(iconAddImageName));
		item->setAnim(anim);

		int x = sw / 2 - (anim->getWidth() / 2);
		int y = sh - anim->getHeight();
		item->setPos(x, y);
		item->setShape(anim->getWidth(), anim->getHeight());
		item->setOverlay(true);

		item->setClickLeftCb([]()
		{	cb_icon_add_clicked();});
		item->setClickRightCb([]()
		{	cb_icon_add_clicked();});

		itemArray.push_back(item);
	}

}

/*****************************************************************************/
void scr_select_compose(Context * context, std::vector<SdlItem *> & itemArray)
{
	static TTF_Font * fontName = nullptr;
	static TTF_Font * fontType = nullptr;

	if (fontName == nullptr)
	{
		fontName = font_get(context, FONT, FONT_SIZE);
	}
	if (fontType == nullptr)
	{
		fontType = font_get(context, FONT, FONT_SIZE);
	}

	init_sfx(*(context->getConnection()));

	sdl_free_mousecb();
	sdl_add_mousecb(MOUSE_WHEEL_UP, cb_wheel_up);
	sdl_add_mousecb(MOUSE_WHEEL_DOWN, cb_wheel_down);

	compose_add_icon(context, itemArray);

	SDL_LockMutex(characterSelectMutex);

	int maxHeight = 0;
	for (auto && characterMarquee : characterMarqueeArray)
	{
		// Compute the marquee file name
		char * marqueeName = nullptr;
		if (entry_read_string(CHARACTER_TABLE, characterMarquee.getId().c_str(), &marqueeName, CHARACTER_KEY_MARQUEE, nullptr) == true)
		{
			std::vector<std::string> nameArray;
			nameArray.push_back(std::string(marqueeName));
			free(marqueeName);

			characterMarquee.setAnim(imageDB_get_anim_array(context, nameArray));
		}
		else
		{
			char ** marqueeArray = nullptr;
			if (entry_read_list(CHARACTER_TABLE, characterMarquee.getId().c_str(), &marqueeArray, CHARACTER_KEY_MARQUEE, nullptr) == false)
			{
				LOG_DESIGN(characterMarquee.getId() + " has no marquee");
				continue;
			}

			std::vector<std::string> nameArray;
			int i = 0;
			while (marqueeArray[i] != nullptr)
			{
				nameArray.push_back(std::string(marqueeArray[i]));
			}

			deep_free(marqueeArray);

			characterMarquee.setAnim(imageDB_get_anim_array(context, nameArray));
		}

		if (characterMarquee.getAnim()[0]->getHeight() > maxHeight)
		{
			maxHeight = characterMarquee.getAnim()[0]->getHeight();
		}

		int w = 0;
		int h = 0;

		if (fontName != nullptr)
		{
			sdl_get_string_size(fontName, characterMarquee.getName(), &w, &h);
			characterMarquee.setWidth(w);
		}
		if (fontType != nullptr)
		{
			sdl_get_string_size(fontType, characterMarquee.getType(), &w, &h);
			if (w > characterMarquee.getWidth())
			{
				characterMarquee.setWidth(w);
			}
		}
		if (characterMarquee.getAnim()[0]->getWidth() > characterMarquee.getWidth())
		{
			characterMarquee.setWidth(characterMarquee.getAnim()[0]->getWidth());
		}
	}

	// Create item list
	int index = -1;
	int currentX = 0;

	for (auto && characterMarquee : characterMarqueeArray)
	{
		index++;

		if (characterMarquee.getAnim().empty() == true)
		{
			continue;
		}

		// Character picture
		SdlItem * item = new SdlItem;
		itemArray.push_back(item);

		characterMarquee.setItem(item);

		int characterMarqueeMidX = currentX + (characterMarquee.getWidth() / 2);

		int x = characterMarqueeMidX - (characterMarquee.getAnim()[0]->getWidth() / 2);
		int y = maxHeight / 2 - (characterMarquee.getAnim()[0]->getHeight() / 2);

		if (x < characterMarquee.getX())
		{
			characterMarquee.setX(x);
		}
		if (y < characterMarquee.getY())
		{
			characterMarquee.setY(y);
		}
		characterMarquee.setHeight(maxHeight);

		item->setPos(x, y);
		item->setShape(characterMarquee.getAnim()[0]->getWidth(), characterMarquee.getAnim()[0]->getHeight());
		item->setAnim(characterMarquee.getAnim());

		item->setClickRightCb([context, index]()
		{	cb_select(context, index);});
		item->setDoubleClickLeftCb([context, index]()
		{	cb_select(context, index);});

		// character name
		if (fontName != nullptr)
		{
			SdlItem * itemCharacterName;
			itemCharacterName = new SdlItem;
			itemArray.push_back(itemCharacterName);

			itemCharacterName->setText(characterMarquee.getName());
			itemCharacterName->setFont(fontName);

			// display name above the picture
			int w = 0;
			int h = 0;
			sdl_get_string_size(itemCharacterName->getFont(), itemCharacterName->getText(), &w, &h);

			x = characterMarqueeMidX - (w / 2);
			y = -h;

			if (x < characterMarquee.getX())
			{
				characterMarquee.setX(x);
			}
			if (y < characterMarquee.getY())
			{
				characterMarquee.setY(y);
			}
			characterMarquee.setHeight(characterMarquee.getHeight() + h);

			itemCharacterName->setPos(x, y);
			itemCharacterName->setShape(w, h);
		}
		else
		{
			ERR_DESIGN("Can't open TTF font " + FONT);
		}

		// character type
		if (fontType != nullptr)
		{
			SdlItem * itemCharacterType;
			itemCharacterType = new SdlItem;
			itemArray.push_back(itemCharacterType);

			itemCharacterType->setText(characterMarquee.getType());
			itemCharacterType->setFont(fontType);

			// display type below the picture
			int w = 0;
			int h = 0;
			sdl_get_string_size(itemCharacterType->getFont(), itemCharacterType->getText(), &w, &h);

			x = characterMarqueeMidX - (w / 2);
			y = maxHeight;

			if (x < characterMarquee.getX())
			{
				characterMarquee.setX(x);
			}
			if (y < characterMarquee.getY())
			{
				characterMarquee.setY(y);
			}
			characterMarquee.setHeight(characterMarquee.getHeight() + h);

			itemCharacterType->setPos(x, y);
			itemCharacterType->setShape(w, h);
		}
		else
		{
			ERR_DESIGN("Can't open TTF font " + FONT);
		}

		currentX += characterMarquee.getWidth() + BORDER;
	}

	if ((centeredCharacter == -1) && (characterMarqueeArray.size() > 0))
	{
		centeredCharacter = 0;

		center_item();
	}

	SDL_UnlockMutex(characterSelectMutex);

	sdl_clean_key_cb();
	sdl_add_down_key_cb(SDL_SCANCODE_ESCAPE, []()
	{	cb_quit();});
	sdl_add_down_key_cb(SDL_SCANCODE_RIGHT, []()
	{	cb_next_character();});
	sdl_add_down_key_cb(SDL_SCANCODE_LEFT, []()
	{	cb_previous_character();});
	sdl_add_down_key_cb(SDL_SCANCODE_RETURN, [context]()
	{	cb_select_current(context);});
}

/*****************************************************************************/
void scr_select_add_user_character(const std::string & id, const std::string & type, const std::string & name)
{
	SDL_LockMutex(characterSelectMutex);

	LOG("Add character");

	CharacterMarquee characterMarquee;

	characterMarquee.setId(id);
	characterMarquee.setType(type);
	characterMarquee.setName(name);
	characterMarquee.setWidth(0);

	characterMarqueeArray.push_back(characterMarquee);

	LOG_DESIGN("Character " + id + " / " + type + " / " + name + " added");

	SDL_UnlockMutex(characterSelectMutex);
}
