/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2017 carabobz@gmail.com

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
#include "file.h"
#include <arpa/inet.h>
#include <dirent.h>
#include <string.h>

// FIXME create NetworkManager
class DataSent
{
public:
	context_t * m_pContext;
	NetworkFrame * m_pFrame;
	bool m_IsData;
};

/*********************************************************************
 return -1 on error
 return 0 on success
 *********************************************************************/
static int async_frame_send(void * p_pUserData)
{
	DataSent * l_pData = static_cast<DataSent*>(p_pUserData);

	context_t * l_pContext = l_pData->m_pContext;
	if (l_pContext == nullptr)
	{
		werr(LOGDEBUG, "null l_pContext");
		delete l_pData->m_pFrame;
		delete l_pData;
		return -1;
	}

	TCPsocket l_Socket = 0;

	if (l_pData->m_IsData)
	{
		l_Socket = context_get_socket_data(l_pContext);
	}
	else
	{
		l_Socket = context_get_socket(l_pContext);
	}

	if (l_Socket == 0)
	{
		wlog(LOGDEBUG, "socket %d is disconnected", l_Socket);
		delete l_pData->m_pFrame;
		delete l_pData;
		return -1;
	}

	SDL_LockMutex(l_pContext->send_mutex);

	//send frame size
	uint32_t l_Length = htonl(
			static_cast<uint32_t>(l_pData->m_pFrame->getSize()));

	int l_BytesWritten = SDLNet_TCP_Send(l_Socket, &l_Length, sizeof(l_Length));
	if (l_BytesWritten != sizeof(l_Length))
	{
		werr(LOGUSER, "Could not send command to %s", l_pContext->id);
		context_set_connected(l_pContext, false);
		goto async_frame_send_end;
	}

	//send frame
	l_BytesWritten = SDLNet_TCP_Send(l_Socket, l_pData->m_pFrame->getFrame(),
			l_pData->m_pFrame->getSize());
	if (l_BytesWritten != static_cast<int>(l_pData->m_pFrame->getSize()))
	{
		werr(LOGUSER, "Could not send command to %s", l_pContext->id);
		context_set_connected(l_pContext, false);
	}

	async_frame_send_end: SDL_UnlockMutex(l_pContext->send_mutex);
	delete l_pData->m_pFrame;
	delete l_pData;

	return RET_OK;
}

/*******************************************************************************
 ******************************************************************************/
void network_send_command(context_t * p_pContext, const uint_fast32_t p_Command,
		const NetworkFrame & p_rFrame, const bool p_IsData)
{
	// FIXME create a NetworkManager
	DataSent *l_pData = new (DataSent);
	NetworkFrame * l_pFrame = new (NetworkFrame);

	l_pFrame->push(p_Command);
	l_pFrame->push(p_rFrame);

	l_pData->m_pContext = p_pContext;
	l_pData->m_pFrame = l_pFrame;
	l_pData->m_IsData = p_IsData;

	SDL_CreateThread(async_frame_send, "async_frame_send", (void*) l_pData);
}

/*******************************************************************************
 ******************************************************************************/
void network_send_command_no_data(context_t * p_pContext, const uint_fast32_t p_Command,
const bool p_IsData)
{
	// FIXME create a NetworkManager
	DataSent *l_pData = new (DataSent);
	NetworkFrame * l_pFrame = new (NetworkFrame);

	l_pFrame->push(p_Command);

	l_pData->m_pContext = p_pContext;
	l_pData->m_pFrame = l_pFrame;
	l_pData->m_IsData = p_IsData;

	SDL_CreateThread(async_frame_send, "async_frame_send", (void*) l_pData);
}

/*******************************************************************************
 Asks to update an int entry on  a context
 ******************************************************************************/
void network_send_entry_int(context_t * context, const char * table,
		const char * file, const char *path, int value)
{
	NetworkFrame l_Frame;
	l_Frame.push(ENTRY_TYPE_INT);
	l_Frame.push(table);
	l_Frame.push(file);
	l_Frame.push(path);
	l_Frame.push(value);

	wlog(LOGDEBUG, "Send CMD_SEND_ENTRY to %s :%s", context->id,
			l_Frame.getFrame());
	network_send_command(context, CMD_SEND_ENTRY, l_Frame, false);
}

/*********************************************************************
 Client request a file
 It adds the local file checksum so that the server only send the file if it is different
 It make sure there are a minimun time between to consecutive request on the same file
 *********************************************************************/
