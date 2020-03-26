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

#include "action.h"
#include "character.h"
#include "context_server.h"
#include "network_server.h"
#include <client_server.h>
#include "Context.h"
#include <cstdint>
#include <cstring>
#include <entry.h>
#include <file.h>
#include <log.h>
#include <network.h>
#include <protocol.h>
#include <stdlib.h>
#include <string>
#include <syntax.h>
#include <util.h>
#include <utility>
#include <vector>
#include <wog.pb.h>

/**************************************
 Return false on error
 **************************************/
static int manage_login(Connection & connection, const pb::Login & login)
{
	char * password = nullptr;
	if (entry_read_string(PASSWD_TABLE, login.user().c_str(), &password,
	PASSWD_KEY_PASSWORD, nullptr) == false)
	{
		return false;
	}

	if (strcmp(password, login.password().c_str()) != 0)
	{
		free(password);
		werr(LOGUSER, "[network] Wrong login for %s", login.user().c_str());
		network_send_login_nok(connection);
		// force client disconnection
		return false;
	}
	else
	{
		free(password);
		connection.setUserName(login.user());

		connection.setConnected(true);

		network_send_login_ok(connection);

		wlog(LOGUSER, "[network] Login successful for user %s", connection.getUserName().c_str());

		return true;
	}
}

/**************************************
 Return false on error
 **************************************/
static int manage_start(Connection & connection, const pb::Start & start)
{
	connection.setContextId(start.id());

	Context * context = context_new();
	context->setId(start.id());
	context->setInGame(true);
	context->setConnection(&connection);

	context->update_from_file();
	context_spread(context);
	context_request_other_context(context);

	wlog(LOGDEVELOPER, "[network] received start request for ID %s and user %s", start.id().c_str(), connection.getUserName().c_str());

	return true;
}

/**************************************
 Return false on error
 **************************************/
static int manage_stop(Connection & connection, const pb::Stop & stop)
{
	Context * context = context_find(connection.getContextId());

	if (context->isInGame() == true)
	{
		context->setInGame(false);
		context->setMap("");
		context->setPreviousMap("");
		context->setId("");
		context_spread(context);
	}

	return true;
}

/**************************************
 Return false on error
 **************************************/
static int manage_playable_character_list(Connection & connection)
{
	character_playable_send_list(connection);

	return true;
}

/**************************************
 Return false on error
 **************************************/
static int manage_user_character_list(Connection & connection)
{
	character_user_send_list(connection);

	return true;
}

/**************************************
 Return false on error
 **************************************/
static int manage_create(Connection & connection, const pb::Create& create)
{
	const std::pair<bool, std::string> file_name = file_new(CHARACTER_TABLE, create.name());

	if (file_name.first == false)
	{
		werr(LOGUSER, "%s already exists", create.name().c_str());
		return false;
	}

	if (file_copy(CHARACTER_TEMPLATE_TABLE, create.id().c_str(),
	CHARACTER_TABLE, create.name().c_str()) == false)
	{
		werr(LOGUSER, "Error copying character template %s to character %s (maybe template doesn't exists ?)", create.id(), create.name());
		file_delete(CHARACTER_TABLE, create.name());
		return false;
	}

	if (entry_write_string(CHARACTER_TABLE, create.name().c_str(), create.name().c_str(),
	CHARACTER_KEY_NAME, nullptr) == false)
	{
		werr(LOGUSER, "Error setting character name %s", create.name().c_str());
		file_delete(CHARACTER_TABLE, create.name());
		return false;
	}

	if (entry_add_to_list(USERS_TABLE, connection.getUserName().c_str(), create.name().c_str(),
	USERS_CHARACTER_LIST, nullptr) == false)
	{
		werr(LOGUSER, "Error adding character %s to user %s", create.name().c_str(), connection.getUserName().c_str());
		file_delete(CHARACTER_TABLE, create.name());
		return false;
	}

	character_user_send(connection, create.name().c_str());

	wlog(LOGDEVELOPER, "Successfully created: ID=%s, NAME=%s", create.id().c_str(), create.name().c_str());

	return true;
}

/**************************************
 Return false on error
 **************************************/
static int manage_action(Connection & connection, const pb::Action& action)
{
	std::vector<std::string> params;
	for (int i = 0; i < action.params_size(); i++)
	{
		params.push_back(action.params(i));
	}

	Context * context = context_find(connection.getContextId());
	action_execute(context, action.action(), params);

	return true;
}

/**************************************
 Return false on error
 **************************************/
static int manage_file(Connection & connection, const pb::File& file)
{
	const std::string file_path = base_directory + "/" + file.name();

	std::pair<bool, std::string> crc = checksum_file(file_path);

	if (crc.first == false)
	{
		werr(LOGUSER, "Required file %s doesn't exists", file_path.c_str());
		return false;
	}

	if (file.crc() == crc.second)
	{
		wlog(LOGDEVELOPER, "Client has already newest %s file", file.name().c_str());
		return false;
	}

	network_send_file(connection, file.name().c_str());

	return true;
}

/**************************************
 Return false on error
 **************************************/
int parse_incoming_data(Connection & connection, const std::string & serialized_data)
{
	pb::ClientMessage message;
	if (message.ParseFromString(serialized_data) == false)
	{
		werr(LOGUSER, "Parsing failed");
	}
	else
	{
		if (message.has_file())
		{
			wlog(LOGDEVELOPER, "[network] Received file request for %s", message.file().name().c_str());
			manage_file(connection, message.file());
		}
		else if (message.has_login())
		{
			wlog(LOGDEVELOPER, "[network] Received login request for user %s", message.login().user().c_str());
			manage_login(connection, message.login());
		}
		else if (connection.isConnected() == false)
		{
			werr(LOGUSER, "Request from not authenticated client, close connection");
		}
		else if (message.has_start())
		{
			wlog(LOGDEVELOPER, "[network] Received start");
			manage_start(connection, message.start());
		}
		else if (message.has_stop())
		{
			wlog(LOGDEVELOPER, "[network] Received stop request for ID %s of user %s", connection.getUserName().c_str(),
					context_find(connection.getContextId())->getId().c_str());
			manage_stop(connection, message.stop());
		}
		else if (message.has_playable_character_list())
		{
			wlog(LOGDEVELOPER, "[network] Received playable character list request");
			manage_playable_character_list(connection);
		}
		else if (message.has_user_character_list())
		{
			wlog(LOGDEVELOPER, "[network] Received user character list request for user %s", message.user_character_list().user().c_str());
			manage_user_character_list(connection);
		}
		else if (message.has_create())
		{
			wlog(LOGDEVELOPER, "[network] Received create ID %s with name %s", message.create().id().c_str(), message.create().name().c_str());
			manage_create(connection, message.create());
		}
		else if (message.has_action())
		{
			wlog(LOGDEVELOPER, "[network] Received action %s", message.action().action().c_str());
			manage_action(connection, message.action());
		}
		else
		{
			werr(LOGUSER, "Unknown message received");
		}
	}

	return true;
}
