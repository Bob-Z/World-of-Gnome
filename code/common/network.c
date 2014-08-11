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

#include <glib.h>
#include <gio/gio.h>
#include <glib/gprintf.h>
#include "common.h"
#include <string.h>
#include <dirent.h>
#include "file.h"

typedef struct send_data {
	Uint32 command;
	long int count;
	char *data;
	int is_data;
	context_t * context;
} send_data_t;

/*********************************************************************
*********************************************************************/
static int async_send(void * user_data)
{
	GError * err = NULL;
	int res;
	gsize bytes_written;
	GOutputStream * stream;
	send_data_t * data = (send_data_t *)user_data;
	context_t * context = data->context;

	if( data->is_data == FALSE ) {
		stream = context->output_stream;
	} else {
		stream = context->output_data_stream;
	}

	SDL_LockMutex(context->send_mutex);

	//send command
	res = g_output_stream_write_all(stream,&data->command,sizeof(Uint32),&bytes_written,NULL,&err);
	if( res == FALSE ) {
		werr(LOGUSER,"Could not send command to %s",context->id);
		goto async_send_end;
	}

	//send additional data size
	Uint32 size = data->count; // Force this to uint because gsize size is platform dependant
	res = g_output_stream_write_all(stream,&size,sizeof(Uint32),&bytes_written,NULL,&err);
	if( res == FALSE ) {
		werr(LOGUSER,"Could not send command to %s",context->id);
		goto async_send_end;
	}

	//send optional data
	if(data->count > 0) {
		res = g_output_stream_write_all(stream,data->data,data->count,&bytes_written,NULL,&err);
		if(res == FALSE) {
			werr(LOGUSER,"Could not send command to %s",context->id);
			goto async_send_end;
		}
	}

async_send_end:
//	g_output_stream_flush(stream,NULL,NULL);
	SDL_UnlockMutex(context->send_mutex);
	free(data->data);
	free(data);

	return 0;
}

/*******************************
*******************************/
void network_send_command(context_t * context, Uint32 command, long int count, const char *data, int is_data)
{
	GOutputStream * stream;
	send_data_t * send_data;

	/* Early check to see if context is connected */
	if(is_data ) {
		stream = context->output_data_stream;
	} else {
		stream = context_get_output_stream(context);
	}
	if( stream == NULL ) {
		wlog(LOGDEBUG, "NPC %s is trying to send a command",context->id);
		/* Probably called by a NPC */
		return;
	}

	send_data = malloc(sizeof(send_data_t));
	send_data->command = command;
	send_data->count = count;
	send_data->data = malloc(count);
	memcpy(send_data->data,data,count);
	send_data->is_data = is_data;
	send_data->context = context;

	SDL_CreateThread(async_send,"async_send",(void*)send_data);
}

/**********************
Clients functions
**********************/
/*****************************
High level commands
*****************************/

/*********************************************************************
sends a login request, the answer is asynchronously read by async_recv
**********************************************************************/
void network_login(context_t * context, const char * name, const char * password)
{
	wlog(LOGDEBUG,"Send CMD_LOGIN_USER");
	network_send_command(context, CMD_LOGIN_USER, strlen(name) + 1, name,FALSE);
	wlog(LOGDEBUG,"Send CMD_LOGIN_PASSWORD");
	network_send_command(context, CMD_LOGIN_PASSWORD, strlen(password) + 1, password,FALSE);
}

/*********************************************************************
*********************************************************************/
void network_request_character_list(context_t * context)
{
	wlog(LOGDEBUG,"Send CMD_REQ_CHARACTER_LIST");
	network_send_command(context, CMD_REQ_CHARACTER_LIST, 0, NULL,FALSE);
}

/*********************************************************************
request a specific user's characters list
*********************************************************************/
void network_request_user_character_list(context_t * context)
{
	wlog(LOGDEBUG,"Send CMD_REQ_USER_CHARACTER_LIST");
	network_send_command(context, CMD_REQ_USER_CHARACTER_LIST, strlen(context->user_name)+1, context->user_name,FALSE);
}

