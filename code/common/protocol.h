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

#ifndef PROTOCOL_H
#define PROTOCOL_H

// Current, POC level, *very naive*, protocol implementation
// first 4 bytes containing a command code
// second 4 bytes containing the size of additional data
// following bytes of additional data

#define ENTRY_TYPE_INT "int"
#define ENTRY_TYPE_STRING "string"

// Special action script name for chat
#define WOG_CHAT	"__wog_chat__"

//List of command :
// *_REQ_* are sent from client to server
// *_SEND_* are sent from server to client

// login with user name , followed by the user name itself, return OK if user exists
#define CMD_SEND_LOGIN_OK		1
#define CMD_SEND_LOGIN_NOK		2
#define CMD_REQ_STOP			4  // Request to stop playing the sent ID
#define CMD_REQ_PLAYABLE_CHARACTER_LIST	5 // Require playable characters list
#define CMD_SEND_PLAYABLE_CHARACTER	6 // Send a playable character list
#define CMD_REQ_FILE			7 // require a file's content
#define CMD_SEND_FILE			8 // send a file's content
#define CMD_REQ_USER_CHARACTER_LIST	9 // Require user's characters list
#define CMD_SEND_USER_CHARACTER		10 // Send a user's character list
#define CMD_SEND_CONTEXT		11 // Send a context
#define CMD_SEND_TEXT			12 // Server sends a message to client
#define CMD_REQ_ACTION			13 // Client sends the name of an action to be executed by the server.
#define CMD_SEND_ENTRY			14 // Sends an entry to be updated on target
#define CMD_SEND_POPUP			15 // Send a popup window
#define CMD_REQ_CREATE			16 // Request a character creation
#define CMD_SEND_EFFECT			17 // Send an effect

#define CMD_PB			99 //

enum class EffectType
{
	CONTEXT, MAP
};

#endif

