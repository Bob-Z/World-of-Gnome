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

static GThread * listenThread = NULL;
extern context_t * context_list_start;

/* Date of the last request for a file (avoid server request flood for the same file */
static GHashTable* clockDB = NULL;
static void free_key(gpointer data)
{
	g_free(data);
}

static void free_value(gpointer data)
{
	g_date_time_unref(data);
}

typedef struct send_data {
	guint32 command;
	gsize count;
	gchar *data;
	gboolean is_data;
} send_data_t;

/*******************************
network_send_command

*******************************/
void network_send_command(context_t * context, guint32 command, gsize count, const gchar *data,gboolean is_data)
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

	send_data = g_malloc(sizeof(send_data_t));
	send_data->command = command;
	send_data->count = count;
	send_data->data = g_memdup(data,count);
	send_data->is_data = is_data;

	g_thread_pool_push(context->send_thread,send_data,NULL);
}

/*********************/
/* Clients functions */
/*********************/
/*****************************
High level commands
*****************************/


/* sends a login request, the answer is asynchronously read by async_recv */
void network_login(context_t * context, const gchar * name, const gchar * password)
{
	wlog(LOGDEBUG,"Send CMD_LOGIN_USER");
	network_send_command(context, CMD_LOGIN_USER, g_utf8_strlen(name,-1) + 1, name,FALSE);
	wlog(LOGDEBUG,"Send CMD_LOGIN_PASSWORD");
	network_send_command(context, CMD_LOGIN_PASSWORD, g_utf8_strlen(password,-1) + 1, password,FALSE);
}

/* request characters list */
void network_request_character_list(context_t * context)
{
	wlog(LOGDEBUG,"Send CMD_REQ_CHARACTER_LIST");
	network_send_command(context, CMD_REQ_CHARACTER_LIST, 0, NULL,FALSE);
}

/* request a specific user's characters list */
void network_request_user_character_list(context_t * context)
{
	wlog(LOGDEBUG,"Send CMD_REQ_USER_CHARACTER_LIST");
	network_send_command(context, CMD_REQ_USER_CHARACTER_LIST, g_utf8_strlen(context->user_name,-1)+1, context->user_name,FALSE);
}

/* server sends a message to client */
void network_send_text(const gchar * id, const gchar * string)
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
	network_send_command(context, CMD_SEND_TEXT, g_utf8_strlen(string,-1)+1, string,FALSE);
}

/* Broadcast text to all connected players */
void network_broadcast_text(context_t * context, const gchar * text)
{
	context_t * ctx = NULL;

	SDL_LockMutex(context_list_mutex);

	ctx = context_list_start;

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
		/*
		                if( same_map_only ) {
					if( target == NULL ) {
						continue;
					}
		                        if( g_strcmp0(target->map,ctx->map) != 0 ) {
		                                continue;
		                        }
		                }
		*/
		network_send_text(ctx->id,text);
	} while( (ctx=ctx->next)!= NULL );

	SDL_UnlockMutex(context_list_mutex);
}

/* server sends a command to be executed by the client */
/* or client sends a list of basic command to be executed by server */
void network_send_action(context_t * context, gchar * script,...)
{
	va_list ap;
	gchar * frame;
	gchar * new_frame;
	gchar * parameter;

	frame = g_strdup(script);
	va_start(ap, script);
	while ( (parameter=va_arg(ap,gchar*)) != NULL ) {
		new_frame = g_strconcat(frame,NETWORK_DELIMITER,parameter,NULL);
		g_free(frame);
		frame = new_frame;
	}
	va_end(ap);

	wlog(LOGDEBUG,"Send CMD_SEND_ACTION :%s",frame);
	network_send_command(context, CMD_SEND_ACTION, g_utf8_strlen(frame,-1)+1, frame,FALSE);

	g_free(frame);
}

/* Server send the full character's file to client */
void network_send_character_file(context_t * context)
{
	gchar * filename;

	/* Check if this context is connected */
	GOutputStream * stream = context_get_output_stream(context);
	if( stream == NULL ) {
		return;
	}

	filename = g_strconcat(CHARACTER_TABLE,"/",context->id,NULL);
	network_send_file(context,filename);
	g_free(filename);
}

