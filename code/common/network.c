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

#include "common.h"
#include <string.h>
#include <dirent.h>
#include "file.h"

typedef struct send_data {
	Uint32 command;
	Uint32 count;
	char *data;
	int is_data;
	context_t * context;
} send_data_t;

/*********************************************************************
*********************************************************************/
static int async_send(void * user_data)
{
	TCPsocket socket = 0;
	int bytes_written = 0;
	int length = 0;
	send_data_t * data = (send_data_t *)user_data;
	context_t * context = data->context;

	if(data->is_data ) {
		socket = context_get_socket_data(context);
	} else {
		socket = context_get_socket(context);
	}

	if( socket == 0 ) {
		wlog(LOGDEBUG, "socket %d is disconnected",socket);
		return FALSE;
	}

	SDL_LockMutex(context->send_mutex);

	//send command
	length = sizeof(Uint32);
	bytes_written = SDLNet_TCP_Send(socket,&data->command,length);
	if( bytes_written != length ) {
		werr(LOGUSER,"Could not send command to %s",context->id);
		context_set_connected(context,FALSE);
		goto async_send_end;
	}

	//send data size
	length = sizeof(Uint32);
	bytes_written = SDLNet_TCP_Send(socket,&data->count,length);
	if( bytes_written != length ) {
		werr(LOGUSER,"Could not send command to %s",context->id);
		context_set_connected(context,FALSE);
		goto async_send_end;
	}

	//send optional data
	if(data->count > 0) {
		length = data->count;
		bytes_written = SDLNet_TCP_Send(socket,data->data,length);
		if( bytes_written != length ) {
			werr(LOGUSER,"Could not send command to %s",context->id);
			context_set_connected(context,FALSE);
			goto async_send_end;
		}
	}

async_send_end:
	SDL_UnlockMutex(context->send_mutex);
	free(data->data);
	free(data);

	return FALSE;
}

/*******************************
*******************************/
void network_send_command(context_t * context, Uint32 command, long int count, const char *data, int is_data)
{
	send_data_t * send_data;

	send_data = malloc(sizeof(send_data_t));
	send_data->command = command;
	send_data->count = count;
	send_data->data = malloc(count);
	memcpy(send_data->data,data,count);
	send_data->is_data = is_data;
	send_data->context = context;

	SDL_CreateThread(async_send,"async_send",(void*)send_data);
}

/*********************************************************************
Asks to update an int entry on  a context
*********************************************************************/
void network_send_entry_int(context_t * context, const char * table, const char * file, const char *path, int value)
{
	char * frame;
	char buf[SMALL_BUF];

	sprintf(buf,"%d",value);

	frame = strconcat(ENTRY_TYPE_INT,NETWORK_DELIMITER,table,NETWORK_DELIMITER,file,NETWORK_DELIMITER,path,NETWORK_DELIMITER,buf,NULL);

	wlog(LOGDEBUG,"Send CMD_SEND_ENTRY to %s :%s",context->id,frame);
	network_send_command(context, CMD_SEND_ENTRY, strlen(frame)+1, frame,FALSE);

	free(frame);
}

/*********************************************************************
Client request a file
It adds the local file checksum so that the server only send the file if it is different
It make sure there are a minimun time between to consecutive request on the same file
*********************************************************************/
void network_send_req_file(context_t * context, char * file)
{
	char * filename;
	char * cksum;
	char * frame;

	/* Sanity check */
	if(file == NULL) {
		werr(LOGDEV,"network_send_req_file_checksum called with NULL");
		return;
	}

	/* Compute checksum of local file */
	filename = strconcat(base_directory,"/",file,NULL);

	cksum = checksum_file(filename);
	if( cksum == NULL ) {
		cksum = "0";
	}
	free(filename);

	frame = strconcat(file,NETWORK_DELIMITER,cksum,NULL);

	wlog(LOGDEBUG,"Send CMD_REQ_FILE :%s",file);
	network_send_command(context, CMD_REQ_FILE, strlen(frame)+1, frame,TRUE);
	free(frame);

	if(cksum[0] != '0' && cksum[1] != 0) {
		free(cksum);
	}
}

