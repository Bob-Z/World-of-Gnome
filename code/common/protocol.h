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
#define CMD_LOGIN_USER			1
// login with password , followed by the password itself (naive isn't it ?), return OK if password correspond to user
#define CMD_LOGIN_PASSWORD		2
#define CMD_LOGIN_OK			3
#define CMD_LOGIN_NOK			4
#define CMD_REQ_AVATAR_LIST		5  /* Request the available avatar types on the server */
#define CMD_SEND_AVATAR			6  /* Add an avatar type to the list of available avatar */
#define CMD_REQ_FILE			7  /* require a file's content */
#define CMD_SEND_FILE			8 /* send a file's content */
#define CMD_REQ_USER_AVATAR_LIST	9 /* Require the list of a user's avatar */
#define CMD_SEND_USER_AVATAR		10 /* Send a user's avatar data */
#define CMD_SEND_CONTEXT		11 /* Send a context */
#define CMD_SEND_TEXT			12 /* Server sends a message to client */
#define CMD_SEND_ACTION			13 /* Client sends the name of an action to be executed by the server. */
#define CMD_SEND_ENTRY			14 /* Sends an entry to be updated on target */

#endif
