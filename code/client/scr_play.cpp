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
#include "client_conf.h"
#include "const.h"
#include "entry.h"
#include "file.h"
#include "font.h"
#include "imageDB.h"
#include "item.h"
#include "log.h"
#include "map.h"
#include "network_client.h"
#include "network.h"
#include "screen.h"
#include "SdlItemCore.h"
#include "sfx.h"
#include "syntax.h"
#include "textview.h"
#include "ui_play.h"
#include "util.h"
#include <limits.h>

static constexpr int SFX_VOLUME_MAX = 100;

static const std::string ITEM_FONT = "Ubuntu-C.ttf";
#define ITEM_FONT_SIZE (15)

#define NORTH (1<<0)
#define SOUTH (1<<1)
#define EAST (1<<2)
#define WEST (1<<3)

enum class Align
{
	CENTER, LOWER
};

#define MAX_LAYER	(100)

static bool isMapChanged = false;
static int currentMapX = -1;
static int currentMapY = -1;
static layer_t * defaultLayer = nullptr;
static char * sfx = nullptr;
static bool isMusicPlaying = false;

/*****************************************************************************/
int scr_play_get_current_x()
{
	return currentMapX;
}

/*****************************************************************************/
int scr_play_get_current_y()
{
	return currentMapY;
}

/*****************************************************************************/
static void cb_select_sprite(const std::string & id)
{
	network_send_action(*(context_get_player()->getConnection()), client_conf_get().action_select_character, id.c_str(), nullptr);
}

/*****************************************************************************/
static void cb_redo_sprite(const std::string & id)
{
	cb_select_sprite(id);

	const std::string & lastAction = ui_play_get_last_action();
	if (lastAction.size() > 0)
	{
		ui_play_cb_action(lastAction);
	}
}

/*****************************************************************************/
static void cb_zoom()
{
	Camera * camera = screen_get_camera();

	camera->setZoom(camera->getZoom() + 1);
}

/*****************************************************************************/
static void cb_unzoom()
{
	Camera * camera = screen_get_camera();

	camera->setZoom(camera->getZoom() - 1);
}

/******************************************************************************
 Select sprite image to display
 Return nullptr if no sprite can be found
 Returned anim_t ** must be FREED
 *****************************************************************************/
