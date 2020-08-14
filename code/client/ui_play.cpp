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

#include "ChatBox.h"
#include "client_conf.h"
#include "const.h"
#include "entry.h"
#include "font.h"
#include "imageDB.h"
#include "item.h"
#include "log.h"
#include "network_client.h"
#include "protocol.h"
#include "scr_play.h"
#include "screen.h"
#include "SdlItemCore.h"
#include "syntax.h"
#include "textview.h"
#include "ui_play.h"
#include "util.h"
#include <stack>
#include <string>
#include <vector>

static const std::string FONT = "Ubuntu-C.ttf";
static constexpr int FONT_SIZE = 30;
static const std::string TEXT_FONT = "Ubuntu-C.ttf";
static constexpr int TEXT_FONT_SIZE = 15;
static const std::string ITEM_FONT = "Ubuntu-C.ttf";
static constexpr int ITEM_FONT_SIZE = 15;
static const std::string SPEAK_FONT = "Ubuntu-C.ttf";
static constexpr int SPEAK_FONT_SIZE = 32;

static const std::string POPUP_TAG_IMAGE = "image";
static const std::string POPUP_TAG_TEXT = "text";
static const std::string POPUP_TAG_ACTION = "action";
static const std::string POPUP_TAG_EOL = "eol";
static const std::string POPUP_TAG_EOP = "eop";
static const std::string POPUP_TAG_END = "popup_end";

static constexpr Uint32 BACKGROUND_COLOR = 0xFFFFFF40;

static UiType currentUi = UiType::MAIN;
static std::string lastAction;

// main UI
static std::vector<std::string> attributeText;
static std::string textBuffer;

// inventory UI
static std::vector<std::string> inventoryArray;

// popup UI
static constexpr int MOUSE_WHEEL_SCROLL = 20;
static std::stack<std::vector<std::string>> popupFifo;

struct ActionParam
{
	std::string action;
	std::string param;
};

static int popupOffset = 0;

static int firstAction = 0;
static int numAction = 0;

/*****************************************************************************/
static void draw_background(Context * ctx, std::vector<SdlItem *> & itemArray)
{
	static SiAnim * bgAnim = nullptr;
	if (bgAnim != nullptr)
	{
		delete bgAnim;
	}

	int sw = 0;
	int sh = 0;
	sdl_get_output_size(&sw, &sh);
	bgAnim = anim_create_color(sw, sh, BACKGROUND_COLOR);

	SdlItem * item = new SdlItem;
	itemArray.push_back(item);

	item->setPos(0, 0);
	item->setAnim(bgAnim);
	item->setOverlay(true);
}

/*****************************************************************************/
void ui_play_set(const UiType type)
{
	currentUi = type;
	screen_compose();
}

/*****************************************************************************/
UiType ui_play_get()
{
	return currentUi;
}

/*****************************************************************************/
const std::string & ui_play_get_last_action()
{
	return lastAction;
}

/*****************************************************************************/
static void cb_main_quit()
{
	if (ui_play_get() == UiType::MAIN)
	{
		context_get_player()->setInGame(false);
		network_request_stop(*(context_get_player()->getConnection()));
		Context * currentCtx = context_get_first();
		while (currentCtx != nullptr)
		{
			Context * nextCtx = currentCtx->m_next;
			if (currentCtx != context_get_player())
			{
				currentCtx->setInGame(false);

				LOG("Waiting for " + currentCtx->getId());
				int threadStatus = 0;
				SDL_WaitThread(currentCtx->getNpcThread(), &threadStatus);
				LOG(currentCtx->getId() + " exited");

				context_free(currentCtx);

				//TODO do this smarter
#if 0
				// Remove selection
				while (item != nullptr)
				{
					if (item->user_ptr != nullptr)
					{
						item->user_ptr = nullptr;
					}

					item = item->next;
				}
#endif
			}
			currentCtx = nextCtx;
		}

		screen_set_screen(Screen::SELECT);
	}
}

/*****************************************************************************/
static void key_up()
{
	Context * ctx = context_get_player();

	network_send_action(*(ctx->getConnection()), client_conf_get().action_move_up, nullptr);
}

/*****************************************************************************/
static void key_up_released()
{
	Context * ctx = context_get_player();

	network_send_action_stop(*(ctx->getConnection()), client_conf_get().action_move_up);
}

/*****************************************************************************/
static void key_down()
{
	Context * ctx = context_get_player();

	network_send_action(*(ctx->getConnection()), client_conf_get().action_move_down, nullptr);
}

