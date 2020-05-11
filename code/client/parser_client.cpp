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

#include "Connection.h"

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
 Return false on error
 **************************************/
static int manage_login_ok(Connection & connection, const pb::LoginOk & login_ok)
{
	if (network_open_data_connection(connection) == false)
	{
		return false;
	}
	connection.setConnected(true);
	LOG_USER("Successfully connected");
	network_request_user_character_list(connection);
	LOG("Character list requested");

	return true;
}

/**************************************
 Return false on error
 **************************************/
static int manage_login_nok(Connection & connection, const pb::LoginNok & login_nok)
{
	connection.setConnected(false);

	werr(LOGUSER, "Check your login and password (they are case sensitive)\n");
	exit(-1);

	return true;
}

/**************************************
 Return false on error
 **************************************/
static int manage_playable_character(const pb::PlayableCharacter & playable_character)
{
	std::vector<std::string> idList;
	for (int i = 0; i < playable_character.id().size(); i++)
	{
		idList.push_back(playable_character.id(i));
	}

	scr_create_add_playable_character(idList);
	screen_compose();

	return true;
}

/**************************************
 Return false on error
 **************************************/
static int manage_file(const pb::File& file)
{
	file_add(file.name(), file.data());
	screen_compose();

	return true;
}

/**************************************
 Return false on error
 **************************************/
static int manage_user_character(const pb::UserCharacter& user_character)
{
	scr_select_add_user_character(user_character.id(), user_character.type(), user_character.name());
	screen_compose();

	return true;
}

/**************************************
 Return false on error
 **************************************/
static int manage_context(const pb::Context& incoming_context)
{
	Context receivedContext;
	receivedContext.setCharacterName(incoming_context.character_name());
	receivedContext.setId(incoming_context.id());
	receivedContext.setInGame(incoming_context.in_game());
	receivedContext.setMap(incoming_context.map());
	receivedContext.setNpc(incoming_context.npc());
	receivedContext.setTile(incoming_context.tile_x(), incoming_context.tile_y());
	receivedContext.setType(incoming_context.type());
	receivedContext.setSelectionEquipment(incoming_context.selection().equipment());
	receivedContext.setSelectionContextId(incoming_context.selection().id());
	receivedContext.setSelectionInventory(incoming_context.selection().inventory());
	receivedContext.setSelectionTile(incoming_context.selection().map(), incoming_context.selection().map_coord_tx(),
			incoming_context.selection().map_coord_ty());

	context_add_or_update_from_network_frame(receivedContext);
	screen_compose();

	return true;
}

/**************************************
 Return false on error
 **************************************/
static int manage_text(const pb::Text& text)
{
	textview_add_line(text.text());

	return true;
}

/**************************************
 Return false on error
 **************************************/
static int manage_entry(const pb::Entry& entry)
{
	if (entry_update(entry.type(), entry.table(), entry.file(), entry.path(), entry.value()) != -1)
	{
		screen_compose();
	}

	return true;
}

/**************************************
 Return false on error
 **************************************/
static int manage_popup(const pb::PopUp& popup)
{
	std::vector<std::string> data;
	for (int i = 0; i < popup.data().size(); i++)
	{
		data.push_back(popup.data(i));
	}

	ui_play_popup_add(data);
	screen_compose();

	return true;
}

/**************************************
 Return false on error
 **************************************/
static int manage_effect(Connection & connection, const pb::Effect& effect)
{
	std::vector<std::string> params;
	for (int i = 0; i < effect.param().size(); i++)
	{
		params.push_back(effect.param(i));
	}

	EffectManager::processEffectFrame(connection, params);

	return true;
}

/***********************************
 Return false on error
 ***********************************/
int parse_incoming_data(Connection & connection, const std::string & serializedData)
{
	pb::ServerMessage message;
	if (message.ParseFromString(serializedData) == false)
	{
		werr(LOGUSER, "Parsing failed");
	}
	else
	{
		if (message.has_login_ok())
		{
			wlog(LOGDEVELOPER, "[network] Received login OK for user %s", connection.getUserName().c_str());
			manage_login_ok(connection, message.login_ok());
		}
		else if (message.has_login_nok())
		{
			wlog(LOGDEVELOPER, "[network] Received login NOK for user %s", connection.getUserName().c_str());
			manage_login_nok(connection, message.login_nok());
		}
		else if (message.has_playable_character())
		{
			wlog(LOGDEVELOPER, "[network] Received playable character for user %s", connection.getUserName().c_str());
			manage_playable_character(message.playable_character());
		}
		else if (message.has_file())
		{
			wlog(LOGDEVELOPER, "[network] Received file %s", message.file().name().c_str());
			manage_file(message.file());
		}
		else if (message.has_user_character())
		{
			wlog(LOGDEVELOPER, "[network] Received user %s character", connection.getUserName().c_str(), message.user_character().name().c_str());
			manage_user_character(message.user_character());
		}
		else if (message.has_context())
		{
			wlog(LOGDEVELOPER, "[network] Received context %s", message.context().id().c_str());
			manage_context(message.context());
		}
		else if (message.has_text())
		{
			wlog(LOGDEVELOPER, "[network] Received text");
			manage_text(message.text());
		}
		else if (message.has_entry())
		{
			wlog(LOGDEVELOPER, "[network] Received entry");
			manage_entry(message.entry());
		}
		else if (message.has_popup())
		{
			wlog(LOGDEVELOPER, "[network] Received pop-up");
			manage_popup(message.popup());
		}
		else if (message.has_effect())
		{
			wlog(LOGDEVELOPER, "[network] Received effect");
			manage_effect(connection, message.effect());
		}
		else
		{
			werr(LOGUSER, "Unknown message received");
		}
	}

	return true;
}