/*********************************************************************
Client send its context to server for quick  update
*********************************************************************/
void network_send_context(context_t * context)
{
	char data[BIG_BUF];
	char itoa[SMALL_BUF];
	int  data_size = 0;
	int  size = 0;
	char * selected_id = NULL;
	char * selected_map = NULL;

	size = strlen(context->user_name)+1;
	memcpy(data+data_size, context->user_name, size);
	data_size += size;

	size = strlen(context->character_name)+1;
	memcpy(data+data_size, context->character_name, size);
	data_size += size;

	size = strlen(context->map)+1;
	memcpy(data+data_size, context->map, size);
	data_size += size;

	snprintf(itoa,sizeof(itoa),"%d",context->in_game);
	size = strlen(itoa)+1;
	memcpy(data+data_size, itoa, size);
	data_size += size;

	snprintf(itoa,sizeof(itoa),"%d",context->connected);
	size = strlen(itoa)+1;
	memcpy(data+data_size, itoa, size);
	data_size += size;

	snprintf(itoa,sizeof(itoa),"%d",context->pos_x);
	size = strlen(itoa)+1;
	memcpy(data+data_size, itoa, size);
	data_size += size;

	snprintf(itoa,sizeof(itoa),"%d",context->pos_y);
	size = strlen(itoa)+1;
	memcpy(data+data_size, itoa, size);
	data_size += size;

	size = strlen(context->type)+1;
	memcpy(data+data_size, context->type, size);
	data_size += size;

	snprintf(itoa,sizeof(itoa),"%d",context->tile_x);
	size = strlen(itoa)+1;
	memcpy(data+data_size, itoa, size);
	data_size += size;

	snprintf(itoa,sizeof(itoa),"%d",context->tile_y);
	size = strlen(itoa)+1;
	memcpy(data+data_size, itoa, size);
	data_size += size;

	size = strlen(context->id)+1;
	memcpy(data+data_size, context->id, size);
	data_size += size;

	if( context->selection.id == NULL ) {
		selected_id = "";
	} else {
		selected_id = context->selection.id;
	}
	size = strlen(selected_id)+1;
	memcpy(data+data_size, selected_id, size);
	data_size += size;

	snprintf(itoa,sizeof(itoa),"%d",context->selection.map_coord[0]);
	size = strlen(itoa)+1;
	memcpy(data+data_size, itoa, size);
	data_size += size;

	snprintf(itoa,sizeof(itoa),"%d",context->selection.map_coord[1]);
	size = strlen(itoa)+1;
	memcpy(data+data_size, itoa, size);
	data_size += size;

	if( context->selection.map == NULL ) {
		selected_map = "";
	} else {
		selected_map = context->selection.map;
	}
	size = strlen(selected_map)+1;
	memcpy(data+data_size, selected_map, size);
	data_size += size;

	if( context->selection.inventory == NULL ) {
		selected_id = "";
	} else {
		selected_id = context->selection.inventory;
	}
	size = strlen(selected_id)+1;
	memcpy(data+data_size, selected_id, size);
	data_size += size;

	if( context->selection.equipment == NULL ) {
		selected_id = "";
	} else {
		selected_id = context->selection.equipment;
	}
	size = strlen(selected_id)+1;
	memcpy(data+data_size, selected_id, size);
	data_size += size;

	wlog(LOGDEBUG,"Send CMD_SEND_CONTEXT of %s",context->id);
	network_send_command(context, CMD_SEND_CONTEXT, data_size, data,FALSE);
}

/*********************************************************************
  Return FALSE on error, TRUE on OK
*********************************************************************/
int network_read_bytes(TCPsocket socket, char * data, int size)
{
	int bytes_read = 0;
	int total_bytes = 0;

	if( socket == 0 ) {
		return FALSE;
	}

	while( total_bytes != size && bytes_read != -1 ) {
		bytes_read = SDLNet_TCP_Recv(socket, data+total_bytes, size);
		if( bytes_read < 1 ) {
			werr(LOGDEBUG,"Read error on socket %d",socket);
			return FALSE;
		}
		total_bytes += bytes_read;
	}

	return TRUE;
}

