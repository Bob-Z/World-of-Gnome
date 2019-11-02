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

#include "common.h"
#include "Context.h"
#include "EffectManager.h"
#include "entry.h"
#include "file_client.h"
#include "file.h"
#include "log.h"
#include "network_client.h"
#include "protocol.h"
#include "scr_create.h"
#include "scr_select.h"
#include "screen.h"
#include "Selection.h"
#include "textview.h"
#include "ui_play.h"
#include "wog.pb.h"
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

/**************************************
 Return RET_NOK on error
 **************************************/
static int manage_login_ok(Context * context, const pb::LoginOk & login_ok)
{
	wlog(LOGDEVELOPER, "[network] Received login OK for user %s", context->getUserName().c_str());

	if (network_open_data_connection(context) == RET_NOK)
	{
		return RET_NOK;
	}
	context->setConnected(true);
	wlog(LOGUSER, "Successfully connected");
	network_request_user_character_list(context);
	wlog(LOGDEVELOPER, "Character list requested");

	return RET_OK;
}

/**************************************
 Return RET_NOK on error
 **************************************/
static int manage_login_nok(Context * context, const pb::LoginNok & login_nok)
{
	wlog(LOGDEVELOPER, "[network] Received login NOK for user %s", context->getUserName().c_str());

	context->setConnected(false);

	werr(LOGUSER, "Check your login and password (they are case sensitive)\n");
	exit(-1);

	return RET_OK;
}

/**************************************
 Return RET_NOK on error
 **************************************/
static int manage_playable_character(Context * context, const pb::PlayableCharacter & playable_character)
{
	wlog(LOGDEVELOPER, "[network] Received playable character for user %s", context->getUserName().c_str());

	std::vector<std::string> id_list;
	for (int i = 0; i < playable_character.id().size(); i++)
	{
		id_list.push_back(playable_character.id(i));
	}

	scr_create_add_playable_character(context, id_list);
	screen_compose();

	return RET_OK;
}

/**************************************
 Return RET_NOK on error
 **************************************/
static int manage_file(Context * context, const pb::File& file)
{
	wlog(LOGDEVELOPER, "[network] Received file %s", file.name().c_str());

	file_add(context, file.name(), file.data());
	screen_compose();

	return RET_OK;
}

/**************************************
 Return RET_NOK on error
 **************************************/
static int manage_user_character(Context * context, const pb::UserCharacter& user_character)
{
	wlog(LOGDEVELOPER, "[network] Received user %s character", context->getUserName().c_str(), user_character.name().c_str());

	scr_select_add_user_character(context, user_character.id(), user_character.type(), user_character.name());
	screen_compose();

	return RET_OK;
}

/**************************************
 Return RET_NOK on error
 **************************************/
static int manage_context(Context * context, const pb::Context& incoming_context)
{
	wlog(LOGDEVELOPER, "[network] Received context %s", incoming_context.id().c_str());

	Context received_context;
	received_context.setCharacterName(incoming_context.character_name());
	received_context.setConnected(incoming_context.connected());
	received_context.setId(incoming_context.id());
	received_context.setInGame(incoming_context.in_game());
	received_context.setMap(incoming_context.map());
	received_context.setNpc(incoming_context.npc());
	received_context.setTileX(incoming_context.tile_x());
	received_context.setTileY(incoming_context.tile_y());
	received_context.setType(incoming_context.type());
	received_context.setUserName(incoming_context.user_name());
	received_context.setSelectionEquipment(incoming_context.selection().equipment());
	received_context.setSelectionContextId(incoming_context.selection().id());
	received_context.setSelectionInventory(incoming_context.selection().inventory());
	received_context.setSelectionTile(incoming_context.selection().map(), incoming_context.selection().map_coord_tx(),
			incoming_context.selection().map_coord_ty());

	context_add_or_update_from_network_frame(received_context);
	screen_compose();

	return RET_OK;
}

/**************************************
 Return RET_NOK on error
 **************************************/
static int manage_text(Context * context, const pb::Text& text)
{
	wlog(LOGDEVELOPER, "[network] Received text");

	textview_add_line(text.text());

	return RET_OK;
}

/**************************************
 Return RET_NOK on error
 **************************************/
static int manage_entry(Context * context, const pb::Entry& entry)
{
	wlog(LOGDEVELOPER, "[network] Received entry");

	if (entry_update(entry.type(), entry.table(), entry.file(), entry.path(), entry.value()) != -1)
	{
		screen_compose();
	}

	return RET_OK;
}

/**************************************
 Return RET_NOK on error
 **************************************/
static int manage_popup(Context * context, const pb::PopUp& popup)
{
	wlog(LOGDEVELOPER, "[network] Received pop-up");

	std::vector<std::string> data;
	for (int i = 0; i < popup.data().size(); i++)
	{
		data.push_back(popup.data(i));
	}

	ui_play_popup_add(data);
	screen_compose();

	return RET_OK;
}

/**************************************
 Return RET_NOK on error
 **************************************/
static int manage_effect(Context * context, const pb::Effect& effect)
{
	wlog(LOGDEVELOPER, "[network] Received effect");

	std::vector<std::string> params;
	for (int i = 0; i < effect.param().size(); i++)
	{
		params.push_back(effect.param(i));
	}

	EffectManager::processEffectFrame(context, params);

	return RET_OK;
}

/***********************************
 Return RET_NOK on error
 ***********************************/
int parse_incoming_data(Context * context, const std::string & serialized_data)
{
	pb::ServerMessage message;
	if (message.ParseFromString(serialized_data) == false)
	{
		werr(LOGUSER, "Parsing failed");
	}
	else
	{
		if (message.has_login_ok())
		{
			manage_login_ok(context, message.login_ok());
		}
		else if (message.has_login_nok())
		{
			manage_login_nok(context, message.login_nok());
		}
		else if (message.has_playable_character())
		{
			manage_playable_character(context, message.playable_character());
		}
		else if (message.has_file())
		{
			manage_file(context, message.file());
		}
		else if (message.has_user_character())
		{
			manage_user_character(context, message.user_character());
		}
		else if (message.has_context())
		{
			manage_context(context, message.context());
		}
		else if (message.has_text())
		{
			manage_text(context, message.text());
		}
		else if (message.has_entry())
		{
			manage_entry(context, message.entry());
		}
		else if (message.has_popup())
		{
			manage_popup(context, message.popup());
		}
		else if (message.has_effect())
		{
			manage_effect(context, message.effect());
		}
		else
		{
			werr(LOGUSER, "Unknown message received");
		}
	}

	return RET_OK;
}