/*****************************************************************************/
static void key_down_released()
{
	Context * ctx = context_get_player();

	network_send_action_stop(*(ctx->getConnection()), client_conf_get().action_move_down);
}

/*****************************************************************************/
static void key_left()
{
	Context * ctx = context_get_player();

	network_send_action(*(ctx->getConnection()), client_conf_get().action_move_left, nullptr);
}

/*****************************************************************************/
static void key_left_released()
{
	Context * ctx = context_get_player();

	network_send_action_stop(*(ctx->getConnection()), client_conf_get().action_move_left);
}

/*****************************************************************************/
static void key_right()
{
	Context * ctx = context_get_player();

	network_send_action(*(ctx->getConnection()), client_conf_get().action_move_right, nullptr);
}

/*****************************************************************************/
static void key_right_released()
{
	Context * ctx = context_get_player();

	network_send_action_stop(*(ctx->getConnection()), client_conf_get().action_move_right);
}

/*****************************************************************************/
static void key_up_left()
{
	Context * ctx = context_get_player();

	network_send_action(*(ctx->getConnection()), client_conf_get().action_move_up_left, nullptr);
}

/*****************************************************************************/
static void key_up_left_released()
{
	Context * ctx = context_get_player();

	network_send_action_stop(*(ctx->getConnection()), client_conf_get().action_move_up_left);
}

/*****************************************************************************/
static void key_up_right()
{
	Context * ctx = context_get_player();

	network_send_action(*(ctx->getConnection()), client_conf_get().action_move_up_right, nullptr);
}

/*****************************************************************************/
static void key_up_right_released()
{
	Context * ctx = context_get_player();

	network_send_action_stop(*(ctx->getConnection()), client_conf_get().action_move_up_right);
}

/*****************************************************************************/
static void key_down_left()
{
	Context * ctx = context_get_player();

	network_send_action(*(ctx->getConnection()), client_conf_get().action_move_down_left, nullptr);
}

/*****************************************************************************/
static void key_down_left_released()
{
	Context * ctx = context_get_player();

	network_send_action_stop(*(ctx->getConnection()), client_conf_get().action_move_down_left);
}

/*****************************************************************************/
static void key_down_right()
{
	Context * ctx = context_get_player();

	network_send_action(*(ctx->getConnection()), client_conf_get().action_move_down_right, nullptr);
}

/*****************************************************************************/
static void key_down_right_released()
{
	Context * ctx = context_get_player();

	network_send_action_stop(*(ctx->getConnection()), client_conf_get().action_move_down_right);
}

/*****************************************************************************/
static int compose_attribute(Context * ctx, std::vector<SdlItem *> & itemArray)
{
	static TTF_Font * font = nullptr;
	if (font == nullptr)
	{
		font = font_get(ctx, FONT, FONT_SIZE);
	}

	attributeText.clear();

	char ** nameArray;
	if (entry_get_group_list(CHARACTER_TABLE, ctx->getId().c_str(), &nameArray,
	ATTRIBUTE_GROUP, nullptr) == false)
	{
		return 0;
	}

	int attributeHeight = 0;
	int attributeQty = 0;
	int index = 0;
	int y = 0;

	while (nameArray[index] != nullptr)
	{
		int value;
		if (entry_read_int(CHARACTER_TABLE, ctx->getId().c_str(), &value, ATTRIBUTE_GROUP, nameArray[index], ATTRIBUTE_CURRENT, nullptr) == false)
		{
			index++;
			continue;
		}

		attributeText.push_back(std::string(nameArray[index]) + ": " + std::to_string(value));

		SdlItem * item = new SdlItem;
		itemArray.push_back(item);

		item->setOverlay(true);
		item->setText(attributeText[attributeQty - 1]);
		item->setFont(font);

		int w = 0;
		int h = 0;
		sdl_get_string_size(item->getFont(), item->getText(), &w, &h);

		item->setPos(0, y);
		item->setShape(w, h);

		y += h;
		if (attributeHeight < y)
		{
			attributeHeight = y;
		}

		index++;
	}

	deep_free(nameArray);

	return attributeHeight;
}

/*****************************************************************************/
void ui_play_cb_action(const std::string & action)
{
	network_send_action(*(context_get_player()->getConnection()), action, nullptr);

	if (action.size() > 0)
	{
		lastAction = action;
	}
	else
	{
		lastAction.clear();
	}
}