void network_send_req_file(context_t * context, const char * file)
{
	// Sanity check
	if (file == nullptr)
	{
		werr(LOGDEV, "network_send_req_file_checksum called with nullptr");
		return;
	}

	// Compute checksum of local file
	char * filename = strconcat(base_directory, "/", file, nullptr);

	char * cksum = checksum_file(filename);
	if (cksum == nullptr)
	{
		cksum = strdup("0");
	}
	free(filename);

	NetworkFrame l_Frame;
	l_Frame.push(file);
	l_Frame.push(cksum);

	wlog(LOGDEBUG, "Send CMD_REQ_FILE :%s", file);
	network_send_command(context, CMD_REQ_FILE, l_Frame, true);

	free(cksum);
}

/*********************************************************************
 Return RET_NOK on error, RET_OK on OK
 *********************************************************************/
ret_code_t network_read_bytes(TCPsocket socket, char * data, int size)
{
	int bytes_read = 0;
	int total_bytes = 0;

	if (socket == 0)
	{
		return RET_NOK;
	}

	while (total_bytes != size && bytes_read != -1)
	{
		bytes_read = SDLNet_TCP_Recv(socket, data + total_bytes, 1);
		if (bytes_read < 1)
		{
			werr(LOGDEBUG, "Read error on socket %d", socket);
			return RET_NOK;
		}
		total_bytes += bytes_read;
	}

	return RET_OK;
}

/*********************************************************************
 send the data of a context to another context
 *********************************************************************/
void network_send_context_to_context(context_t * dest_ctx, context_t * src_ctx)
{
	// Skip if Dest context is an NPC
	if (context_is_npc(dest_ctx))
	{
		return;
	}
	// Source context is not ready yet
	if (src_ctx->in_game == 0)
	{
		return;
	}
	if (src_ctx->user_name == nullptr)
	{
		return;
	}

	NetworkFrame l_Frame;
	l_Frame.push(src_ctx->user_name);
	l_Frame.push(src_ctx->character_name);
	l_Frame.push(src_ctx->npc);
	l_Frame.push(src_ctx->map);
	l_Frame.push(src_ctx->in_game);
	l_Frame.push(src_ctx->connected);
	l_Frame.push(src_ctx->pos_tx);
	l_Frame.push(src_ctx->pos_ty);
	l_Frame.push(src_ctx->type);
	l_Frame.push(src_ctx->id);
	l_Frame.push(src_ctx->selection.id);
	l_Frame.push(src_ctx->selection.map);
	l_Frame.push(src_ctx->selection.map_coord[0]);
	l_Frame.push(src_ctx->selection.map_coord[1]);
	l_Frame.push(src_ctx->selection.equipment);
	l_Frame.push(src_ctx->selection.inventory);

	wlog(LOGDEBUG, "Send CMD_SEND_CONTEXT of %s to %s", src_ctx->id,
			dest_ctx->id);
	network_send_command(dest_ctx, CMD_SEND_CONTEXT, l_Frame, false);
}

/*********************************************************************
 filename is relative to the data dir

 send a file to a context
 return 0 on success
 *********************************************************************/
int network_send_file(context_t * context, char * filename)
{
	char * file_data = nullptr;
	int file_length = 0;
	int res = 0;

	// Check if NPC
	if (context_is_npc(context) == true)
	{
		return -1;
	}

	// Never send files with password
	if (strstr(filename, PASSWD_TABLE) != nullptr)
	{
		werr(LOGUSER, "send_file : Do not serve personal file  \"%s\"",
				filename);
		return -1;
	}

	// Read the file
	res = file_get_contents(filename, &file_data, &file_length);
	if (res == RET_NOK)
	{
		werr(LOGUSER, "send_file : Error reading file \"%s\"", filename);
		return -1;
	}

	//TODO Use NetworkFrame
	NetworkFrame l_Frame;

	l_Frame.push(filename);
	l_Frame.push(file_data);

	// send the frame
	wlog(LOGDEBUG, "Send CMD_SEND_FILE : %s", filename);
	network_send_command(context, CMD_SEND_FILE, l_Frame, false);

	return 0;
}

/*********************************************************************
 send table/file to a context
 return FALSE on success
 *********************************************************************/
int network_send_table_file(context_t * context, const char * table,
		const char * id)
{
	char * filename;
	int ret;

	filename = strconcat(table, "/", id, nullptr);
	ret = network_send_file(context, filename);
	free(filename);

	return ret;
}

/*********************************************************************
 *********************************************************************/
void network_send_text(const char * id, const char * string)
{
	context_t * context = context_find(id);
	if (context == nullptr)
	{
		werr(LOGDEV, "Could not find context %s", id);
		return;
	}

	//TODO use NetworkFrame
	NetworkFrame l_Frame;
	l_Frame.push(string);

	wlog(LOGDEBUG, "Send CMD_SEND_TEXT :\"%s\" to %s (%s)", string,
			context->character_name, context->user_name);
	network_send_command(context, CMD_SEND_TEXT, l_Frame, false);
}