/*********************************************************************
send the data of a context to another context
*********************************************************************/
void network_send_context_to_context(context_t * dest_ctx, context_t * src_ctx)
{
	char data[BIG_BUF];
	char itoa[SMALL_BUF];
	int  data_size = 0;
	int  size = 0;

	/* Source context is not ready yet */
	if( src_ctx->user_name == NULL ) {
		return;
	}

	size = strlen(src_ctx->user_name)+1;
	memcpy(data+data_size, src_ctx->user_name, size);
	data_size += size;

	size = strlen(src_ctx->character_name)+1;
	memcpy(data+data_size, src_ctx->character_name, size);
	data_size += size;

	size = strlen(src_ctx->map)+1;
	memcpy(data+data_size, src_ctx->map, size);
	data_size += size;

	snprintf(itoa,sizeof(itoa),"%d",src_ctx->in_game);
	size = strlen(itoa)+1;
	memcpy(data+data_size, itoa, size);
	data_size += size;

	snprintf(itoa,sizeof(itoa),"%d",src_ctx->connected);
	size = strlen(itoa)+1;
	memcpy(data+data_size, itoa, size);
	data_size += size;

	snprintf(itoa,sizeof(itoa),"%d",src_ctx->pos_x);
	size = strlen(itoa)+1;
	memcpy(data+data_size, itoa, size);
	data_size += size;

	snprintf(itoa,sizeof(itoa),"%d",src_ctx->pos_y);
	size = strlen(itoa)+1;
	memcpy(data+data_size, itoa, size);
	data_size += size;

	size = strlen(src_ctx->type)+1;
	memcpy(data+data_size, src_ctx->type, size);
	data_size += size;

	snprintf(itoa,sizeof(itoa),"%d",src_ctx->tile_x);
	size = strlen(itoa)+1;
	memcpy(data+data_size, itoa, size);
	data_size += size;

	snprintf(itoa,sizeof(itoa),"%d",src_ctx->tile_y);
	size = strlen(itoa)+1;
	memcpy(data+data_size, itoa, size);
	data_size += size;

	size = strlen(src_ctx->id)+1;
	memcpy(data+data_size, src_ctx->id, size);
	data_size += size;

	wlog(LOGDEBUG,"Send CMD_SEND_CONTEXT of %s to %s",src_ctx->id,dest_ctx->id);
	network_send_command(dest_ctx, CMD_SEND_CONTEXT, data_size, data,FALSE);
}

/*********************************************************************
filename is relative to the data dir

send a file to a context
return 0 on success
*********************************************************************/
int network_send_file(context_t * context, char * filename)
{
	char * file_data = NULL;
	int file_length = 0;
	int res = 0;
	Uint32 count = 0;
	char * frame = NULL;
	char * ptr = NULL;
	TCPsocket socket = 0;

	/* Check if NPC */
	socket = context_get_socket(context);
	if( socket == 0 ) {
		return -1;
	}

	/* Never send files with password */
	if ( strstr(filename,PASSWD_TABLE) != NULL ) {
		werr(LOGUSER,"send_file : Do not serve hazardous file  \"%s\"",filename);
		return -1;
	}

	/* Read the file */
	res = file_get_contents(filename,&file_data,&file_length);
	if( res == FALSE) {
		werr(LOGUSER,"send_file : Error reading file \"%s\"",filename);
		return -1;
	}

	/* Prepare the frame = file_name_size + file_name + file_data_size + file_data*/
	count = sizeof(Uint32)+strlen(filename)+1+sizeof(Uint32)+file_length;
	frame = malloc(count);
	if( frame == NULL) {
		free(file_data);
		werr(LOGUSER,"send_file : Error allocating memory");
		return -1;
	}

	/* first the filename size */
	*((Uint32 *)frame) = strlen(filename)+1;
	ptr = frame;
	ptr += sizeof(Uint32);

	/* then the name of the file itself */
	memcpy(ptr,filename,strlen(filename)+1);
	ptr += strlen(filename) + 1;

	/* then the size of the file data */
	*((Uint32 *)ptr) = file_length;
	ptr += sizeof(Uint32);

	/* then the file data itself */
	memcpy(ptr,file_data,file_length);

	free(file_data);

	/* send the frame */
	wlog(LOGDEBUG,"Send CMD_SEND_FILE : %s",filename);
	network_send_command(context, CMD_SEND_FILE, count, frame,FALSE);

	free(frame);

	return 0;
}

/*********************************************************************
send table/file to a context
return FALSE on success
*********************************************************************/
int network_send_table_file(context_t * context, char * table, const char * id)
{
	char * filename;
	int ret;

	filename = strconcat(table,"/",id,NULL);
	ret = network_send_file(context,filename);
	free(filename);

	return ret;
}

/*********************************************************************
*********************************************************************/
void network_send_text(const char * id, const char * string)
{
	context_t * context = context_find(id);
	if( context == NULL ) {
		werr(LOGDEV,"Could not find context %s",id);
		return;
	}

	wlog(LOGDEBUG,"Send CMD_SEND_TEXT :\"%s\" to %s (%s)",string,context->character_name,context->user_name);
	network_send_command(context, CMD_SEND_TEXT, strlen(string)+1, string,FALSE);
}