/*****************************************************************************/
static void cb_wheel_up_action()
{
	firstAction--;
	if (firstAction < 0)
	{
		firstAction = 0;
		return;
	}
	screen_compose();
}

/*****************************************************************************/
static void cb_wheel_down_action()
{
	firstAction++;
	screen_compose();
}

/*****************************************************************************/
static int compose_action(Context * ctx, std::vector<SdlItem *> & itemArray)
{
	int action_bar_height = 0;

	int sw = 0;
	int sh = 0;
	sdl_get_output_size(&sw, &sh);

	// Read action list for current user
	char ** actionArray = nullptr;
	if (entry_read_list(CHARACTER_TABLE, ctx->getId().c_str(), &actionArray,
	CHARACTER_KEY_ACTION, nullptr) == false)
	{
		return 0;
	}

	int x = 0;
	int i = 0;
	while (actionArray[i] != nullptr)
	{
		if (i < firstAction)
		{
			if (actionArray[i + 1] != nullptr)
			{
				i++;
				continue;
			}
			firstAction = i;
		}

#if 0
		if (entry_read_string(ACTION_TABLE, actionArray[i], &text,
		ACTION_KEY_TEXT, nullptr) == false)
		{
			i++;
			continue;
		}
#endif

		char * icon = nullptr;
		char ** iconArray = nullptr;

		if (entry_read_string(ACTION_TABLE, actionArray[i], &icon,
		ACTION_KEY_ICON, nullptr) == true)
		{
			iconArray = (char **) malloc(sizeof(char *) * 2);
			iconArray[0] = icon;
			iconArray[1] = nullptr;
		}
		else
		{
			if (entry_read_list(ACTION_TABLE, actionArray[i], &iconArray,
			ACTION_KEY_ICON, nullptr) == false)
			{
				i++;
				continue;
			}
		}

		SdlItem * item = new SdlItem;

		SdlItem::Layout layout = SdlItem::Layout::TOP_LEFT;
		int layoutRead = 0;
		entry_read_int(ACTION_TABLE, actionArray[i], &layoutRead,
		ACTION_KEY_ICON_LAYOUT, nullptr);
		layout = static_cast<SdlItem::Layout>(layoutRead);
		item->setLayout(layout);

		item->setOverlay(true);

		std::string action = std::string(actionArray[i]);
		item->setClickLeftCb([action]()
		{	ui_play_cb_action(action);});

		item->setWheelUpCb([]()
		{	cb_wheel_up_action();});
		item->setWheelDownCb([]()
		{	cb_wheel_down_action();});

		// load image
		std::vector<std::string> iconArrayStd;
		int k = 0;
		while (iconArray[k] != nullptr)
		{
			iconArrayStd.push_back(std::string(iconArray[k]));
			k++;
		}
		deep_free(iconArray);

		std::vector<SiAnim*> animArray = imageDB_get_anim_array(ctx, iconArrayStd);
		item->setAnim(animArray);
		item->setPos(x, sh - animArray[0]->getHeight());

		// calculate next icon start X
		x += animArray[0]->getWidth();
		if (action_bar_height < animArray[0]->getHeight())
		{
			action_bar_height = animArray[0]->getHeight();
		}

		char ** iconOver = nullptr;
		if (entry_read_list(ACTION_TABLE, actionArray[i], &iconOver,
		ACTION_KEY_ICON_OVER, nullptr) == true)
		{
			std::vector<std::string> iconOverStd;
			int i = 0;
			while (iconOver[i] != nullptr)
			{
				iconOverStd.push_back(std::string(iconOver[i]));
			}
			deep_free(iconOver);

			animArray.clear();
			animArray = imageDB_get_anim_array(ctx, iconOverStd);
			item->setAnimOver(animArray);
		}

		char ** iconClick = nullptr;
		if (entry_read_list(ACTION_TABLE, actionArray[i], &iconClick,
		ACTION_KEY_ICON_CLICK, nullptr) == true)
		{
			std::vector<std::string> iconClickStd;
			int j = 0;
			while (iconClick[j] != nullptr)
			{
				iconClickStd.push_back(std::string(iconClick[j]));
				j++;
			}

			deep_free(iconClick);

			animArray.clear();
			animArray = imageDB_get_anim_array(ctx, iconClickStd);
			item->setAnimClick(animArray);
		}

		itemArray.push_back(item);

		i++;
	}

	numAction = firstAction + i;

	return action_bar_height;
}