static std::vector<SiAnim *> select_sprite(Context * ctx)
{
	char ** spriteList = nullptr;
	Context * playerContext = context_get_player();

	// Try to find a sprite depending on the orientation
	char * spriteName = nullptr;
	if (ctx->getOrientation() & NORTH)
	{
		entry_read_string(CHARACTER_TABLE, ctx->getId().c_str(), &spriteName, CHARACTER_KEY_DIR_N_SPRITE, nullptr);
	}
	if ((ctx->getOrientation() & SOUTH) && spriteName == nullptr)
	{
		entry_read_string(CHARACTER_TABLE, ctx->getId().c_str(), &spriteName, CHARACTER_KEY_DIR_S_SPRITE, nullptr);
	}
	if ((ctx->getOrientation() & EAST) && spriteName == nullptr)
	{
		entry_read_string(CHARACTER_TABLE, ctx->getId().c_str(), &spriteName, CHARACTER_KEY_DIR_E_SPRITE, nullptr);
	}
	if ((ctx->getOrientation() & WEST) && spriteName == nullptr)
	{
		entry_read_string(CHARACTER_TABLE, ctx->getId().c_str(), &spriteName, CHARACTER_KEY_DIR_W_SPRITE, nullptr);
	}

	if (spriteName != nullptr)
	{
		if (spriteName[0] != 0)
		{
			std::vector<std::string> spriteNameArray;
			spriteNameArray.push_back(std::string(spriteName));
			free(spriteName);
			return imageDB_get_anim_array(playerContext, spriteNameArray);
		}
		free(spriteName);
	}

	// Try sprite lists
	if (ctx->getOrientation() & NORTH)
	{
		entry_read_list(CHARACTER_TABLE, ctx->getId().c_str(), &spriteList, CHARACTER_KEY_DIR_N_SPRITE, nullptr);
	}
	if ((ctx->getOrientation() & SOUTH) && spriteList == nullptr)
	{
		entry_read_list(CHARACTER_TABLE, ctx->getId().c_str(), &spriteList, CHARACTER_KEY_DIR_S_SPRITE, nullptr);
	}
	if ((ctx->getOrientation() & EAST) && spriteList == nullptr)
	{
		entry_read_list(CHARACTER_TABLE, ctx->getId().c_str(), &spriteList, CHARACTER_KEY_DIR_E_SPRITE, nullptr);
	}
	if ((ctx->getOrientation() & WEST) && spriteList == nullptr)
	{
		entry_read_list(CHARACTER_TABLE, ctx->getId().c_str(), &spriteList, CHARACTER_KEY_DIR_W_SPRITE, nullptr);
	}

	if (spriteList != nullptr)
	{
		std::vector<std::string> spriteNameArray;
		int i = 0;
		while (spriteList[i] != nullptr)
		{
			spriteNameArray.push_back(std::string(spriteList[i]));
			i++;
		}
		deep_free(spriteList);

		return imageDB_get_anim_array(playerContext, spriteNameArray);
	}

	// try default sprite file
	if (entry_read_string(CHARACTER_TABLE, ctx->getId().c_str(), &spriteName,
	CHARACTER_KEY_SPRITE, nullptr) == true)
	{
		if (spriteName != nullptr)
		{
			if (spriteName[0] != 0)
			{
				std::vector<std::string> spriteNameArray;
				spriteNameArray.push_back(std::string(spriteName));
				free(spriteName);
				return imageDB_get_anim_array(playerContext, spriteNameArray);
			}
			free(spriteName);
		}
	}

	// try default sprite list
	if (entry_read_list(CHARACTER_TABLE, ctx->getId().c_str(), &spriteList,
	CHARACTER_KEY_SPRITE, nullptr) == true)
	{
		std::vector<std::string> spriteNameArray;
		int i = 0;
		while (spriteList[i] != nullptr)
		{
			spriteNameArray.push_back(std::string(spriteList[i]));
			i++;
		}
		deep_free(spriteList);

		return imageDB_get_anim_array(playerContext, spriteNameArray);
	}

	ERR_DESIGN("Can't read sprite name for \"" + ctx->getId() + "\"");

	std::vector<SiAnim *> empty;
	return empty;
}

/******************************************************************************
 Draw a single sprite
 if image_file_name is not nullptr, this file is used as an image rather than the normal sprite image
 *****************************************************************************/
