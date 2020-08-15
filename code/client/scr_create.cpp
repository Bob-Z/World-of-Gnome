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

#include "Camera.h"
#include "CharacterMarquee.h"
#include "entry.h"
#include "file.h"
#include "font.h"
#include "global.h"
#include "imageDB.h"
#include "item.h"
#include "LockGuard.h"
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
static constexpr unsigned long BACKGROUND_COLOR = 0xFFFFFF40U;
static bool isMusicPlaying = false;
static std::vector<CharacterMarquee> characterMarqueeArray;
static int currentCharacter = -1;
static int selectedCharacter = -1;
static std::string sfxFileName;
static std::string textBuffer;
Lock characterMarqueeArrayLock;

/*****************************************************************************/
static void cb_quit()
{
	textBuffer.clear();

	sfx_stop(MUSIC_CHANNEL);
	isMusicPlaying = false;

	screen_set_screen(Screen::SELECT);
}

/*****************************************************************************/
static void cb_show_item(SdlItem & item)
{
	Camera * camera = screen_get_camera();
	camera->setX(item.getRect().x + item.getRect().w / 2);
	camera->setY(item.getRect().y + item.getRect().h / 2);

	selectedCharacter = currentCharacter;
}

/*****************************************************************************/
static void cb_over(int characterIndex)
{
	currentCharacter = characterIndex;
}

/*****************************************************************************/
static void cb_next_character()
{
	if (currentCharacter != -1)
	{
		LockGuard guard(characterMarqueeArrayLock);

		currentCharacter++;
		if (currentCharacter >= static_cast<int>(characterMarqueeArray.size()))
		{
			currentCharacter = characterMarqueeArray.size() - 1;
		}

		cb_show_item(*(characterMarqueeArray[currentCharacter].getItem()));
	}
}

