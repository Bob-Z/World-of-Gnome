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
#include "Context.h"
#include "DataManager.h"
#include "global.h"
#include "network_server.h"
#include <client_server.h>
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

/*****************************************************************************/
static void manage_login(Connection &connection, const pb::Login &login)
{
	try
	{
		std::string passwd = getDataManager().get<std::string>(PASSWD_TABLE,
				login.user(),
				{ PASSWD_KEY_PASSWORD });

		if (passwd != login.password())
		{
			ERR_USER("[network] Wrong login for %s" + login.user());
			network_send_login_nok(connection);
			return;
		}
		else
		{
			connection.setUserName(login.user());

			connection.setConnected(true);

			network_send_login_ok(connection);

			LOG_USER(
					"[network] Login successful for user "
							+ connection.getUserName());

			return;
		}

	} catch (...)
	{
		ERR_USER("[network] Password error for user " + login.user());
		network_send_login_nok(connection);
		return;
	}
}

/*****************************************************************************/
static void manage_start(Connection &connection, const pb::Start &start)
{
	connection.setContextId(start.id());

	Context *context = context_new();
	context->setId(start.id());
	context->setInGame(true);
	context->setConnection(&connection);

	// A missing character file resolves to an empty JSON value; a real
	// character file is an object (even when optional keys are absent).
	if (getDataManager().getJson(CHARACTER_TABLE + "/" + start.id()).is_object()
			== false)
	{
		ERR_USER("Cannot start with character " + start.id() + ": character not found");
		context_free(context);
		connection.setContextId("");
		connection.disconnect();
		return;
	}

	context->update_from_file();
	context_spread(context);
	context_request_other_context(context);

	LOG("[network] received start request for ID " + start.id() + " and user " + connection.getUserName());
}

/*****************************************************************************/
static void manage_stop(Connection &connection, const pb::Stop &stop)
{
	Context *context = context_find(connection.getContextId());

	if (context->isInGame() == true)
	{
		context->setInGame(false);
		context->setMap("");
		context->setPreviousMap("");
		context->setId("");
		context_spread(context);
	}
}

/*****************************************************************************/
static void manage_playable_character_list(Connection &connection)
{
	character_playable_send_list(connection);
}

/*****************************************************************************/
static void manage_user_character_list(Connection &connection)
{
	character_user_send_list(connection);
}

/*****************************************************************************/
static void manage_create(Connection &connection, const pb::Create &create)
{
	const std::pair<bool, std::string> file_name = file_new(CHARACTER_TABLE,
			create.name());

	if (file_name.first == false)
	{
		ERR_USER(create.name() + " already exists");
		return;
	}

	if (file_copy(CHARACTER_TEMPLATE_TABLE, create.id(),
	CHARACTER_TABLE, create.name()) == false)
	{
		ERR_USER("Error copying character template " + create.id() + " to character " + create.name() + " (maybe template doesn't exists ?)");
		file_delete(CHARACTER_TABLE, create.name());
		return;
	}

	if (entry_write_string(CHARACTER_TABLE, create.name(),
			create.name(), { CHARACTER_KEY_NAME }) == false)
	{
		ERR_USER("Error setting character name " + create.name());
		file_delete(CHARACTER_TABLE, create.name());
		return;
	}

	try
	{
		getDataManager().add(USERS_TABLE, connection.getUserName(),
		{ USERS_CHARACTER_LIST }, create.name());
	} catch (...)
	{
		ERR_USER(
				"Error adding character " + create.name() + " to user "
						+ connection.getUserName());
		file_delete(CHARACTER_TABLE, create.name());
		return;
	}

	character_user_send(connection, create.name());

	LOG("Successfully created: ID=" + create.id() + ", NAME=" + create.name());
}

/*****************************************************************************/
static void manage_action(Connection &connection, const pb::Action &action)
{
	std::vector<std::string> params;
	for (int i = 0; i < action.params_size(); i++)
	{
		params.push_back(action.params(i));
	}

	Context *context = context_find(connection.getContextId());
	action_run_or_execute(context, action.action(), params);
}

/*****************************************************************************/
static void manage_action_stop(Connection &connection,
		const pb::ActionStop &action)
{
	Context *context = context_find(connection.getContextId());
	action_stop(context, action.action());
}

/*****************************************************************************/
static void manage_file(Connection &connection, const pb::File &file)
{
	const std::string file_path = base_directory + "/" + file.name();

	std::pair<bool, std::string> crc = checksum_file(file_path);

	if (crc.first == false)
	{
		ERR_USER("Required file " + file_path + " doesn't exists");
		return;
	}

	if (file.crc() == crc.second)
	{
		LOG("Client has already newest " + file.name() + " file");
		return;
	}

	network_send_file(connection, file.name());
}

/**************************************
 Return false on error
 **************************************/
int parse_incoming_data(Connection &connection,
		const std::string &serialized_data)
{
	pb::ClientMessage message;
	if (message.ParseFromString(serialized_data) == false)
	{
		ERR_USER("Parsing failed");
	}
	else
	{
		if (message.has_file())
		{
			LOG("[network] Received file request for " + message.file().name());
			manage_file(connection, message.file());
		}
		else if (message.has_login())
		{
			LOG("[network] Received login request for user " + message.login().user());
			manage_login(connection, message.login());
		}
		else if (connection.isConnected() == false)
		{
			ERR_USER("Request from not authenticated client, close connection");
		}
		else if (message.has_start())
		{
			LOG("[network] Received start");
			manage_start(connection, message.start());
		}
		else if (message.has_stop())
		{
			LOG("[network] Received stop request for ID " + connection.getUserName() + " of user " + context_find(connection.getContextId())->getId());
			manage_stop(connection, message.stop());
		}
		else if (message.has_playable_character_list())
		{
			LOG("[network] Received playable character list request");
			manage_playable_character_list(connection);
		}
		else if (message.has_user_character_list())
		{
			LOG("[network] Received user character list request for user " + message.user_character_list().user());
			manage_user_character_list(connection);
		}
		else if (message.has_create())
		{
			LOG("[network] Received create ID " + message.create().id() + " with name " + message.create().name());
			manage_create(connection, message.create());
		}
		else if (message.has_action())
		{
			LOG("[network] Received action " + message.action().action());
			manage_action(connection, message.action());
		}
		else if (message.has_action_stop())
		{
			LOG("[network] Received action stop " + message.action_stop().action());
			manage_action_stop(connection, message.action_stop());
		}
		else
		{
			ERR_USER("Unknown message received");
		}
	}

	return true;
}