static void set_up_sprite(Context * ctx, std::vector<SdlItem *> & itemArray)
{
	if (ctx->getMap().size() == 0)
	{
		return;
	}
	if (ctx->isInGame() == false)
	{
		return;
	}
	if (ctx->getMap() != context_get_player()->getMap())
	{
		return;
	}

	Uint32 currentTime = sdl_get_global_time();

	// Force position when the player has changed map
	bool forcePosition = false;
	if (isMapChanged == true)
	{
		ctx->setAnimationTick(currentTime);
		forcePosition = true;
	}
	// Force position when this context has changed map
	if (ctx->isMapChanged() == true)
	{
		ctx->setAnimationTick(currentTime);
		ctx->setMapChanged(false);
		forcePosition = true;
	}

	if (ctx->getAnimationTick() == 0)
	{
		ctx->setAnimationTick(currentTime);
	}

	// Detect sprite movement, initiate animation
	if ((ctx->isPositionChanged() == true) && (forcePosition == false))
	{
		ctx->setPositionChanged(false);

		/* flip need to remember previous direction to avoid resetting a
		 east -> west flip when a sprite goes to north for instance.
		 On the contrary rotation must not remember previous state, or
		 the rotation will be wrong.
		 Hence the distinction between orientation (no memory) and
		 direction (memory). */
		ctx->setOrientation(0);
		// Compute direction
		if (ctx->getTileX() > ctx->getPreviousTileX())
		{
			ctx->setDirection(ctx->getDirection() & ~WEST);
			ctx->setDirection(ctx->getDirection() | EAST);

			ctx->setOrientation(ctx->getOrientation() | EAST);
		}
		if (ctx->getTileX() < ctx->getPreviousTileX())
		{
			ctx->setDirection(ctx->getDirection() & ~EAST);
			ctx->setDirection(ctx->getDirection() | WEST);

			ctx->setOrientation(ctx->getOrientation() | WEST);
		}
		if (ctx->getTileY() > ctx->getPreviousTileY())
		{
			ctx->setDirection(ctx->getDirection() & ~NORTH);
			ctx->setDirection(ctx->getDirection() | SOUTH);

			ctx->setOrientation(ctx->getOrientation() | SOUTH);
		}
		if (ctx->getTileY() < ctx->getPreviousTileY())
		{
			ctx->setDirection(ctx->getDirection() & ~SOUTH);
			ctx->setDirection(ctx->getDirection() | NORTH);

			ctx->setOrientation(ctx->getOrientation() | NORTH);
		}
	}

	// Select sprite to display
	std::vector<SiAnim *> animArray;

	animArray = select_sprite(ctx);
	if (animArray.size() == 0)
	{
		return;
	}

	// Get position in pixel
	int px = map_t2p_x(ctx->getTileX(), ctx->getTileY(), defaultLayer);
	int py = map_t2p_y(ctx->getTileX(), ctx->getTileY(), defaultLayer);

	// Get per sprite zoom
	char * zoomStr = nullptr;
	double zoom = 1.0;
	if (entry_read_string(CHARACTER_TABLE, ctx->getId().c_str(), &zoomStr,
	CHARACTER_KEY_ZOOM, nullptr) == true)
	{
		zoom = atof(zoomStr);
		free(zoomStr);
	}

	// Align sprite on tile
	int align = 0;
	entry_read_int(CHARACTER_TABLE, ctx->getId().c_str(), &align, CHARACTER_KEY_ALIGN, nullptr);
	Align spriteAlign = static_cast<Align>(align);
	if (spriteAlign == Align::CENTER)
	{
		px -= ((animArray[0]->getWidth() * defaultLayer->map_zoom * zoom) - defaultLayer->tile_width) / 2;
		py -= ((animArray[0]->getHeight() * defaultLayer->map_zoom * zoom) - defaultLayer->tile_height) / 2;
	}
	if (spriteAlign == Align::LOWER)
	{
		px -= ((animArray[0]->getWidth() * defaultLayer->map_zoom * zoom) - defaultLayer->tile_width) / 2;
		py -= (animArray[0]->getHeight() * defaultLayer->map_zoom * zoom) - defaultLayer->tile_height;
	}

	// Add Y offset
	int spriteOffsetY = 0;
	entry_read_int(CHARACTER_TABLE, ctx->getId().c_str(), &spriteOffsetY,
	CHARACTER_KEY_OFFSET_Y, nullptr);
	py += spriteOffsetY;

	// Set sprite to item
	SdlItem * item = new SdlItem;
	itemArray.push_back(item);

	item->setPos(px, py);
	item->setAnimStartTick(ctx->getAnimationTick());
	item->setAnim(animArray);

	// Get rotation configuration
	int angle = 0;
	if ((ctx->getOrientation() & NORTH) && (ctx->getOrientation() & EAST))
	{
		entry_read_int(CHARACTER_TABLE, ctx->getId().c_str(), &angle,
		CHARACTER_KEY_DIR_NE_ROT, nullptr);
		item->setAngle(static_cast<double>(angle));
	}
	else if ((ctx->getOrientation() & SOUTH) && (ctx->getOrientation() & EAST))
	{
		entry_read_int(CHARACTER_TABLE, ctx->getId().c_str(), &angle,
		CHARACTER_KEY_DIR_SE_ROT, nullptr);
		item->setAngle(static_cast<double>(angle));
	}
	else if ((ctx->getOrientation() & SOUTH) && (ctx->getOrientation() & WEST))
	{
		entry_read_int(CHARACTER_TABLE, ctx->getId().c_str(), &angle,
		CHARACTER_KEY_DIR_SW_ROT, nullptr);
		item->setAngle(static_cast<double>(angle));
	}
	else if ((ctx->getOrientation() & NORTH) && (ctx->getOrientation() & WEST))
	{
		entry_read_int(CHARACTER_TABLE, ctx->getId().c_str(), &angle,
		CHARACTER_KEY_DIR_NW_ROT, nullptr);
		item->setAngle(static_cast<double>(angle));
	}
	else if (ctx->getOrientation() & NORTH)
	{
		entry_read_int(CHARACTER_TABLE, ctx->getId().c_str(), &angle,
		CHARACTER_KEY_DIR_N_ROT, nullptr);
		item->setAngle(static_cast<double>(angle));
	}
	else if (ctx->getOrientation() & SOUTH)
	{
		entry_read_int(CHARACTER_TABLE, ctx->getId().c_str(), &angle,
		CHARACTER_KEY_DIR_S_ROT, nullptr);
		item->setAngle(static_cast<double>(angle));
	}
	else if (ctx->getOrientation() & WEST)
	{
		entry_read_int(CHARACTER_TABLE, ctx->getId().c_str(), &angle,
		CHARACTER_KEY_DIR_W_ROT, nullptr);
		item->setAngle(static_cast<double>(angle));
	}
	else if (ctx->getOrientation() & EAST)
	{
		entry_read_int(CHARACTER_TABLE, ctx->getId().c_str(), &angle,
		CHARACTER_KEY_DIR_E_ROT, nullptr);
		item->setAngle(static_cast<double>(angle));
	}

	// Get flip configuration
	int forceFlip = 0;
	entry_read_int(CHARACTER_TABLE, ctx->getId().c_str(), &forceFlip,
	CHARACTER_KEY_FORCE_FLIP, nullptr);

	int moveStatus = ctx->getDirection();
	if (forceFlip == true)
	{
		moveStatus = ctx->getOrientation();
	}

	int flip = 0;
	if (angle == 0)
	{
		if (moveStatus & NORTH)
		{
			entry_read_int(CHARACTER_TABLE, ctx->getId().c_str(), &flip,
			CHARACTER_KEY_DIR_N_FLIP, nullptr);
		}
		if (moveStatus & SOUTH)
		{
			entry_read_int(CHARACTER_TABLE, ctx->getId().c_str(), &flip,
			CHARACTER_KEY_DIR_S_FLIP, nullptr);
		}
		if (moveStatus & WEST)
		{
			entry_read_int(CHARACTER_TABLE, ctx->getId().c_str(), &flip,
			CHARACTER_KEY_DIR_W_FLIP, nullptr);
		}
		if (moveStatus & EAST)
		{
			entry_read_int(CHARACTER_TABLE, ctx->getId().c_str(), &flip,
			CHARACTER_KEY_DIR_E_FLIP, nullptr);
		}

		switch (flip)
		{
		case 1:
			item->setFlip(SDL_FLIP_HORIZONTAL);
			break;
		case 2:
			item->setFlip(SDL_FLIP_VERTICAL);
			break;
		case 3:
			item->setFlip(static_cast<SDL_RendererFlip>(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL));
			break;
		default:
			item->setFlip(SDL_FLIP_NONE);
		}
	}

	item->setClickLeftCb([ctx]()
	{	cb_select_sprite(ctx->getId());});
	item->setClickRightCb([ctx]()
	{	cb_redo_sprite(ctx->getId());});

	item->setZoomX(zoom * defaultLayer->map_zoom);
	item->setZoomY(zoom * defaultLayer->map_zoom);

	item->setUserPtr(ctx);
}

