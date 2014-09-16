/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2014 carabobz@gmail.com

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
// second 4 bytes containg the size of additional data
// following bytes of additional data

#define NETWORK_DELIMITER "|||"

#define ENTRY_TYPE_INT "int"
#define ENTRY_TYPE_STRING "string"

/* Special action script name for chat */
#define WOG_CHAT	"__wog_chat__"

//List of command :

// login with user name , followed by the user name itself, return OK if user exists
#define CMD_LOGIN			1 /* user name followed by password in plain text FIXME */
#define CMD_LOGIN_OK			3
#define CMD_LOGIN_NOK			4
#define CMD_REQ_CHARACTER_LIST		5  /* Request the available characters template on the server */
#define CMD_SEND_CHARACTER		6  /* Add a character template to the list of available character */
#define CMD_REQ_FILE			7  /* require a file's content */
#define CMD_SEND_FILE			8 /* send a file's content */
#define CMD_REQ_USER_CHARACTER_LIST	9 /* Require the list of a user's character */
#define CMD_SEND_USER_CHARACTER		10 /* Send a user's character data */
#define CMD_SEND_CONTEXT		11 /* Send a context */
#define CMD_SEND_TEXT			12 /* Server sends a message to client */
#define CMD_SEND_ACTION			13 /* Client sends the name of an action to be executed by the server. */
#define CMD_SEND_ENTRY			14 /* Sends an entry to be updated on target */
#define CMD_SEND_SPEAK			16 /* NPC is speaking */

#endif