/*********************************************************************
server sends a message to client
*********************************************************************/
void network_send_text(const char * id, const char * string)
{
	context_t * context = context_find(id);
	if( context == NULL ) {
		werr(LOGDEV,"Could not find context %s",id);
		return;
	}

	/* Early check to see if context is connected */
	GOutputStream * stream = context_get_output_stream(context);
	if( stream == NULL ) {
		werr(LOGDEV,"%s not connected",id);
		return;
	}

	wlog(LOGDEBUG,"Send CMD_SEND_TEXT :\"%s\" to %s (%s)",string,context->character_name,context->user_name);
	network_send_command(context, CMD_SEND_TEXT, strlen(string)+1, string,FALSE);
}

/*********************************************************************
Broadcast text to all connected players
*********************************************************************/
void network_broadcast_text(context_t * context, const char * text)
{
	context_t * ctx = NULL;

	SDL_LockMutex(context_list_mutex);

	ctx = context_get_list_first();

	if( ctx == NULL ) {
		SDL_UnlockMutex(context_list_mutex);
		return;
	}

	do {
		/* Skip if not connected (NPC) */
		if( ctx->output_stream == NULL ) {
			continue;
		}

		/* Skip data transmission network */
		if( ctx->user_name == NULL ) {
			continue;
		}

		/* Skip if not on the same map */
#if 0
		if( same_map_only ) {
			if( target == NULL ) {
				continue;
			}
			if( strcmp(target->map,ctx->map) != 0 ) {
				continue;
			}
		}
#endif
		network_send_text(ctx->id,text);
	} while( (ctx=ctx->next)!= NULL );

	SDL_UnlockMutex(context_list_mutex);
}

/*********************************************************************
server sends a command to be executed by the client
or client sends a list of basic command to be executed by server
*********************************************************************/
void network_send_action(context_t * context, char * script,...)
{
	va_list ap;
	char * frame;
	char * new_frame;
	char * parameter;

	frame = strdup(script);
	va_start(ap, script);
	while ( (parameter=va_arg(ap,char*)) != NULL ) {
		new_frame = strconcat(frame,NETWORK_DELIMITER,parameter,NULL);
		free(frame);
		frame = new_frame;
	}
	va_end(ap);

	wlog(LOGDEBUG,"Send CMD_SEND_ACTION :%s",frame);
	network_send_command(context, CMD_SEND_ACTION, strlen(frame)+1, frame,FALSE);
	free(frame);
}