/*****************************************************************************/
static void compose_sprite(int layerIndex, std::vector<SdlItem *> & itemArray)
{
	Context * ctx = context_get_player();

	context_lock_list();

	while (ctx != nullptr)
	{
		int layer = 0;
		entry_read_int(CHARACTER_TABLE, ctx->getId().c_str(), &layer, CHARACTER_LAYER, nullptr);

		if (layer == layerIndex)
		{
			set_up_sprite(ctx, itemArray);
		}
		ctx = ctx->m_next;
	}

	context_unlock_list();
}

/*****************************************************************************/
static void compose_item(int layerIndex, std::vector<SdlItem *> & itemArray)
{
	static TTF_Font * font = nullptr;

	Context * ctx = context_get_player();

	if (font == nullptr)
	{
		font = font_get(ctx, ITEM_FONT, ITEM_FONT_SIZE);
	}

	std::string layerName;
	layerName = std::string(MAP_KEY_LAYER) + std::to_string(layerIndex);

	char ** itemId;
	if (entry_get_group_list(MAP_TABLE, ctx->getMap().c_str(), &itemId, layerName.c_str(),
	MAP_ENTRY_ITEM_LIST, nullptr) == false)
	{
		return;
	}

	int i = 0;
	int spriteOffsetY = 0;
	Align spriteAlign = Align::CENTER;

	while (itemId[i] != nullptr)
	{
		spriteAlign = Align::CENTER;

		int x = 0;
		if (entry_read_int(MAP_TABLE, ctx->getMap().c_str(), &x, layerName.c_str(),
		MAP_ENTRY_ITEM_LIST, itemId[i], MAP_ITEM_TILE_X, nullptr) == false)
		{
			i++;
			continue;
		}

		int y = 0;
		if (entry_read_int(MAP_TABLE, ctx->getMap().c_str(), &y, layerName.c_str(),
		MAP_ENTRY_ITEM_LIST, itemId[i], MAP_ITEM_TILE_Y, nullptr) == false)
		{
			i++;
			continue;
		}

		char * mytemplate = item_is_resource(std::string(itemId[i]));

		char * spriteName = nullptr;
		int align = 0;
		if (mytemplate == nullptr)
		{
			if (entry_read_string(ITEM_TABLE, itemId[i], &spriteName,
			ITEM_SPRITE, nullptr) == false)
			{
				i++;
				continue;
			}

			entry_read_int(ITEM_TABLE, itemId[i], &align, ITEM_ALIGN, nullptr);
			spriteAlign = static_cast<Align>(align);
			entry_read_int(ITEM_TABLE, itemId[i], &spriteOffsetY,
			ITEM_OFFSET_Y, nullptr);
		}
		else
		{
			if (entry_read_string(ITEM_TEMPLATE_TABLE, mytemplate, &spriteName,
			ITEM_SPRITE, nullptr) == false)
			{
				free(mytemplate);
				i++;
				continue;
			}
			entry_read_int(ITEM_TEMPLATE_TABLE, mytemplate, &align,
			ITEM_ALIGN, nullptr);
			spriteAlign = static_cast<Align>(align);
			entry_read_int(ITEM_TEMPLATE_TABLE, mytemplate, &spriteOffsetY,
			ITEM_OFFSET_Y, nullptr);
			free(mytemplate);
		}

		SdlItem * item = new SdlItem;
		itemArray.push_back(item);

		SiAnim * anim = imageDB_get_anim(ctx, std::string(spriteName));
		free(spriteName);

		int tempX = map_t2p_x(x, y, defaultLayer);
		int tempY = map_t2p_y(x, y, defaultLayer);
		x = tempX;
		y = tempY;
		// Align on tile
		if (spriteAlign == Align::CENTER)
		{
			x -= ((anim->getWidth() * defaultLayer->map_zoom) - defaultLayer->tile_width) / 2;
			y -= ((anim->getHeight() * defaultLayer->map_zoom) - defaultLayer->tile_height) / 2;
		}
		if (spriteAlign == Align::LOWER)
		{
			x -= ((anim->getWidth() * defaultLayer->map_zoom) - defaultLayer->tile_width) / 2;
			y -= (anim->getHeight() * defaultLayer->map_zoom) - defaultLayer->tile_height;
		}

		y += spriteOffsetY;

		item->setPos(x, y);
		item->setAnim(anim);
		item->setZoomX(defaultLayer->map_zoom);
		item->setZoomY(defaultLayer->map_zoom);

		if (font != nullptr)
		{
			int quantity = 0;
			quantity = resource_get_quantity(std::string(itemId[i]));
			item->setText(std::to_string(quantity));
			item->setFont(font);
		}

		i++;
	}

	deep_free(itemId);
}