/* Asks to update an int entry on  a context */
void network_send_entry_int(context_t * context, const gchar * table, const gchar * file, const gchar *path, int value)
{
	gchar * frame;
	gchar buf[SMALL_BUF];

	g_sprintf(buf,"%d",value);

	frame = g_strconcat(ENTRY_TYPE_INT,NETWORK_DELIMITER,table,NETWORK_DELIMITER,file,NETWORK_DELIMITER,path,NETWORK_DELIMITER,buf,NULL);

	wlog(LOGDEBUG,"Send CMD_SEND_ENTRY to %s :%s",context->id,frame);
	network_send_command(context, CMD_SEND_ENTRY, g_utf8_strlen(frame,-1)+1, frame,FALSE);
}

/* Asks to update an int entry on all connected contexts */
void network_broadcast_entry_int(const gchar * table, const gchar * file, const gchar * path, gint value, gboolean same_map_only)
{
	context_t * ctx = NULL;
	context_t * target = NULL;

	target = context_find(file);

	SDL_LockMutex(context_list_mutex);

	ctx = context_list_start;

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
			if( g_strcmp0(target->map,ctx->map) != 0 ) {
				continue;
			}
		}

		network_send_entry_int(ctx,table,file, path, value);

	} while( (ctx=ctx->next)!= NULL );

	SDL_UnlockMutex(context_list_mutex);
}

/* Client request a file */
/* It adds the local file checksum so that the server only send the file if it is different */
/* It make sure there are a minimun time between to consecutive request on the same file */
void network_send_req_file(context_t * context, gchar * file)
{
	gchar * filename;
	gchar * cksum;
	gchar * frame;
	GDateTime * last_date = NULL;
	GDateTime * current_date = NULL;
	GDateTime * max_date = NULL;

	/* Sanity check */
	if(file == NULL) {
		werr(LOGDEV,"network_send_req_file_checksum called with NULL");
		return;
	}

	/* init clockDB if necessary */
	if( clockDB == NULL ) {
		clockDB = g_hash_table_new_full(g_str_hash,g_str_equal, free_key, free_value);
	}

	/* Ckeck that a previous request for this file has not been send recently */
	last_date = g_hash_table_lookup(clockDB,file);
	current_date = g_date_time_new_now_local();
	if( last_date != NULL ) {
		max_date = g_date_time_add_seconds(last_date, FILE_REQUEST_TIMEOUT);
		/* max_date < current_date we can send a request */
		if( g_date_time_compare( max_date, current_date ) == -1 ) {
			g_hash_table_replace(clockDB, g_strdup(file), current_date);
			g_date_time_unref(max_date);
		}
		/* max_date > current_date, we cancel this request */
		else {
			werr(LOGDEBUG,"Previous request of file  %s has been done in a time too short",file);
			g_date_time_unref(max_date);
			g_date_time_unref(current_date);
			return;
		}
	} else {
		g_hash_table_replace(clockDB, file, current_date);
	}

	/* Compute checksum of local file */
	filename = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", file, NULL);
	cksum = checksum_file(filename);
	if( cksum == NULL ) {
		cksum = "0";
	}
	g_free(filename);

	frame = g_strconcat(file,NETWORK_DELIMITER,cksum,NULL);
	wlog(LOGDEBUG,"Send CMD_REQ_FILE :%s",file);
	network_send_command(context, CMD_REQ_FILE, g_utf8_strlen(frame,-1)+1, frame,TRUE);
	g_free(frame);
	if(cksum[0] != '0' && cksum[1] != 0) {
		g_free(cksum);
	}
}
/* Client send its context to server for quick  update */
void network_send_context(context_t * context)
{
	gchar data[BIG_BUF];
	gchar itoa[SMALL_BUF];
	gint  data_size = 0;
	gint  size = 0;
	gchar * selected_id = NULL;
	gchar * selected_map = NULL;

	size = g_utf8_strlen(context->user_name,-1)+1;
	g_memmove(data+data_size, context->user_name, size);
	data_size += size;

	size = g_utf8_strlen(context->character_name,-1)+1;
	g_memmove(data+data_size, context->character_name, size);
	data_size += size;

	size = g_utf8_strlen(context->map,-1)+1;
	g_memmove(data+data_size, context->map, size);
	data_size += size;

	g_snprintf(itoa,sizeof(itoa),"%d",context->connected);
	size = g_utf8_strlen(itoa,-1)+1;
	g_memmove(data+data_size, itoa, size);
	data_size += size;

	g_snprintf(itoa,sizeof(itoa),"%d",context->pos_x);
	size = g_utf8_strlen(itoa,-1)+1;
	g_memmove(data+data_size, itoa, size);
	data_size += size;

	g_snprintf(itoa,sizeof(itoa),"%d",context->pos_y);
	size = g_utf8_strlen(itoa,-1)+1;
	g_memmove(data+data_size, itoa, size);
	data_size += size;

	size = g_utf8_strlen(context->type,-1)+1;
	g_memmove(data+data_size, context->type, size);
	data_size += size;

	g_snprintf(itoa,sizeof(itoa),"%d",context->tile_x);
	size = g_utf8_strlen(itoa,-1)+1;
	g_memmove(data+data_size, itoa, size);
	data_size += size;

	g_snprintf(itoa,sizeof(itoa),"%d",context->tile_y);
	size = g_utf8_strlen(itoa,-1)+1;
	g_memmove(data+data_size, itoa, size);
	data_size += size;

	size = g_utf8_strlen(context->id,-1)+1;
	g_memmove(data+data_size, context->id, size);
	data_size += size;

	if( context->selection.id == NULL ) {
		selected_id = "";
	} else {
		selected_id = context->selection.id;
	}
	size = g_utf8_strlen(selected_id,-1)+1;
	g_memmove(data+data_size, selected_id, size);
	data_size += size;

	g_snprintf(itoa,sizeof(itoa),"%d",context->selection.map_coord[0]);
	size = g_utf8_strlen(itoa,-1)+1;
	g_memmove(data+data_size, itoa, size);
	data_size += size;

	g_snprintf(itoa,sizeof(itoa),"%d",context->selection.map_coord[1]);
	size = g_utf8_strlen(itoa,-1)+1;
	g_memmove(data+data_size, itoa, size);
	data_size += size;

	if( context->selection.map == NULL ) {
		selected_map = "";
	} else {
		selected_map = context->selection.map;
	}
	size = g_utf8_strlen(selected_map,-1)+1;
	g_memmove(data+data_size, selected_map, size);
	data_size += size;

	if( context->selection.inventory == NULL ) {
		selected_id = "";
	} else {
		selected_id = context->selection.inventory;
	}
	size = g_utf8_strlen(selected_id,-1)+1;
	g_memmove(data+data_size, selected_id, size);
	data_size += size;

	if( context->selection.equipment == NULL ) {
		selected_id = "";
	} else {
		selected_id = context->selection.equipment;
	}
	size = g_utf8_strlen(selected_id,-1)+1;
	g_memmove(data+data_size, selected_id, size);
	data_size += size;

	wlog(LOGDEBUG,"Send CMD_SEND_CONTEXT of %s",context->id);
	network_send_command(context, CMD_SEND_CONTEXT, data_size, data,FALSE);
}