/*********************************************************************
Server send the full character's file to client
*********************************************************************/
void network_send_character_file(context_t * context)
{
	char * filename;

	/* Check if this context is connected */
	GOutputStream * stream = context_get_output_stream(context);
	if( stream == NULL ) {
		return;
	}

	filename = strconcat(CHARACTER_TABLE,"/",context->id,NULL);
	network_send_file(context,filename);
	free(filename);
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
Asks to update an int entry on all connected contexts
*********************************************************************/
void network_broadcast_entry_int(const char * table, const char * file, const char * path, int value, int same_map_only)
{
	context_t * ctx = NULL;
	context_t * target = NULL;

	target = context_find(file);

	SDL_LockMutex(context_list_mutex);

	ctx = context_get_list_first();

	if( ctx == NULL ) {
		SDL_UnlockMutex(context_list_mutex);
		return;
	}

	do {
		/* Skip if not connected (NPC) */
		if( ctx->output_stream == NULL ) {
			continue;
		}
		/* Skip if not on the same map */
		if( same_map_only ) {
			if( target == NULL ) {
				continue;
			}
			if( target->map && ctx->map ) {
				if( strcmp(target->map,ctx->map) != 0 ) {
					continue;
				}
			}
		}

		network_send_entry_int(ctx,table,file, path, value);

	} while( (ctx=ctx->next)!= NULL );

	SDL_UnlockMutex(context_list_mutex);
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
int read_bytes(context_t * context, char * data, gsize size, int is_data)
{
	GError *err = NULL;
	int res;
	GInputStream * stream;
	gsize byte_read = 0;

	if(is_data) {
		stream = context->input_data_stream;
	} else {
		stream = context_get_input_stream(context);
	}

	res = g_input_stream_read_all(stream,data,size,&byte_read,NULL,&err);
	g_prefix_error(&err,"Error receiving data");

	if( res == FALSE) {
		//if(g_socket_is_connected(socket) ) {
		werr(LOGDEV,"Network error: %d bytes requested, %d received.",(int)size,(int)byte_read);
		return FALSE;
	}
	/*
	        if(byte_read == 0 ) {
	                //if(g_socket_is_connected(socket) ) {
	                g_warning("Network error: No data received");
	                return FALSE;
	        }
	*/

	return TRUE;
}

/*********************************************************************
Callback from client listening to server in its own thread
only used for game information transfer
*********************************************************************/
static int async_recv(void * data)
{
	context_t * context = (context_t *)data;

	Uint32 command = 0;
	Uint32 command_size = 0;
	char *buf = NULL;

	do {
		command = 0;
		command_size = 0;
		buf = NULL;

		if( !read_bytes(context,(char *)&command, sizeof(Uint32),FALSE) ) {
			break;
		}
		/* Read a size */
		if( !read_bytes(context,(char *)&command_size, sizeof(Uint32),FALSE)) {
			break;
		}

		/* Read additional data */
		if( command_size > 0) {
			buf = malloc(command_size);
			if( !read_bytes(context,buf, command_size,FALSE) ) {
				break;
			}
		}

		if (!parse_incoming_data(context, command, command_size, buf) ) {
			if( buf ) {
				free(buf);
				buf = NULL;
			}
			break;
		}

		if( buf != NULL) {
			free(buf);
			buf = NULL;
		}

//	} while( bytes_read != 0);
//	} while( g_socket_is_connected(socket) );
	} while( ! g_io_stream_is_closed( (GIOStream *)context_get_connection(context)));

	werr(LOGUSER,"Socket closed on server side.");

	context_set_connected(context,FALSE);
	context_set_connection(context,NULL);
	context_set_input_stream(context,NULL);
	context_set_output_stream(context,NULL);

	parse_incoming_data(NULL,0,0,NULL);

	return 0;
}

/*********************************************************************
Callback from client listening to server in its own thread
Only used for data transfers
*********************************************************************/
static int async_data_recv(void * data)
{
	context_t * context = (context_t *)data;

	Uint32 command = 0;
	Uint32 command_size = 0;
	char *buf = NULL;

	do {
		command = 0;
		command_size = 0;
		buf = NULL;

		if( !read_bytes(context,(char *)&command, sizeof(Uint32),TRUE)) {
			break;
		}
		/* Read a size */
		if( !read_bytes(context,(char *)&command_size, sizeof(Uint32),TRUE) ) {
			break;
		}

		/* Read additional data */
		if( command_size > 0) {
			buf = malloc(command_size);
			if( !read_bytes(context,buf, command_size,TRUE) ) {
				break;
			}
		}

		if (!parse_incoming_data(context, command, command_size, buf) ) {
			if( buf ) {
				free(buf);
				buf = NULL;
			}
			break;
		}

		if( buf != NULL) {
			free(buf);
			buf = NULL;
		}

//	} while( bytes_read != 0);
//	} while( g_socket_is_connected(socket) );
	} while( ! g_io_stream_is_closed( (GIOStream *)context_get_connection(context)));

	werr(LOGUSER,"Socket closed on server side.");

	context_set_connected(context,FALSE);
	context_set_connection(context,NULL);
	context_set_input_stream(context,NULL);
	context_set_output_stream(context,NULL);

	parse_incoming_data(NULL,0,0,NULL);

	return 0;
}

/*********************************************************************
Called by client
return FALSE on error
return TRUE if OK
*********************************************************************/
int network_connect(context_t * context, const char * hostname)
{
	GSocketClient * client;
	GSocketConnection * connection = NULL;
	GError * error = NULL;

	if( context->hostname ) {
		free(context->hostname);
	}
	context->hostname = strdup(hostname);

	client = g_socket_client_new();
	connection = g_socket_client_connect_to_host(client,hostname,PORT,NULL,&error);
	if( connection == NULL ) {
		werr(LOGUSER,"Can't connect to server");
		return FALSE;
	}

	context_set_connection(context, connection);
	context_set_input_stream(context,g_io_stream_get_input_stream((GIOStream *)connection));
	context_set_output_stream(context,g_io_stream_get_output_stream((GIOStream *)connection));

	SDL_CreateThread(async_recv,"async_recv",(void*)context);

	return TRUE;
}

/*********************************************************************
*********************************************************************/
int network_open_data_connection(context_t * context)
{
	GSocketClient * client;
	GSocketConnection * connection = NULL;
	GError *error;

	client = g_socket_client_new();
	connection = g_socket_client_connect_to_host(client,context->hostname,PORT,NULL,&error);
	if( connection == NULL ) {
		werr(LOGUSER,"Can't open data connection to server");
		return FALSE;
	}

	context->data_connection = connection;
	context->input_data_stream = g_io_stream_get_input_stream((GIOStream *)connection);
	context->output_data_stream = g_io_stream_get_output_stream((GIOStream *)connection);

	SDL_CreateThread(async_data_recv,"data_connection",(void*)context);

	return TRUE;
}

/*********************
server functions
*********************/
/*********************************************************************
Callback function created in a new thread by GLib
*********************************************************************/
int network_connection(GThreadedSocketService *service,GSocketConnection *connection, GObject *source_object, gpointer user_data)
{
	wlog(LOGUSER,"Client connected");

	context_t * context;
	context = context_new();
	if(context == NULL ) {
		werr(LOGUSER,"Failed to create context");
		return TRUE;
	}

	context_set_connection(context,connection);
	context_set_input_stream(context,g_io_stream_get_input_stream((GIOStream *)connection));
	context_set_output_stream(context,g_io_stream_get_output_stream((GIOStream *)connection));

	context_new_VM(context);

	context_set_connected(context,TRUE);

	while(context_get_connected(context)) {
		Uint32 command = 0;
		Uint32 command_size = 0;
		char *buf = NULL;
		/* Read a command code */
		if( !read_bytes(context,(char *)&command, sizeof(Uint32),FALSE)) {
			context_set_connected(context,FALSE);
			break;
		}
		/* Read a size */
		if( !read_bytes(context,(char *)&command_size, sizeof(Uint32),FALSE)) {
			context_set_connected(context,FALSE);
			break;
		}

		/* Read additional data */
		if( command_size > 0) {
			buf = malloc(command_size);
			if( !read_bytes(context,buf, command_size,FALSE)) {
				context_set_connected(context,FALSE);
				break;
			}
		}

		if (!parse_incoming_data(context, command, command_size, buf) ) {
			if( buf ) {
				free(buf);
				buf = NULL;
			}
			context_set_connected(context,FALSE);
			break;
		}

		if( buf != NULL) {
			free(buf);
			buf = NULL;
		}
	}
	wlog(LOGUSER,"Client disconnected");
	context_spread(context);
	context_write_to_file(context);
	context_free(context);

	return TRUE;
}

/*********************************************************************
Need to be run before g_main_loop_run
*********************************************************************/
void network_init(void)
{
	GError *err = NULL;

	GSocketService * service;
	service = g_threaded_socket_service_new(MAX_CLIENT);

	g_socket_listener_add_inet_port ((GSocketListener *)service,PORT,NULL,&err);
	g_assert_no_error(err);

	g_signal_connect(service,"run", (GCallback)network_connection, NULL);
	g_socket_service_start(service);

	return;
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
	int res;
	Uint32 count;
	char * frame;
	char * ptr;

	/* Check if this context is connected */
	GOutputStream * stream = context_get_output_stream(context);
	if( stream == NULL ) {
		return 1;
	}

	/* Never send files with password */
	if ( strstr(filename,PASSWD_TABLE) != NULL ) {
		werr(LOGUSER,"send_file : Do not serve hazardous file  \"%s\"",filename);
		return 1;
	}

	/* Read the file */
	res = file_get_contents(filename,&file_data,&file_length);
	if( res == FALSE) {
		werr(LOGUSER,"send_file : Error reading file \"%s\"",filename);
		return 1;
	}

	/* Prepare the frame = file_name_size + file_name + file_data_size + file_data*/
	count = sizeof(Uint32)+strlen(filename)+1+sizeof(Uint32)+file_length;
	frame = malloc(count);
	if( frame == NULL) {
		free(file_data);
		werr(LOGUSER,"send_file : Error allocating memory");
		return 1;
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
return 0 on success
*********************************************************************/
int network_send_table_file(context_t * context, char * table, char * id)
{
	char * filename;
	int ret;

	filename = strconcat(table,"/",id,NULL);
	ret = network_send_file(context,filename);
	free(filename);

	return ret;
}