/*****************************************************************************/
static void cb_select_map(SdlItem * item)
{
	Context * ctx = context_get_player();

	std::string x = std::to_string(item->getUser1());
	std::string y = std::to_string(item->getUser2());

	network_send_action(*(ctx->getConnection()), client_conf_get().action_select_tile, ctx->getMap().c_str(), x.c_str(), y.c_str(), nullptr);
}

/*****************************************************************************/
static void cb_redo_map(SdlItem *item)
{
	cb_select_map(item);

	const std::string & lastAction = ui_play_get_last_action();
	if (lastAction.size() > 0)
	{
		ui_play_cb_action(lastAction);
	}
}

/*****************************************************************************/
static void cb_over(SdlItem * item)
{
	currentMapX = item->getUser1();
	currentMapY = item->getUser2();
}

/*****************************************************************************/
static void compose_map_button(std::vector<SdlItem *> & itemArray)
{
	Context * ctx = context_get_player();

	SiAnim * anim = nullptr;
	if (client_conf_get().cursor_over_tile)
	{
		anim = imageDB_get_anim(ctx, client_conf_get().cursor_over_tile);
	}

	int x = 0;
	int y = 0;
	for (y = 0; y < defaultLayer->map_h; y++)
	{
		for (x = 0; x < defaultLayer->map_w; x++)
		{
			SdlItem * item = new SdlItem;
			itemArray.push_back(item);

			item->setPos(map_t2p_x(x, y, defaultLayer), map_t2p_y(x, y, defaultLayer));
			item->setShape(defaultLayer->tile_width, defaultLayer->tile_height);
			item->setUser1(x);
			item->setUser2(y);
			item->setClickLeftCb([item]()
			{	cb_select_map(item);});
			item->setClickRightCb([item]()
			{	cb_redo_map(item);});
			item->setOverCb([item](int x, int y)
			{	cb_over(item);});
			item->setAnimOver(anim);
		}
	}
}

