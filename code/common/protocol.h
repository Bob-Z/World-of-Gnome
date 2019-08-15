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

const std::string ENTRY_TYPE_INT("int");
const std::string ENTRY_TYPE_STRING("string");

// Special action script name for chat
#define WOG_CHAT	"__wog_chat__"

//List of command :
// *_REQ_* are sent from client to server
// *_SEND_* are sent from server to client

#define CMD_PB			99 //

enum class EffectType
{
	CONTEXT, MAP
};

#endif