/*****************************************************************************/
static void cb_previous_character()
{
	if (currentCharacter != -1)
	{
		currentCharacter--;
		if (currentCharacter < 0)
		{
			currentCharacter = 0;
		}

		LockGuard guard(characterMarqueeArrayLock);
		cb_show_item(*(characterMarqueeArray[currentCharacter].getItem()));
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
static void cb_keyboard_text(const std::string & text)
{
	if (text.size() == 0)
	{
		ERR_USER("Cannot create character with no name");
		return;
	}

	if (selectedCharacter == -1)
	{
		ERR_USER("Character not selected");
		return;
	}

	LockGuard guard(characterMarqueeArrayLock);
	network_request_character_creation(*(context_get_player()->getConnection()), characterMarqueeArray[selectedCharacter].getId(), text);

	if (sfxFileName.size() != 0)
	{
		sfx_stop(MUSIC_CHANNEL);
		isMusicPlaying = false;
	}

	screen_set_screen(Screen::SELECT);

	textBuffer.clear();
}

/*****************************************************************************/
void scr_create_frame_start(Context * context)
{
}

/*****************************************************************************/
void scr_create_init()
{
	currentCharacter = -1;

	LockGuard guard(characterMarqueeArrayLock);
	characterMarqueeArray.clear();

	network_request_playable_character_list(*(context_get_player()->getConnection()));
}
/*****************************************************************************/
static void init_sfx(Connection & connection)
{
	try
	{
		if (isMusicPlaying == false)
		{
			if (sfx_play(connection, getDataManager().get<std::string>("", CLIENT_CONF_FILE,
			{ CLIENT_KEY_CREATE_CHARACTER_SFX }), MUSIC_CHANNEL, LOOP) != -1)
			{
				isMusicPlaying = true;
			}

			sfx_set_volume(MUSIC_CHANNEL, getDataManager().getNoExcept<int>("", CLIENT_CONF_FILE,
			{ CLIENT_KEY_CREATE_CHARACTER_SFX_VOLUME }, 100));
		}
	} catch (...)
	{
	}
}
/*****************************************************************************/
static void fill_character_marquee(Context * context, TTF_Font * fontName, TTF_Font * fontType, int & maxHeight)
{
	// Load all anim compute max height and width of anim + string
	for (auto && characterMarquee : characterMarqueeArray)
	{
		// Compute the marquee file name
		char * marqueeName = nullptr;
		if (entry_read_string(CHARACTER_TEMPLATE_TABLE, characterMarquee.getId().c_str(), &marqueeName, CHARACTER_KEY_MARQUEE, nullptr) == true)
		{
			std::vector<std::string> nameArray;
			nameArray.push_back(std::string(marqueeName));
			free(marqueeName);

			characterMarquee.setAnim(imageDB_get_anim_array(context, nameArray));
		}
		else
		{
			char ** marqueeList = nullptr;
			if (entry_read_list(CHARACTER_TEMPLATE_TABLE, characterMarquee.getId().c_str(), &marqueeList, CHARACTER_KEY_MARQUEE, nullptr) == false)
			{
				LOG_DESIGN(characterMarquee.getId() + " has no marquee");
				continue;
			}

			std::vector<std::string> nameArray;
			int i = 0;
			while (marqueeList[i] != nullptr)
			{
				nameArray.push_back(std::string(marqueeList[i]));
				i++;
			}
			characterMarquee.setAnim(imageDB_get_anim_array(context, nameArray));
			deep_free(marqueeList);
		}

		if (characterMarquee.getAnim().empty() == true)
		{
			LOG_DESIGN(characterMarquee.getId() + " has no marquee");
			continue;
		}

		characterMarquee.setWidth(characterMarquee.getAnim()[0]->getWidth());
		characterMarquee.setHeight(characterMarquee.getAnim()[0]->getHeight());

		char * name = nullptr;
		entry_read_string(CHARACTER_TEMPLATE_TABLE, characterMarquee.getId().c_str(), &name, CHARACTER_KEY_NAME, nullptr);
		if (name != nullptr)
		{
			characterMarquee.setName(std::string(name));
			free(name);
		}

		char * type = nullptr;
		entry_read_string(CHARACTER_TEMPLATE_TABLE, characterMarquee.getId().c_str(), &type, CHARACTER_KEY_TYPE, nullptr);
		if (type != nullptr)
		{
			characterMarquee.setType(std::string(type));
			free(type);
		}

		if ((fontName != nullptr) && (characterMarquee.getName().size() != 0))
		{
			int w = 0;
			int h = 0;
			sdl_get_string_size(fontName, characterMarquee.getName(), &w, &h);

			if (w > characterMarquee.getWidth())
			{
				characterMarquee.setWidth(w);
			}

			characterMarquee.setHeight(characterMarquee.getHeight() + h);
		}

		if ((fontType != nullptr) && (characterMarquee.getType().size() != 0))
		{
			int w = 0;
			int h = 0;
			sdl_get_string_size(fontType, characterMarquee.getType(), &w, &h);
			if (w > characterMarquee.getWidth())
			{
				characterMarquee.setWidth(w);
			}

			characterMarquee.setHeight(characterMarquee.getHeight() + h);
		}

		if (characterMarquee.getHeight() > maxHeight)
		{
			maxHeight = characterMarquee.getHeight();
		}
	}
}

/*****************************************************************************/
static void sort_characterMarqueeArray()
{
	std::sort(characterMarqueeArray.begin(), characterMarqueeArray.end(), [](const auto c1, const auto c2)
	{	if(c1.getType() == c2.getType())
		{
			return (c1.getName() < c2.getName());
		}
		else
		{
			return (c1.getType() < c2.getType());
		}});
}

/*****************************************************************************/
static void fill_itemArray(std::vector<SdlItem *> & itemArray, const int maxHeight, TTF_Font * fontName, TTF_Font * fontType)
{
	// Create item list
	int currentX = 0;
	int characterIndex = -1;

	for (auto && characterMarquee : characterMarqueeArray)
	{
		characterIndex++;

		if (characterMarquee.getAnim().size() == 0)
		{
			continue;
		}

		// Character picture
		SdlItem * itemAnim = new SdlItem;
		itemArray.push_back(itemAnim);

		characterMarquee.setItem(itemAnim);

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

		itemAnim->setPos(x, y);
		itemAnim->setAnim(characterMarquee.getAnim());

		itemAnim->setClickLeftCb([itemAnim]()
		{	cb_show_item(*itemAnim);});

		itemAnim->setOverCb([characterIndex](int x, int y)
		{	cb_over(characterIndex);});

		currentX += characterMarquee.getWidth() + BORDER;
		// character name
		if (fontName == nullptr)
		{
			ERR_DESIGN("Can't open TTF font " + FONT);
		}
		else
		{
			if (characterMarquee.getName().size() != 0)
			{
				SdlItem * itemName = new SdlItem;
				itemArray.push_back(itemName);

				itemName->setText(characterMarquee.getName());
				itemName->setFont(fontName);
				// display string just above the picture

				int w = 0;
				int h = 0;
				sdl_get_string_size(itemName->getFont(), itemName->getText(), &w, &h);

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

				itemName->setPos(x, y);
				itemName->setShape(w, h);
			}
		}

		// character type
		if (fontType == nullptr)
		{
			ERR_DESIGN("Can't open TTF font " + FONT);
		}
		else
		{
			SdlItem * itemType = new SdlItem;
			itemArray.push_back(itemType);

			itemType->setText(characterMarquee.getType());
			itemType->setFont(fontType);

			// display string just below the picture
			int w = 0;
			int h = 0;
			sdl_get_string_size(itemType->getFont(), itemType->getText(), &w, &h);

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

			itemType->setPos(x, y);
			itemType->setShape(w, h);
		}
	}
}

/*****************************************************************************/
void scr_create_compose(Context * context, std::vector<SdlItem *> & itemArray)
{
	static TTF_Font * fontName = nullptr;
	static TTF_Font * fontType = nullptr;

	if (characterMarqueeArray.size() == 0)
	{
		return;
	}

	init_sfx(*(context->getConnection()));

	if (fontName == nullptr)
	{
		fontName = font_get(context, FONT, FONT_SIZE);
	}

	if (fontType == nullptr)
	{
		fontType = font_get(context, FONT, FONT_SIZE);
	}

	sdl_free_mousecb();
	sdl_add_mousecb(MOUSE_WHEEL_UP, cb_wheel_up);
	sdl_add_mousecb(MOUSE_WHEEL_DOWN, cb_wheel_down);

	// Name box
	static TTF_Font * font = nullptr;
	font = font_get(context, FONT, FONT_SIZE);
	if (font == nullptr)
	{
		ERR_DESIGN("Can't open TTF font " + FONT);
	}

	int sw = 0;
	int sh = 0;
	sdl_get_output_size(&sw, &sh);

	SdlItem * itemNameBox = new SdlItem;
	itemArray.push_back(itemNameBox);

	itemNameBox->setOverlay(true);
	itemNameBox->setBackGroudColor(BACKGROUND_COLOR);
	itemNameBox->setFont(font);
	itemNameBox->setEditable(true);
	itemNameBox->setEditCb([](std::string text)
	{	cb_keyboard_text(text);});

	int w = 0;
	int h = 0;
	sdl_get_string_size(font, "111111111122222222223333333333", &w, &h);
	itemNameBox->setPos(sw / 2 - w / 2, sh - FONT_SIZE);
	itemNameBox->setShape(w, h);

	LockGuard guard(characterMarqueeArrayLock);

	int maxHeight = 0;

	fill_character_marquee(context, fontName, fontType, maxHeight);

	sort_characterMarqueeArray();

	fill_itemArray(itemArray, maxHeight, fontName, fontType);

	if ((currentCharacter != -1) && (characterMarqueeArray[0].getItem() != nullptr))
	{
		cb_show_item(*(characterMarqueeArray[0].getItem()));
	}

	sdl_clean_key_cb();
	sdl_add_down_key_cb(SDL_SCANCODE_ESCAPE, []()
	{	cb_quit();});
	sdl_add_down_key_cb(SDL_SCANCODE_RIGHT, []()
	{	cb_next_character();});
	sdl_add_down_key_cb(SDL_SCANCODE_LEFT, []()
	{	cb_previous_character();});
	//sdl_add_keycb(SDL_SCANCODE_RETURN,cb_select,nullptr,(void *)context);
}

/*****************************************************************************/
void scr_create_add_playable_character(const std::vector<std::string> & idList)
{
	LockGuard guard(characterMarqueeArrayLock);

	for (auto && id : idList)
	{
		CharacterMarquee characterMarquee;

		characterMarquee.setId(id);

		char * name = nullptr;
		entry_read_string(CHARACTER_TEMPLATE_TABLE, id.c_str(), &name, CHARACTER_KEY_NAME, nullptr);
		if (name != nullptr)
		{
			characterMarquee.setName(std::string(name));
			free(name);
		}

		char * type = nullptr;
		entry_read_string(CHARACTER_TEMPLATE_TABLE, id.c_str(), &type, CHARACTER_KEY_TYPE, nullptr);
		if (type != nullptr)
		{
			characterMarquee.setType(std::string(type));
			free(type);
		}

		characterMarqueeArray.push_back(characterMarquee);
		LOG("Character " + id + " added");
	}

	if (characterMarqueeArray.size() > 0)
	{
		currentCharacter = 0;
	}
}