/*****************************************************************************/
static void compose_map_set(int layerIndex, std::vector<SdlItem *> & itemArray)
{
	Context * ctx = context_get_player();

	std::string layerName = std::string(MAP_KEY_LAYER) + std::to_string(layerIndex);
	char ** tileSet = nullptr;
	if (entry_read_list(MAP_TABLE, ctx->getMap().c_str(), &tileSet, layerName.c_str(), MAP_KEY_SET, nullptr) == false)
	{
		return;
	}

	layer_t * layer = map_layer_new(ctx->getMap(), layerIndex, defaultLayer);
	if (layer == nullptr)
	{
		return;
	}

	int i = 0;
	int x = 0;
	int y = 0;
	while (tileSet[i] != nullptr)
	{
		// Skip empty tile
		if (tileSet[i][0] != 0)
		{
			SdlItem * item = new SdlItem;
			itemArray.push_back(item);

			item->setPos(map_t2p_x(x, y, layer), map_t2p_y(x, y, layer));

			item->setAnim(imageDB_get_anim(ctx, tileSet[i]));
		}

		x++;
		if (x >= layer->map_w)
		{
			x = 0;
			y++;
		}
		i++;
	}

	deep_free(tileSet);

	map_layer_delete(layer);
}

/*****************************************************************************/
static void compose_map_scenery(int layerIndex, std::vector<SdlItem *> & itemArray)
{
	Context * ctx = context_get_player();

	std::string layerName = std::string(MAP_KEY_LAYER) + std::to_string(layerIndex);
	char ** sceneryList = nullptr;
	if (entry_get_group_list(MAP_TABLE, ctx->getMap().c_str(), &sceneryList, layerName.c_str(),
	MAP_KEY_SCENERY, nullptr) == false)
	{
		return;
	}

	int i = 0;
	int x = 0;
	int y = 0;
	while (sceneryList[i] != nullptr)
	{
		if (entry_read_int(MAP_TABLE, ctx->getMap().c_str(), &x, layerName.c_str(), MAP_KEY_SCENERY, sceneryList[i], MAP_KEY_SCENERY_X, nullptr) == false)
		{
			i++;
			continue;
		}
		if (entry_read_int(MAP_TABLE, ctx->getMap().c_str(), &y, layerName.c_str(), MAP_KEY_SCENERY, sceneryList[i], MAP_KEY_SCENERY_Y, nullptr) == false)
		{
			i++;
			continue;
		}

		char * imageName = nullptr;
		if (entry_read_string(MAP_TABLE, ctx->getMap().c_str(), &imageName, layerName.c_str(),
		MAP_KEY_SCENERY, sceneryList[i], MAP_KEY_SCENERY_IMAGE, nullptr) == false)
		{
			i++;
			continue;
		}

		SdlItem * item = new SdlItem;
		itemArray.push_back(item);

		item->setPos(x, y);
		item->setAnim(imageDB_get_anim(ctx, std::string(imageName)));

		free(imageName);

		i++;
	}

	deep_free(sceneryList);
}