/*****************************************************************************/
static void cb_select_slot(const std::string & id)
{
	Context * ctx = context_get_player();

	network_send_action(*(ctx->getConnection()), client_conf_get().action_select_equipment, id.c_str(), nullptr);
}

/*****************************************************************************/
static void show_inventory()
{
	ui_play_set(UiType::INVENTORY);
}

/*****************************************************************************/
static void compose_equipment(Context * ctx, std::vector<SdlItem*> & itemArray)
{
	SiAnim * anim;
	SiAnim * anim2;
	SiAnim * anim3;

	char * mytemplate = nullptr;
#if 0
	char * name;
#endif
	char * iconName = nullptr;
	char * equippedName = nullptr;
#if 0
	char * equipped_text = nullptr;
#endif
	char * equippedIconName = nullptr;
	char * inventoryIconName = nullptr;
	SiAnim * inventoryIconAnim = nullptr;

	int sw = 0;
	int sh = 0;
	sdl_get_output_size(&sw, &sh);

	char ** slot_list = nullptr;
	entry_get_group_list(CHARACTER_TABLE, ctx->getId().c_str(), &slot_list, EQUIPMENT_GROUP, nullptr);

	int max_h = 0;
	int max_w = 0;
	int y = 0;
	int x = 0;
	int h1;
	int index = 0;
	while (slot_list && slot_list[index] != nullptr)
	{
#if 0
		// Get the slot name
		if(entry_read_string(CHARACTER_TABLE,ctx->m_id,&item_name,EQUIPMENT_GROUP,slot_list[index],EQUIPMENT_NAME,nullptr) == false )
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
		if (entry_read_string(CHARACTER_TABLE, ctx->getId().c_str(), &iconName,
		EQUIPMENT_GROUP, slot_list[index], EQUIPMENT_ICON, nullptr) == false)
		{
			continue;
		}
		else
		{
			// load image
			anim = imageDB_get_anim(ctx, iconName);
			free(iconName);

			SdlItem * item = new SdlItem;
			itemArray.push_back(item);

			x = sw - anim->getWidth();
			h1 = anim->getHeight();
			item->setOverlay(true);
			item->setPos(x, y);
			item->setAnim(anim);

			std::string slot = std::string(slot_list[index]);
			item->setClickLeftCb([slot]()
			{	cb_select_slot(slot);});

			if (anim->getWidth() > max_w)
			{
				max_w = anim->getWidth();
			}
			if (anim->getHeight() > max_h)
			{
				max_h = anim->getHeight();
			}
		}

		// Is there an equipped object ?
		if (entry_read_string(CHARACTER_TABLE, ctx->getId().c_str(), &equippedName,
		EQUIPMENT_GROUP, slot_list[index], EQUIPMENT_EQUIPPED, nullptr) == true && equippedName[0] != 0)
		{
#if 0
			// Get the equipped object name
			if(entry_read_string(ITEM_TABLE,equippedName,&equipped_text,ITEM_NAME,nullptr) == false )
			{
				werr(LOGDESIGNER,"Can't read object %s name in equipment slot %s",equippedName,slot_list[index]);
			}
			free(equipped_text);
#endif
			// Get its icon
			mytemplate = item_is_resource(std::string(equippedName));

			if (mytemplate == nullptr)
			{
				if (entry_read_string(ITEM_TABLE, equippedName, &equippedIconName, ITEM_ICON, nullptr) == false)
				{
					werr(LOGDESIGNER, "Can't read object %s icon in equipment slot %s", equippedName, slot_list[index]);
				}
			}
			else
			{
				if (entry_read_string(ITEM_TEMPLATE_TABLE, mytemplate, &equippedIconName, ITEM_ICON, nullptr) == false)
				{
					werr(LOGDESIGNER, "Can't read item %s icon name (template: %s)", equippedName, mytemplate);
				}
				free(mytemplate);
			}

			if (equippedIconName)
			{
				SdlItem * item = new SdlItem;
				itemArray.push_back(item);

				anim2 = imageDB_get_anim(ctx, equippedIconName);
				free(equippedIconName);

				item->setOverlay(true);
				item->setPos(x - anim->getWidth(), y);
				item->setAnim(anim2);

				std::string slot = std::string(slot_list[index]);
				item->setClickLeftCb([slot]()
				{	cb_select_slot(slot);});

				if (h1 < anim->getHeight())
				{
					h1 = anim->getHeight();
				}
			}
			free(equippedName);
		}

		// Draw selection cursor
		if (client_conf_get().cursor_equipment.empty() == false)
		{
			if (ctx->getSelectionEquipment() == std::string(slot_list[index]))
			{
				anim3 = imageDB_get_anim(ctx, client_conf_get().cursor_equipment);

				SdlItem * item = new SdlItem;
				itemArray.push_back(item);

				// Center on icon
				item->setOverlay(true);
				item->setPos(x - (anim3->getWidth() - anim->getWidth()) / 2, y - (anim3->getHeight() - anim->getWidth()) / 2);
				item->setAnim(anim3);
			}
		}

		if (h1 > anim->getHeight())
		{
			y += h1;
		}
		else
		{
			y += anim->getHeight();
		}

		index++;
	}
	deep_free(slot_list);

	// Draw selected item
	const std::string & inventory = ctx->getSelectionInventory();

	if (inventory != "")
	{
		mytemplate = item_is_resource(inventory);

		if (mytemplate == nullptr)
		{
			if (entry_read_string(ITEM_TABLE, inventory.c_str(), &inventoryIconName, ITEM_ICON, nullptr) == false)
			{
				werr(LOGDESIGNER, "Can't read item %s icon name", inventory.c_str());
			}
		}
		else
		{
			if (entry_read_string(ITEM_TEMPLATE_TABLE, mytemplate, &inventoryIconName, ITEM_ICON, nullptr) == false)
			{
				werr(LOGDESIGNER, "Can't read item %s icon name (template: %s)", inventory.c_str(), mytemplate);
			}
			free(mytemplate);
		}

		if (inventoryIconName)
		{
			SdlItem * item = new SdlItem;
			itemArray.push_back(item);

			anim = imageDB_get_anim(ctx, inventoryIconName);
			free(inventoryIconName);

			item->setOverlay(true);
			item->setPos(sw - anim->getWidth(), y);
			item->setAnim(anim);

			item->setClickLeftCb([]()
			{	show_inventory();});
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

		if (inventoryIconAnim == nullptr)
		{
			inventoryIconAnim = anim_create_color(max_w, max_h, 0x7f7f7f7f);
		}

		SdlItem * item = new SdlItem;
		itemArray.push_back(item);

		item->setOverlay(true);
		item->setPos(sw - inventoryIconAnim->getWidth(), y);
		item->setAnim(inventoryIconAnim);

		item->setClickLeftCb([]()
		{	show_inventory();});
	}
}

/*****************************************************************************/
static void keyboard_text(std::string text)
{
	network_send_action(*(context_get_player()->getConnection()), WOG_CHAT, text, nullptr);
	textBuffer.clear();
	screen_compose();
}

/****************************
 ****************************/
static void cb_print_coord()
{
	Context * ctx = context_get_player();

	int mapWidth = 0;
	entry_read_int(MAP_TABLE, ctx->getMap().c_str(), &mapWidth, MAP_KEY_WIDTH, nullptr);

//TODO: take layer into account
#if 0
	entry_read_list_index(MAP_TABLE,ctx->m_map,&type,scr_play_get_current_x()+scr_play_get_current_y()*mapWidth,layer_name,MAP_KEY_TYPE,nullptr);
	sprintf(buf,"x=%d y=%d type=%s",scr_play_get_current_x(),scr_play_get_current_y(),type);
	free(type);
#endif
	std::string text = "x= " + std::to_string(scr_play_get_current_x()) + " y=" + std::to_string(scr_play_get_current_y());
	textview_add_line(text);

	screen_compose();
}

/*****************************************************************************/
static void main_compose(Context * ctx, std::vector<SdlItem*> & itemArray)
{
	static TTF_Font * font = font_get(ctx, TEXT_FONT, TEXT_FONT_SIZE);
	static ChatBox chatBox(font, keyboard_text);

	int attribute_height = compose_attribute(ctx, itemArray);
	int action_bar_height = compose_action(ctx, itemArray);
	chatBox.compose(itemArray, attribute_height, action_bar_height);

	compose_equipment(ctx, itemArray);

	sdl_add_down_key_cb(SDL_SCANCODE_I, show_inventory);
	sdl_add_down_key_cb(SDL_SCANCODE_UP, key_up);
	sdl_add_up_key_cb(SDL_SCANCODE_UP, key_up_released);
	sdl_add_down_key_cb(SDL_SCANCODE_KP_8, key_up);
	sdl_add_up_key_cb(SDL_SCANCODE_KP_8, key_up_released);
	sdl_add_down_key_cb(SDL_SCANCODE_DOWN, key_down);
	sdl_add_up_key_cb(SDL_SCANCODE_DOWN, key_down_released);
	sdl_add_down_key_cb(SDL_SCANCODE_KP_2, key_down);
	sdl_add_up_key_cb(SDL_SCANCODE_KP_2, key_down_released);
	sdl_add_down_key_cb(SDL_SCANCODE_LEFT, key_left);
	sdl_add_up_key_cb(SDL_SCANCODE_LEFT, key_left_released);
	sdl_add_down_key_cb(SDL_SCANCODE_KP_4, key_left);
	sdl_add_up_key_cb(SDL_SCANCODE_KP_4, key_left_released);
	sdl_add_down_key_cb(SDL_SCANCODE_RIGHT, key_right);
	sdl_add_up_key_cb(SDL_SCANCODE_RIGHT, key_right_released);
	sdl_add_down_key_cb(SDL_SCANCODE_KP_6, key_right);
	sdl_add_up_key_cb(SDL_SCANCODE_KP_6, key_right_released);
	sdl_add_down_key_cb(SDL_SCANCODE_KP_7, key_up_left);
	sdl_add_up_key_cb(SDL_SCANCODE_KP_7, key_up_left_released);
	sdl_add_down_key_cb(SDL_SCANCODE_KP_9, key_up_right);
	sdl_add_up_key_cb(SDL_SCANCODE_KP_9, key_up_right_released);
	sdl_add_down_key_cb(SDL_SCANCODE_KP_1, key_down_left);
	sdl_add_up_key_cb(SDL_SCANCODE_KP_1, key_down_left_released);
	sdl_add_down_key_cb(SDL_SCANCODE_KP_3, key_down_right);
	sdl_add_up_key_cb(SDL_SCANCODE_KP_3, key_down_right_released);
	sdl_add_down_key_cb(SDL_SCANCODE_ESCAPE, cb_main_quit);
	sdl_add_down_key_cb(SDL_SCANCODE_SCROLLLOCK, cb_print_coord);
}

/*****************************************************************************/
static void cb_inventory_quit()
{
	ui_play_set(UiType::MAIN);
}

/*****************************************************************************/
void cb_inventory_select(const std::string & itemId)
{
	Context * ctx = context_get_player();

	network_send_action(*(ctx->getConnection()), client_conf_get().action_select_inventory, itemId.c_str(), nullptr);
}

/*****************************************************************************/
static void compose_inventory(Context * ctx, std::vector<SdlItem*> & itemArray)
{
	std::string description;
	static TTF_Font * font = nullptr;

	if (font != nullptr)
	{
		font = font_get(ctx, ITEM_FONT, ITEM_FONT_SIZE);
	}

	inventoryArray.clear();

	draw_background(ctx, itemArray);

	// read data from file
	char ** inventoryList = nullptr;
	if (entry_read_list(CHARACTER_TABLE, ctx->getId().c_str(), &inventoryList,
	CHARACTER_KEY_INVENTORY, nullptr) == false)
	{
		return;
	}

	int i = 0;
	while (inventoryList[i] != nullptr)
	{
		inventoryArray.push_back(std::string(inventoryList[i]));
		i++;
	}

	std::string label;
	int x = 0;

	for (auto && inventoryId : inventoryArray)
	{
		char * mytemplate = item_is_resource(std::string(inventoryArray[i]));

		SiAnim * anim = nullptr;
		char * value = nullptr;
		if (mytemplate == nullptr)
		{
			// Icon is mandatory for now
			if (entry_read_string(ITEM_TABLE, inventoryId.c_str(), &value,
			ITEM_ICON, nullptr) == false)
			{
				continue;
			}
			// load image
			anim = imageDB_get_anim(ctx, value);
			free(value);

			if (entry_read_string(ITEM_TABLE, inventoryId.c_str(), &value,
			ITEM_NAME, nullptr) == false)
			{
				label = inventoryId;
			}
			else
			{
				label = std::string(value);
			}

			if (entry_read_string(ITEM_TABLE, inventoryId.c_str(), &value,
			ITEM_DESC, nullptr) == false)
			{
				description = "";
			}
			else
			{
				description = std::string(value);
			}
		}
		else
		{
			// Icon is mandatory for now
			if (entry_read_string(ITEM_TEMPLATE_TABLE, mytemplate, &value,
			ITEM_ICON, nullptr) == false)
			{
				free(mytemplate);
				continue;
			}
			// load image
			anim = imageDB_get_anim(ctx, value);
			free(value);

			if (entry_read_string(ITEM_TEMPLATE_TABLE, mytemplate, &value,
			ITEM_NAME, nullptr) == false)
			{
				label = inventoryId;
			}
			else
			{
				label = std::string(value);
			}

			if (entry_read_string(ITEM_TEMPLATE_TABLE, mytemplate, &value,
			ITEM_DESC, nullptr) == false)
			{
				description = "";
			}
			else
			{
				description = value;
			}
			free(mytemplate);

		}

		int quantity = resource_get_quantity(inventoryId);

		if (quantity > 0)
		{
			int w = 0;
			int h = 0;

			SdlItem * item = new SdlItem;
			itemArray.push_back(item);

			item->setPos(x, 0);
			item->setAnim(anim);
			if (quantity > 1)
			{
				item->setText(std::to_string(quantity));

				static constexpr Uint32 BACKGROUND_COLOR = 0xFFFFFF40;
				item->setBackGroudColor(BACKGROUND_COLOR);
				item->setFont(font);
				sdl_get_string_size(item->getFont(), item->getText(), &w, &h);
			}

			item->setOverlay(true);
			if (w > anim->getWidth())
			{
				x += w;
			}
			else
			{
				x += anim->getWidth();
			}

			std::string itemId = std::string(inventoryArray[i]);
			item->setClickLeftCb([itemId]()
			{	cb_inventory_select(itemId);});
		}
	}
}

/*****************************************************************************/
static void compose_inventory_select(Context * ctx, std::vector<SdlItem*> & itemArray)
{
	const std::string & inventorySelected = ctx->getSelectionInventory();

	if (inventorySelected == "")
	{
		return;
	}

	if (client_conf_get().cursor_inventory.empty() == true)
	{
		return;
	}

	SiAnim * animCursor = imageDB_get_anim(ctx, client_conf_get().cursor_inventory);

	inventoryArray.clear();

	// read data from file
	char ** inventoryList = nullptr;
	if (entry_read_list(CHARACTER_TABLE, ctx->getId().c_str(), &inventoryList,
	CHARACTER_KEY_INVENTORY, nullptr) == false)
	{
		return;
	}

	int i = 0;
	while (inventoryList[i] != nullptr)
	{
		inventoryArray.push_back(std::string(inventoryList[i]));
		i++;
	}

	int x = 0;
	char * mytemplate = nullptr;
	bool isItemFound = false;
	SiAnim * iconAnim = nullptr;

	for (auto && inventory : inventoryArray)
	{
		mytemplate = item_is_resource(inventory);

		char * icon_name = nullptr;

		if (mytemplate == nullptr)
		{
			if (entry_read_string(ITEM_TABLE, inventory.c_str(), &icon_name,
			ITEM_ICON, nullptr) == false)
			{
				ERR_DESIGN("Can't read item " + inventory + " icon name");
			}
		}
		else
		{
			if (entry_read_string(ITEM_TEMPLATE_TABLE, mytemplate, &icon_name,
			ITEM_ICON, nullptr) == false)
			{
				ERR_DESIGN("Can't read item " + inventory + " icon name (template: " + std::string(mytemplate));
			}
			free(mytemplate);
		}

		iconAnim = imageDB_get_anim(ctx, icon_name);
		free(icon_name);

		x += iconAnim->getWidth();

		if (inventory == inventorySelected)
		{
			isItemFound = true;
			break;
		}

	}

	if (isItemFound == true)
	{
		SdlItem * item = new SdlItem;
		itemArray.push_back(item);

		item->setPos(x, 0);
		item->setAnim(animCursor);
		item->setOverlay(true);
	}
}

/*****************************************************************************/
static void inventory_compose(Context * ctx, std::vector<SdlItem*> & itemArray)
{
	compose_inventory(ctx, itemArray);
	compose_inventory_select(ctx, itemArray);

	sdl_add_down_key_cb(SDL_SCANCODE_I, cb_inventory_quit);
	sdl_add_down_key_cb(SDL_SCANCODE_ESCAPE, cb_inventory_quit);
	sdl_add_down_key_cb(SDL_SCANCODE_SPACE, cb_inventory_quit);
}

/*****************************************************************************/
static void cb_popup_quit()
{
	popupOffset = 0;

	popupFifo.pop();

	if (popupFifo.size() == 0)
	{
		ui_play_set(UiType::MAIN);
	}
}

/*****************************************************************************/
void cb_popup(const ActionParam & actionParam)
{
	Context * player = context_get_player();

	cb_popup_quit();

	network_send_action(*(player->getConnection()), actionParam.action.c_str(), actionParam.param.c_str(), nullptr);

	screen_compose();
}

/*****************************************************************************/
static void cb_wheel_up()
{
	popupOffset -= MOUSE_WHEEL_SCROLL;
	if (popupOffset < 0)
	{
		popupOffset = 0;
	}
	screen_compose();
}

/*****************************************************************************/
static void cb_wheel_down()
{
	popupOffset += MOUSE_WHEEL_SCROLL;
	screen_compose();
}

/*****************************************************************************/
static void compose_popup(Context * ctx, std::vector<SdlItem*> & itemArray)
{
	if (popupFifo.size() == 0)
	{
		return;
	}

	draw_background(ctx, itemArray);

	static TTF_Font *font = font_get(ctx, SPEAK_FONT, SPEAK_FONT_SIZE);

	sdl_free_mousecb();
	sdl_add_mousecb(MOUSE_WHEEL_UP, cb_wheel_up);
	sdl_add_mousecb(MOUSE_WHEEL_DOWN, cb_wheel_down);

	std::vector<std::string> popupData = popupFifo.top();

	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
	int maxHeight = 0;
	ActionParam * actionParams = nullptr;

	for (auto iter = popupData.cbegin(); iter != popupData.cend(); ++iter)
	{
		if (*iter == POPUP_TAG_IMAGE)
		{
			++iter;
			SiAnim * anim = imageDB_get_anim(ctx, *iter);

			SdlItem * item = new SdlItem;
			itemArray.push_back(item);

			item->setPos(x, y - popupOffset);
			item->setAnim(anim);
			item->setOverlay(true);

			if (actionParams != nullptr)
			{
				ActionParam params = *actionParams;
				item->setClickLeftCb([params]()
				{	cb_popup(params);});
				delete actionParams;
				actionParams = nullptr;

				actionParams = nullptr;
			}
			x += anim->getWidth();
			if (maxHeight < anim->getHeight())
			{
				maxHeight = anim->getHeight();
			}
			continue;
		}
		if (*iter == POPUP_TAG_TEXT)
		{
			++iter;

			SdlItem * item = new SdlItem;
			itemArray.push_back(item);

			item->setText(*iter);
			item->setFont(font);
			sdl_get_string_size(item->getFont(), item->getText(), &width, &height);
			item->setPos(x, y - popupOffset);
			item->setShape(width, height);
			item->setOverlay(true);

			if (actionParams != nullptr)
			{
				ActionParam params = *actionParams;
				item->setClickLeftCb([params]()
				{	cb_popup(params);});
				delete actionParams;
				actionParams = nullptr;
			}
			x += width;
			if (maxHeight < height)
			{
				maxHeight = height;
			}
			continue;
		}
		if (*iter == POPUP_TAG_ACTION)
		{
			actionParams = new ActionParam;
			// get action
			++iter;
			actionParams->action = *iter;
			// get param
			++iter;
			actionParams->param = *iter;
			continue;
		}
		if (*iter == POPUP_TAG_EOL)
		{
			y += maxHeight;
			maxHeight = 0;
			x = 0;
		}
		if (*iter == POPUP_TAG_EOP)
		{
			y += maxHeight;
			maxHeight = 0;
			x = 0;
		}
	}
}

/*****************************************************************************/
void ui_play_popup_add(const std::vector<std::string> & data)
{
	popupFifo.push(data);

	ui_play_set(UiType::POPUP);
}

/*****************************************************************************/
static void popup_compose(Context * ctx, std::vector<SdlItem*> & itemArray)
{
	compose_popup(ctx, itemArray);

	sdl_add_down_key_cb(SDL_SCANCODE_ESCAPE, cb_popup_quit);
	sdl_add_down_key_cb(SDL_SCANCODE_SPACE, cb_popup_quit);
}

/*****************************************************************************/
void ui_play_compose(Context * ctx, std::vector<SdlItem*> & itemArray)
{
	switch (currentUi)
	{
	case UiType::MAIN:
		main_compose(ctx, itemArray);
		break;
	case UiType::INVENTORY:
		inventory_compose(ctx, itemArray);
		break;
	case UiType::POPUP:
		popup_compose(ctx, itemArray);
		break;
	default:
		ERR_USER("Wrong UI type");
		break;
	}
}
/******************************************************************************
 Called once
 *****************************************************************************/
void ui_play_init()
{
	textBuffer.clear();
}