/**************************
  read_bytes

  Return FALSE on error, TRUE on OK

 **************************/
gboolean read_bytes(context_t * context, gchar * data, gsize size, gboolean is_data)
{
	GError *err = NULL;
	gboolean res;
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

/*************************
  async_send

*************************/
void async_send(gpointer input_data,gpointer user_data)
{
	GError * err = NULL;
	gboolean res;
	gsize bytes_written;
	GOutputStream * stream;
	context_t * context = (context_t*)user_data;
	send_data_t * data = (send_data_t *)input_data;

	if( data->is_data == FALSE ) {
		stream = context->output_stream;
	} else {
		stream = context->output_data_stream;
	}

	g_mutex_lock(&context->send_mutex);

	//send command
	res = g_output_stream_write_all(stream,&data->command,sizeof(guint32),&bytes_written,NULL,&err);
	if( res == FALSE ) {
		werr(LOGUSER,"Could not send command to %s",context->id);
		goto async_send_end;
	}

	//send additional data size
	guint32 size = data->count; // Force this to uint because gsize size is platform dependant
	res = g_output_stream_write_all(stream,&size,sizeof(guint32),&bytes_written,NULL,&err);
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
	g_mutex_unlock(&context->send_mutex);
	g_free(data->data);
	g_free(data);
}

/*************************
  async_recv

Callback from client listening to server in its own thread
only used for game information transfer
*************************/

gpointer async_recv(gpointer data)
{
	context_t * context = (context_t *)data;

	guint32 command = 0;
	guint32 command_size = 0;
	gchar *buf = NULL;

	do {
		command = 0;
		command_size = 0;
		buf = NULL;

		if( !read_bytes(context,(gchar *)&command, sizeof(guint32),FALSE) ) {
			break;
		}
		/* Read a size */
		if( !read_bytes(context,(gchar *)&command_size, sizeof(guint32),FALSE)) {
			break;
		}

		/* Read additional data */
		if( command_size > 0) {
			buf = g_malloc(command_size);
			g_assert(buf != NULL);
			if( !read_bytes(context,buf, command_size,FALSE) ) {
				break;
			}
		}

		if (!parse_incoming_data(context, command, command_size, buf) ) {
			if( buf ) {
				g_free(buf);
				buf = NULL;
			}
			break;
		}

		if( buf != NULL) {
			g_free(buf);
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

	return NULL;
}

/*************************
  async_data_recv

Callback from client listening to server in its own thread
Only used for data transfers
*************************/

gpointer async_data_recv(gpointer data)
{
	context_t * context = (context_t *)data;

	guint32 command = 0;
	guint32 command_size = 0;
	gchar *buf = NULL;

	do {
		command = 0;
		command_size = 0;
		buf = NULL;

		if( !read_bytes(context,(gchar *)&command, sizeof(guint32),TRUE)) {
			break;
		}
		/* Read a size */
		if( !read_bytes(context,(gchar *)&command_size, sizeof(guint32),TRUE) ) {
			break;
		}

		/* Read additional data */
		if( command_size > 0) {
			buf = g_malloc(command_size);
			g_assert(buf != NULL);
			if( !read_bytes(context,buf, command_size,TRUE) ) {
				break;
			}
		}

		if (!parse_incoming_data(context, command, command_size, buf) ) {
			if( buf ) {
				g_free(buf);
				buf = NULL;
			}
			break;
		}

		if( buf != NULL) {
			g_free(buf);
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

	return NULL;
}

/* Called by client
 return FALSE on error
 return TRUE if OK */
gboolean network_connect(context_t * context, const gchar * hostname)
{
	GSocketClient * client;
	GSocketConnection * connection = NULL;
	GError * error = NULL;

	if( context->hostname ) {
		g_free(context->hostname);
	}
	context->hostname = g_strdup(hostname);

	client = g_socket_client_new();
	connection = g_socket_client_connect_to_host(client,hostname,PORT,NULL,&error);
	if( connection == NULL ) {
		werr(LOGUSER,"Can't connect to server");
		return FALSE;
	}

	context_set_connection(context, connection);
	context_set_input_stream(context,g_io_stream_get_input_stream((GIOStream *)connection));
	context_set_output_stream(context,g_io_stream_get_output_stream((GIOStream *)connection));

	g_mutex_init(&context->send_mutex);
	context->send_thread = g_thread_pool_new(async_send,(gpointer)context,-1,FALSE,NULL);
	listenThread = g_thread_new("recv_thread",async_recv,(gpointer)context);
	g_assert_no_error(error);

	return TRUE;
}

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

	listenThread = g_thread_new("data_connection",async_data_recv,(gpointer)context);
	g_assert_no_error(error);

	return TRUE;
}

/*********************/
/* server functions  */
/*********************/
/**************************
  network_connection

  Callback function created in a new thread by GLib

 **************************/
gboolean network_connection(GThreadedSocketService *service,GSocketConnection *connection, GObject *source_object, gpointer user_data)
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
	context->send_thread = g_thread_pool_new(async_send,(gpointer)context,-1,FALSE,NULL);

	context_new_VM(context);

	context_set_connected(context,TRUE);

	while(context_get_connected(context)) {
		guint32 command = 0;
		guint32 command_size = 0;
		gchar *buf = NULL;
		/* Read a command code */
		if( !read_bytes(context,(gchar *)&command, sizeof(guint32),FALSE)) {
			context_set_connected(context,FALSE);
			break;
		}
		/* Read a size */
		if( !read_bytes(context,(gchar *)&command_size, sizeof(guint32),FALSE)) {
			context_set_connected(context,FALSE);
			break;
		}

		/* Read additional data */
		if( command_size > 0) {
			buf = g_malloc(command_size);
			g_assert(buf != NULL);
			if( !read_bytes(context,buf, command_size,FALSE)) {
				context_set_connected(context,FALSE);
				break;
			}
		}

		if (!parse_incoming_data(context, command, command_size, buf) ) {
			if( buf ) {
				g_free(buf);
				buf = NULL;
			}
			context_set_connected(context,FALSE);
			break;
		}

		if( buf != NULL) {
			g_free(buf);
			buf = NULL;
		}
	}
	wlog(LOGUSER,"Client disconnected");
	context_spread(context);
	context_write_to_file(context);
	context_free(context);

	return TRUE;
}

/**************************
  network_init

  Need to be run before g_main_loop_run

 **************************/

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

/**************************
  network_send_context

  send the data of a context to another context

 **************************/
void network_send_context_to_context(context_t * dest_ctx, context_t * src_ctx)
{
	gchar data[BIG_BUF];
	gchar itoa[SMALL_BUF];
	gint  data_size = 0;
	gint  size = 0;

	size = g_utf8_strlen(src_ctx->user_name,-1)+1;
	g_memmove(data+data_size, src_ctx->user_name, size);
	data_size += size;

	size = g_utf8_strlen(src_ctx->character_name,-1)+1;
	g_memmove(data+data_size, src_ctx->character_name, size);
	data_size += size;

	size = g_utf8_strlen(src_ctx->map,-1)+1;
	g_memmove(data+data_size, src_ctx->map, size);
	data_size += size;

	g_snprintf(itoa,sizeof(itoa),"%d",src_ctx->connected);
	size = g_utf8_strlen(itoa,-1)+1;
	g_memmove(data+data_size, itoa, size);
	data_size += size;

	g_snprintf(itoa,sizeof(itoa),"%d",src_ctx->pos_x);
	size = g_utf8_strlen(itoa,-1)+1;
	g_memmove(data+data_size, itoa, size);
	data_size += size;

	g_snprintf(itoa,sizeof(itoa),"%d",src_ctx->pos_y);
	size = g_utf8_strlen(itoa,-1)+1;
	g_memmove(data+data_size, itoa, size);
	data_size += size;

	size = g_utf8_strlen(src_ctx->type,-1)+1;
	g_memmove(data+data_size, src_ctx->type, size);
	data_size += size;

	g_snprintf(itoa,sizeof(itoa),"%d",src_ctx->tile_x);
	size = g_utf8_strlen(itoa,-1)+1;
	g_memmove(data+data_size, itoa, size);
	data_size += size;

	g_snprintf(itoa,sizeof(itoa),"%d",src_ctx->tile_y);
	size = g_utf8_strlen(itoa,-1)+1;
	g_memmove(data+data_size, itoa, size);
	data_size += size;

	size = g_utf8_strlen(src_ctx->id,-1)+1;
	g_memmove(data+data_size, src_ctx->id, size);
	data_size += size;

	wlog(LOGDEBUG,"Send CMD_SEND_CONTEXT of %s to %s",src_ctx->id,dest_ctx->id);
	network_send_command(dest_ctx, CMD_SEND_CONTEXT, data_size, data,FALSE);

}

/**************************
  network_send_file

 filename is relative to the data dir

  send a file to a context
return 0 on success
 **************************/
int network_send_file(context_t * context, gchar * filename)
{
	/* Check if this context is connected */
	GOutputStream * stream = context_get_output_stream(context);
	if( stream == NULL ) {
		return 1;
	}

	/* Never send files with password */
	if ( g_strstr_len(PASSWD_TABLE,-1,filename) != NULL ) {
		werr(LOGUSER,"send_file : Do not serve hazardous file  \"%s\"",filename);

		return 1;
	}

	/* Read the file */
	gchar * full_name = NULL;
	full_name = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", filename, NULL);

	gchar * file_data = NULL;
	gsize file_length = 0;

	SDL_LockMutex(file_mutex);
	gboolean res = file_get_contents(full_name,&file_data,&file_length,NULL);
	SDL_UnlockMutex(file_mutex);
	if( res == FALSE) {
		werr(LOGUSER,"send_file : Error reading file \"%s\"",full_name);
		g_free(full_name);
		return 1;
	}

	g_free(full_name);

	/* Prepare the frame = file_name_size + file_name + file_data_size + file_data*/
	guint32 count = sizeof(guint32)+g_utf8_strlen(filename,-1)+1+sizeof(guint32)+file_length;
	gchar * frame = g_malloc0(count);
	if( frame == NULL) {
		g_free(file_data);
		werr(LOGUSER,"send_file : Error allocating memory");
		return 1;
	}

	/* first the filename size */
	*((guint32 *)frame) = g_utf8_strlen(filename,-1)+1;
	gchar * ptr = frame;
	ptr += sizeof(guint32);

	/* then the name of the file itself */
	g_memmove(ptr,filename,g_utf8_strlen(filename,-1)+1);
	ptr += g_utf8_strlen(filename,-1) + 1;

	/* then the size of the file data */
	*((guint32 *)ptr) = file_length;
	ptr += sizeof(guint32);

	/* then the file data itself */
	g_memmove(ptr,file_data,file_length);

	g_free(file_data);

	/* send the frame */
	wlog(LOGDEBUG,"Send CMD_SEND_FILE : %s",filename);
	network_send_command(context, CMD_SEND_FILE, count, frame,FALSE);

	g_free(frame);

	return 0;
}