/*****************************************************************************/
static void compose_type(int layerIndex, std::vector<SdlItem *> & itemArray)
{
	static TTF_Font * font = nullptr;

	if (client_conf_get().show_tile_type == false)
	{
		return;
	}

	Context * ctx = context_get_player();

	std::string layerName = std::string(MAP_KEY_LAYER) + std::to_string(layerIndex);
	if (entry_exist(MAP_TABLE, ctx->getMap().c_str(), layerName.c_str(), MAP_KEY_TYPE, nullptr) == false)
	{
		return;
	}

	if (font == nullptr)
	{
		font_get(ctx, ITEM_FONT, ITEM_FONT_SIZE);

	}

	int x = 0;
	int y = 0;
	for (x = 0; x < defaultLayer->map_w; x++)
	{
		for (y = 0; y < defaultLayer->map_h; y++)
		{
			char * type = nullptr;
			if (entry_read_list_index(MAP_TABLE, ctx->getMap().c_str(), &type, x + y * defaultLayer->map_w, layerName.c_str(), MAP_KEY_TYPE, nullptr) == false)
			{
				continue;
			}

			if (type[0] == 0)
			{
				continue;
			}

			SdlItem * item = new SdlItem;
			itemArray.push_back(item);
			item->setText(std::string(type));
			item->setFont(font);

			int w = 0;
			int h = 0;
			sdl_get_string_size(item->getFont(), item->getText(), &w, &h);
			item->setPos(map_t2p_x(x, y, defaultLayer), map_t2p_y(x, y, defaultLayer));
			item->setShape(w, h);
		}
	}
}

/******************************************************************************
 Compose select cursor
 *****************************************************************************/
static void compose_select(std::vector<SdlItem *> & itemArray)
{
	Context * ctx = context_get_player();

	// Tile selection
	if (client_conf_get().cursor_tile)
	{
		if (ctx->getSelectionMap() == ctx->getMap())
		{
			int pos_tx = ctx->getSelectionMapTx();
			int pos_ty = ctx->getSelectionMapTy();

			if (pos_tx != -1 && pos_ty != -1)
			{
				SdlItem * item = new SdlItem;
				itemArray.push_back(item);

				item->setAnim(imageDB_get_anim(ctx, client_conf_get().cursor_tile));

				// get pixel coordinate from tile coordinate
				int x = map_t2p_x(pos_tx, pos_ty, defaultLayer);
				int y = map_t2p_y(pos_tx, pos_ty, defaultLayer);
				item->setPos(x, y);
			}
		}
	}

	// Sprite selection
	if (client_conf_get().cursor_character_draw_script)
	{
		Context * selected_context = nullptr;
		selected_context = context_find(ctx->getSelectionContextId());
		if (selected_context == nullptr)
		{
			return;
		}

		SdlItem * item = new SdlItem;
		itemArray.push_back(item);

		item->setUserPtr(selected_context);
		item->setUser1Ptr(client_conf_get().cursor_character_draw_script);
	}
}

/*****************************************************************************/
void scr_play_frame_start(Context * context)
{
}

/*****************************************************************************/
void scr_play_init()
{
	// Register this character to receive server notifications
	Context * context = context_get_player();
	network_request_start(*(context->getConnection()), context->getId());
	ui_play_init();
}

/*****************************************************************************/
void scr_play_compose(Context * context, std::vector<SdlItem *> & itemArray)
{
	if (context->getMap() == "")
	{
		if (context->update_from_file() == false)
		{
			return;
		}
	}

	sdl_clean_key_cb();
	sdl_free_mousecb();
	sdl_add_mousecb(MOUSE_WHEEL_UP, cb_zoom);
	sdl_add_mousecb(MOUSE_WHEEL_DOWN, cb_unzoom);

	isMapChanged = context->isMapChanged();

	if (isMapChanged == true)
	{
		const std::string map_file_path = std::string(MAP_TABLE) + "/" + context->getMap();
		network_send_req_file(*(context->getConnection()), map_file_path);

		if (defaultLayer != nullptr)
		{
			map_layer_delete(defaultLayer);
		}
		defaultLayer = map_layer_new(context->getMap(), DEFAULT_LAYER, nullptr);
	}

	if ((defaultLayer != nullptr) && (defaultLayer->active != 0))
	{ // Make sure map data are available
		for (int layerIndex = 0; layerIndex < MAX_LAYER; layerIndex++)
		{
			compose_map_set(layerIndex, itemArray);
			compose_map_scenery(layerIndex, itemArray);
			compose_item(layerIndex, itemArray);
			compose_sprite(layerIndex, itemArray);
			compose_type(layerIndex, itemArray);
		}

		compose_map_button(itemArray);
		compose_select(itemArray);
		ui_play_compose(context, itemArray);
	}

	int bg_red = 0;
	int bg_blue = 0;
	int bg_green = 0;
	entry_read_int(MAP_TABLE, context->getMap().c_str(), &bg_red, MAP_KEY_BG_RED, nullptr);
	entry_read_int(MAP_TABLE, context->getMap().c_str(), &bg_blue, MAP_KEY_BG_BLUE, nullptr);
	entry_read_int(MAP_TABLE, context->getMap().c_str(), &bg_green, MAP_KEY_BG_GREEN, nullptr);
	sdl_set_background_color(bg_red, bg_blue, bg_green, 255);

	char * old_sfx = nullptr;
	old_sfx = sfx;
	sfx = nullptr;

	entry_read_string(MAP_TABLE, context->getMap().c_str(), &sfx, MAP_SFX, nullptr);

	if (old_sfx != nullptr)
	{
		if (sfx != nullptr)
		{
			if (strcmp(old_sfx, sfx) != 0)
			{
				sfx_stop(MUSIC_CHANNEL);
				isMusicPlaying = false;
			}
		}
		else // sfx == nullptr
		{
			sfx_stop(MUSIC_CHANNEL);
			isMusicPlaying = false;
		}
		free(old_sfx);
	}

	if ((sfx != nullptr) && (sfx[0] != '\0'))
	{
		if (isMusicPlaying == false)
		{
			if (sfx_play(*(context->getConnection()), std::string(sfx), MUSIC_CHANNEL, LOOP) != -1)
			{
				isMusicPlaying = true;
			}
			int sfx_volume = SFX_VOLUME_MAX;
			entry_read_int(MAP_TABLE, context->getMap().c_str(), &sfx_volume, MAP_SFX_VOLUME, nullptr);
			sfx_set_volume(MUSIC_CHANNEL, sfx_volume);
		}
	}
}
