/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013 carabobz@gmail.com

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
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include "common.h"
#include <string.h>
#include <libconfig.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define AUTO_SAVE_TIMEOUT 30

GStaticMutex file_mutex = G_STATIC_MUTEX_INIT;
static GHashTable* updateDB = NULL; /* To remember which file we have never tried to read before */
typedef struct file_data {
	gchar * data;
	gsize size;
	gboolean file_update;
	config_t * config;
	gboolean config_update;
} file_data_t;
static GHashTable* fileDB = NULL; /* table to store files data */

static gint found = 0;

static void config_print_error(char * file, const config_t * config)
{
	werr(LOGUSER,"libconfig error %s@%d : %s\n",
		 file,
		 config_error_line(config),
		 config_error_text(config));
}

static void free_key(gpointer ptr)
{
	g_free(ptr);
}

static void free_file(file_data_t * data)
{
	if( data->data ) {
		g_free(data->data);
		data->data = NULL;
		data->size = 0;
		data->file_update = FALSE;
	}
}
static void free_config(file_data_t * data)
{
	if( data->config ) {
		config_destroy (data->config);
		g_free(data->config);
		data->config = NULL;
		data->config_update = FALSE;
	}
}
static void free_fileDB_value(gpointer ptr)
{
	file_data_t * data = (file_data_t *)ptr;

	free_file(data);

	free_config(data);

	g_free(data);
}

static void free_updateDB_value(gpointer data)
{
	g_free(data);
}

/****************************
file_dump_to_disk
****************************/
static void dump_file(gchar * filename, file_data_t * data)
{
	if( data->file_update) {
		wlog(LOGDEBUG,"Writing file %s",filename);
		free_config(data);
		g_file_set_contents(filename,data->data,data->size,NULL);
		data->file_update = FALSE;

		free_config(data);
	}
}
static void dump_config(gchar * filename, file_data_t * data)
{
	if( data->config_update) {
		wlog(LOGDEBUG,"Writing config %s",filename);
		config_write_file(data->config,filename);
		data->config_update = FALSE;

		free_file(data);
	}
}
static void file_dump_to_disk(gpointer key,gpointer value,gpointer user_data)
{
	gchar * filename = (gchar *) key;
	file_data_t * data = (file_data_t *)value;

	dump_file(filename,data);

	dump_config(filename,data);
}

/****************************
file_dump_all_to_disk
****************************/
void file_dump_all_to_disk(void)
{
	wlog(LOGDEV,"Dumping all files to disk");
	g_static_mutex_lock(&file_mutex);
	if( fileDB ) {
		g_hash_table_foreach(fileDB,file_dump_to_disk,NULL);
	}
	g_static_mutex_unlock(&file_mutex);
}
/**********************************
auto_save_files
*********************************/
static gpointer auto_save_files(gpointer data)
{
	while(1) {
		g_usleep(AUTO_SAVE_TIMEOUT * 1000 * 1000);
		file_dump_all_to_disk();
	}

	return NULL;
}

/****************************
Parameter 1: Name of the table to create the new file
return the name of an available empty file
if success the file is created on disk
the return string must be freed by caller
****************************/
gchar * file_new(gchar * table)
{
	GDir * dir;
	const gchar * name;
	gchar * dirname;
	gchar * filename;
	gchar tag[7];
	gint index = 0;
	GFile * file;
	GFileOutputStream * file_stream;

	g_static_mutex_lock(&file_mutex);
	g_sprintf(tag,"A%05x",index);

	dirname = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", table,  NULL);
	dir = g_dir_open(dirname,0,NULL);

	while(( name = g_dir_read_name(dir)) != NULL ) {
		if( g_strcmp0(name,".") == 0 ) {
			continue;
		}
		if( g_strcmp0(name,"..") == 0 ) {
			continue;
		}
		if( g_strcmp0(name,tag) == 0 ) {
			index++;
			g_sprintf(tag,"A%05x",index);
			g_dir_rewind (dir);
			continue;
		}
	}

	filename = g_strconcat(dirname,"/",tag,NULL);
	file = g_file_new_for_path(filename);
	file_stream = g_file_create(file,G_FILE_CREATE_NONE,NULL,NULL);

	g_static_mutex_unlock(&file_mutex);

	g_object_unref(file_stream);
	g_object_unref(file);

	g_free(filename);
	g_free(dirname);

	return g_strdup(tag);
}

/****************************
file_get_contents
Warning : return binary data do not forget to add a terminal 0 if you know its a string using "turn_data_into_string" function
****************************/
gboolean file_get_contents(const gchar *filename,gchar **contents,gsize *length,GError **error)
{
	file_data_t * old_data;
	file_data_t * new_data;
	gboolean ret;

	if( fileDB == NULL ) {
		fileDB = g_hash_table_new_full(g_str_hash,g_str_equal, free_key, free_fileDB_value);
		if( client_server == SERVER) {
			g_thread_create(auto_save_files,NULL,FALSE,NULL);
		}
	}

	old_data = g_hash_table_lookup(fileDB,filename);

	if( old_data == NULL ) {
		/*First time we try to acces this file, read it from the disk */
		ret = g_file_get_contents(filename,contents,length,error);
		if( ret == FALSE ) {
			return FALSE;
		}

		new_data = g_malloc0(sizeof(file_data_t));
		new_data->data = g_malloc0(*length);
		g_memmove(new_data->data,*contents,*length);
		new_data->size = *length;
		g_hash_table_replace(fileDB,g_strdup(filename),new_data);
		return TRUE;
	} else {
		/* file modified by libconfig: dump modif to disk and
		read them back */
		if( old_data->config_update == TRUE ) {
			file_dump_to_disk((gpointer)filename,old_data,NULL);

			g_free(old_data->data);
			ret = g_file_get_contents(filename,&old_data->data,
									  &old_data->size,error);
			if( ret == FALSE ) {
				return FALSE;
			}
		} else {
			/* The file has been accessed by libconfig but
			 not yet directly, load it */
			if(old_data->data == NULL) {
				ret = g_file_get_contents(filename,
										  contents,
										  length,
										  error);
				if( ret == FALSE ) {
					return FALSE;
				}

				old_data->data = g_malloc0(*length);
				g_memmove(old_data->data,*contents,*length);
				old_data->size = *length;
				return TRUE;
			}
		}
	}

	*contents = g_malloc0(old_data->size);
	g_memmove(*contents,old_data->data,old_data->size);
	*length = old_data->size;

	return TRUE;
}
/****************************
file_set_contents
****************************/
gboolean file_set_contents(const gchar *filename,const gchar *contents,gssize length,GError **error)
{
	file_data_t * new_data;
	gchar * image_name;

	if( fileDB == NULL ) {
		fileDB = g_hash_table_new_full(g_str_hash,g_str_equal, free_key, free_fileDB_value);
		if( client_server == SERVER) {
			g_thread_create(auto_save_files,NULL,FALSE,NULL);
		}
	}

	new_data = g_malloc0(sizeof(file_data_t));
	new_data->data = g_malloc0(length);
	g_memmove(new_data->data,contents,length);
	new_data->size = length;
	new_data->file_update = TRUE;
	g_hash_table_replace(fileDB,g_strdup(filename),new_data);

	if(client_server == CLIENT) {
		/* Image widget are created from files on the disk so we have to
		   write images to disk */
		image_name = g_strconcat( g_getenv("HOME"),"/", base_directory, "/",IMAGE_TABLE,NULL);
		if( g_ascii_strncasecmp(image_name,filename,strlen(image_name)) == 0) {
			file_dump_to_disk((gpointer)filename,new_data,NULL);
		}
		g_free(image_name);
	}

	return TRUE;
}

/*********************/
/*********************/
/*********************/

static file_data_t * create_config(gchar * file)
{
	int ret;

	file_data_t * config = NULL;

	config = g_malloc0(sizeof(file_data_t));
	if(config == NULL) {
		return NULL;
	}
	config->config = g_malloc0(sizeof(config_t));
	if(config->config == NULL) {
		return NULL;
	}
	config->config_update = FALSE;

	ret = config_read_file (config->config, file);
	if( ret == CONFIG_FALSE ) {
		/* if file doesn't exist we do not log an error since
		config_read_file should have made a request for this file
		to the server.*/
		if(open(file,O_RDONLY) != -1) {
			config_print_error(file,config->config);
		}
		config_destroy(config->config);
		g_free(config->config);
		g_free(config);
		return NULL;
	}

	return config;
}

static void init_updateDB(const gchar * table, const gchar * file)
{
	gint * found_ptr;
	gchar * filename;

	if( updateDB == NULL ) {
		updateDB = g_hash_table_new_full(g_str_hash,g_str_equal, free_key, free_updateDB_value);
	}

	filename = g_strconcat( table, "/", file, NULL);
	found_ptr = g_hash_table_lookup(updateDB,filename);
	if( found_ptr == NULL ) {
		/* First time we try to acces this file */
		/* Mark it as requested for full update */
		g_hash_table_replace(updateDB,g_strdup(filename),&found);
		network_send_req_file(context_get_list_first(),filename);
	}
	g_free(filename);

}

static file_data_t * get_config(const gchar * table, const gchar * file)
{
	gchar * filename ;
	file_data_t * config;
	int ret;

	if( client_server == CLIENT ) {
		init_updateDB(table,file);
	}

	filename = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", table, "/", file, NULL);

	if( fileDB == NULL ) {
		fileDB = g_hash_table_new_full(g_str_hash,g_str_equal, free_key, free_fileDB_value);
		if( client_server == SERVER) {
			g_thread_create(auto_save_files,NULL,FALSE,NULL);
		}
	}

	config = g_hash_table_lookup(fileDB,filename);

	if( config == NULL ) {
		/*First time we try to acces this file */
		if( (config=create_config(filename)) == NULL ) {
			g_free(filename);
			return NULL;
		}
		g_hash_table_replace(fileDB,g_strdup(filename),config);
	} else {
		/* Write cached data if any */
		dump_file(filename,config);
		/* Recreate a config if needed */
		if(config->config == NULL) {
			config->config = g_malloc0(sizeof(config_t));
			memset(config->config,0,sizeof(config_t));
			if(config->config == NULL) {
				return NULL;
			}
			config->config_update = FALSE;

			ret = config_read_file (config->config, filename);
			if( ret == CONFIG_FALSE ) {
				config_print_error(filename,config->config);
				config_destroy(config->config);
				g_free(config->config);
				return NULL;
			}
		}
	}

	g_free(filename);

	return config;
}

static gchar * add_entry_to_path(gchar * path, gchar * entry)
{
	gchar * new_path;

	if( path == NULL ) {
		path = g_strconcat(entry,NULL);
	} else {
		new_path=g_strconcat(path,".",entry,NULL);
		g_free(path);
		path = new_path;
	}

	return path;
}
/* Create a libconfig path from a list of string */
/* the return path must be freed */
static gchar * get_path(va_list ap)
{
	char * path=NULL;
	gchar * entry = NULL;

	entry=va_arg(ap,gchar*);
	if( entry == NULL ) {
		return NULL;
	}

	while( entry != NULL ) {
		path = add_entry_to_path(path,entry);
		if(path == NULL) {
			return NULL;
		}
		entry=va_arg(ap,gchar*);
	}

	return path;
}

/*********************/
/*********************/
/*********************/
/* return FALSE on error */
static int __read_int(const gchar * table, const gchar * file, int * res, va_list ap)
{
	const file_data_t * config = NULL;
	char * path=NULL;
	long int result;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	path = get_path(ap);
	if(path == NULL) {
		return FALSE;
	}

	if(config_lookup_int (config->config, path, &result)==CONFIG_FALSE) {
//		g_warning("%s: Can't read %s/%s/%s",__func__,table,file,path);
		g_free(path);
		return FALSE;
	}
	g_free(path);

	*res = (int)result;

	return TRUE;
}
int read_int(const gchar * table, const gchar * file, int * res, ...)
{
	int ret;
	va_list ap;

	g_static_mutex_lock(&file_mutex);
	va_start(ap, res);
	ret = __read_int(table, file, res, ap);
	va_end(ap);
	g_static_mutex_unlock(&file_mutex);

	return ret;
}

/* Not used yet
static int _read_int(const gchar * table, const gchar * file, int * res, ...)
{
	int ret;
	va_list ap;

        va_start(ap, res);
	ret = __read_int(table, file, res, ap);
        va_end(ap);

	return ret;
}
*/
/*********************/
/*********************/
/*********************/
/* return FALSE on error */
static int __read_string(const gchar * table, const gchar * file, const gchar ** res, va_list ap)
{
	const file_data_t * config = NULL;
	gchar * path;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	path = get_path(ap);
	if(path == NULL) {
		return FALSE;
	}

	if(config_lookup_string (config->config, path, res)==CONFIG_FALSE) {
//		g_warning("%s: Can't read %s/%s/%s",__func__,table,file,path);
		g_free(path);
		return FALSE;
	}
	g_free(path);

	return TRUE;
}
int read_string(const gchar * table, const gchar * file, const gchar ** res, ...)
{
	int ret;
	va_list ap;

	g_static_mutex_lock(&file_mutex);
	va_start(ap, res);
	ret = __read_string(table, file, res, ap);
	va_end(ap);
	g_static_mutex_unlock(&file_mutex);

	return ret;
}

/* Not used yet
static int _read_string(const gchar * table, const gchar * file, const gchar ** res, ...)
{
	int ret;
	va_list ap;

        va_start(ap, res);
	ret = __read_string(table, file, res, ap);
        va_end(ap);

	return ret;
}
*/
/*********************/
/*********************/
/*********************/
/* return FALSE on error */
static int __read_list_index(const gchar * table, const gchar * file, const gchar ** res, gint index, va_list ap)
{
	const file_data_t * config = NULL;
	config_setting_t * setting = NULL;
	gchar * path;

	*res = NULL;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	path = get_path(ap);
	if(path == NULL) {
		return FALSE;
	}

	setting = config_lookup (config->config, path);
	if( setting == NULL ) {
//		g_warning("%s: Can't read %s/%s/%s",__func__,table,file,path);
		g_free(path);
		return FALSE;
	}
	g_free(path);

	*res = config_setting_get_string_elem (setting,index);
	if(*res == NULL ) {
		return FALSE;
	}
	return TRUE;
}
int read_list_index(const gchar * table, const gchar * file, const gchar ** res,gint index, ...)
{
	int ret;
	va_list ap;

	g_static_mutex_lock(&file_mutex);
	va_start(ap, index);
	ret = __read_list_index(table,file, res,index, ap);
	va_end(ap);
	g_static_mutex_unlock(&file_mutex);

	return ret;
}
/* Not used yet
static int _read_list_index(const gchar * table, const gchar * file, const gchar ** res,gint index, ...)
{
	int ret;
	va_list ap;

        va_start(ap, index);
	ret = __read_list_index(table,file, res,index, ap);
	va_end(ap);

	return ret;
}
*/
/*********************/
/*********************/
/*********************/
/* return FALSE on error */
static int __read_list(const gchar * table, const gchar * file, gchar *** res, va_list ap)
{
	const file_data_t * config = NULL;
	config_setting_t * setting = NULL;
	gchar * path;
	int i = 0;
	const char * elem;

	*res = NULL;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	path = get_path(ap);
	if(path == NULL) {
		return FALSE;
	}

	setting = config_lookup (config->config, path);
	if( setting == NULL ) {
		g_free(path);
		return FALSE;
	}
	g_free(path);

	while( (elem=config_setting_get_string_elem (setting,i )) != NULL ) {
		i++;
		*res = g_realloc(*res,(i+1)*sizeof(gchar *));
		(*res)[i-1] = (gchar *)elem;
		(*res)[i] = NULL;

	}

	if(*res == NULL) {
		return FALSE;
	}

	return TRUE;
}
int read_list(const gchar * table, const gchar * file, gchar *** res, ...)
{
	int ret;
	va_list ap;

	g_static_mutex_lock(&file_mutex);
	va_start(ap, res);
	ret = __read_list(table, file, res, ap);
	va_end(ap);
	g_static_mutex_unlock(&file_mutex);

	return ret;
}

/* Not used yet
static int _read_list(const gchar * table, const gchar * file, gchar *** res, ...)
{
	int ret;
	va_list ap;

        va_start(ap, res);
	ret = __read_list(table, file, res, ap);
        va_end(ap);

	return ret;
}
*/
/*********************/
/*********************/
/*********************/
static config_setting_t * create_tree(const config_t * config, config_setting_t * prev_setting, gchar * prev_entry, gchar * path, int type, va_list ap)
{
	gchar * entry = NULL;
	gchar * new_path;
	config_setting_t * setting = NULL;

	if( prev_setting == NULL) {
		prev_setting = config_root_setting(config);
	}

	entry = va_arg(ap,gchar*);

	/* no more leaf */
	if(entry == NULL) {
		/* check if leaf exists */
		setting = config_lookup(config, path);
		if( setting == NULL) {
			/* create the leaf */
			setting=config_setting_add(prev_setting, prev_entry,type);
		}
		g_free(path);
		return setting;
	}

	/* still not finished */
	/* check if we need to create a group */
	if( path ) {
		setting = config_lookup(config, path);
		if( setting == NULL ) {
			/* create a group with previous path */
			setting = config_setting_add(prev_setting, prev_entry, CONFIG_TYPE_GROUP);
			if(setting == NULL) {
				g_free(path);
				return NULL;
			}
		}
	}

	/* update path */
	if( path ) {
		new_path = g_strconcat(path,".",entry,NULL);
		g_free(path);
		path = new_path;
	} else {
		path = g_strconcat(entry,NULL);
	}

	return create_tree(config,setting,entry,path,type,ap);
}

/*********************/
/*********************/
/*********************/
/* Returns :

TRUE on success, FALSE if an error occurred
*/
static int __write_int(const gchar * table, const gchar * file, int data, va_list ap)
{
	config_setting_t * setting;
	file_data_t * config;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	setting = create_tree(config->config,NULL,NULL,NULL,CONFIG_TYPE_INT,ap);

	/* update int */
	if(config_setting_set_int(setting, data)==CONFIG_FALSE) {
		return FALSE;
	}

	config->config_update = TRUE;
	return TRUE;
}
int write_int(const gchar * table, const gchar * file,int data, ...)
{
	int ret;
	va_list ap;

	g_static_mutex_lock(&file_mutex);
	va_start(ap, data);
	ret = __write_int(table, file, data, ap);
	va_end(ap);
	g_static_mutex_unlock(&file_mutex);

	return ret;
}

/* Not used yet
static int _write_int(const gchar * table, const gchar * file,int data, ...)
{
	int ret;
	va_list ap;

        va_start(ap, data);
	ret = __write_int(table, file, data, ap);
        va_end(ap);

	return ret;
}
*/
/*********************/
/*********************/
/*********************/
/* Returns :

TRUE on success, FALSE if an error occurred
*/
static int __write_string(const gchar * table, const gchar * file, const char * data, va_list ap)
{
	config_setting_t * setting;
	file_data_t * config;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	setting = create_tree(config->config,NULL,NULL,NULL,CONFIG_TYPE_STRING,ap);

	/* update string */
	if(config_setting_set_string(setting, data)==CONFIG_FALSE) {
		return FALSE;
	}
	config->config_update = TRUE;

	return TRUE;
}
int write_string(const gchar * table, const gchar * file, const char * data, ...)
{
	int ret;
	va_list ap;

	g_static_mutex_lock(&file_mutex);
	va_start(ap, data);
	ret = __write_string(table, file, data, ap);
	va_end(ap);
	g_static_mutex_unlock(&file_mutex);

	return ret;
}

/* Not used yet
static int _write_string(const gchar * table, const gchar * file, const char * data, ...)
{
	int ret;
	va_list ap;

        va_start(ap, data);
	ret = __write_string(table, file, data, ap);
        va_end(ap);

	return ret;
}
*/
/*********************/
/*********************/
/*********************/
/* Returns :

TRUE on success, FALSE if an error occurred
*/
static int __write_list_index(const gchar * table, const gchar * file, const char * data, gint index, va_list ap)
{
	config_setting_t * setting = NULL;
	file_data_t * config;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	setting = create_tree(config->config,NULL,NULL,NULL,CONFIG_TYPE_ARRAY,ap);

	if(config_setting_set_string_elem(setting,index,data)==NULL) {
		return FALSE;
	}

	config->config_update = TRUE;

	return TRUE;
}
int write_list_index(const gchar * table, const gchar * file, const char * data, gint index, ...)
{
	int ret;
	va_list ap;

	g_static_mutex_lock(&file_mutex);
	va_start(ap, index);
	ret = __write_list_index(table, file, data,index,ap);
	va_end(ap);
	g_static_mutex_unlock(&file_mutex);

	return ret;
}
/* Not used yet
static int _write_list_index(const gchar * table, const gchar * file, const char * data, gint index, ...)
{
	int ret;
	va_list ap;

        va_start(ap, index);
	ret = __write_list_index(table, file, data,index,ap);
        va_end(ap);

	return ret;
}
*/
/*********************/
/*********************/
/*********************/
/* Returns :

TRUE on success, FALSE if an error occurred
*/
static int __write_list(const gchar * table, const gchar * file, char ** data, va_list ap)
{
	config_setting_t * setting;
	file_data_t * config;
	int i=0;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	setting = create_tree(config->config,NULL,NULL,NULL,CONFIG_TYPE_ARRAY,ap);

	while(data[i] != NULL) {
		if(config_setting_set_string_elem(setting,-1,data[i])==NULL) {
			return FALSE;
		}
		i++;
	}

	config->config_update = TRUE;

	return TRUE;
}
int write_list(const gchar * table, const gchar * file, char ** data, ...)
{
	int ret;
	va_list ap;

	g_static_mutex_lock(&file_mutex);
	va_start(ap, data);
	ret = __write_list(table, file, data, ap);
	va_end(ap);
	g_static_mutex_unlock(&file_mutex);

	return ret;
}
/* Not used yet
static int _write_list(const gchar * table, const gchar * file, char ** data, ...)
{
	int ret;
	va_list ap;

        va_start(ap, data);
	ret = __write_list(table, file, data, ap);
        va_end(ap);

	return ret;
}
*/
/*********************/
/*********************/
/* return 0 on failure */
/*********************/
static gboolean __add_to_list(const gchar * table, const gchar * file, const gchar * to_be_added, va_list ap)
{
	file_data_t * config = NULL;
	config_setting_t * setting = NULL;
	gchar * path;

	config = get_config(table,file);
	if(config==NULL) {
		return 0;
	}

	path = get_path(ap);
	if(path == NULL) {
		return FALSE;
	}

	setting = config_lookup (config->config, path);
	if( setting == NULL ) {
		g_free(path);
		return 0;
	}
	g_free(path);

	if(config_setting_set_string_elem(setting,-1,to_be_added)==NULL) {
		return 0;
	}

	config->config_update = TRUE;
	return 1;
}
gboolean add_to_list(const gchar * table, const gchar * file, const gchar * to_be_added, ...)
{
	gboolean ret;
	va_list ap;

	g_static_mutex_lock(&file_mutex);
	va_start(ap, to_be_added);
	ret = __add_to_list(table,file,to_be_added,ap);
	va_end(ap);
	g_static_mutex_unlock(&file_mutex);

	return ret;
}

/* Not used yet
static gboolean _add_to_list(const gchar * table, const gchar * file, const gchar * to_be_added, ...)
{
	gboolean ret;
	va_list ap;

        va_start(ap, to_be_added);
	ret = __add_to_list(table,file,to_be_added,ap);
        va_end(ap);

	return ret;
}
*/
/*********************/
/*********************/
/*********************/
/*********************/

static gboolean __remove_group(const gchar * table, const gchar * file, const gchar * group, va_list ap)
{

	file_data_t * config;
	config_setting_t * setting = NULL;
	gchar * path;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	path = get_path(ap);
	if(path != NULL) {
		setting = config_lookup(config->config,path);
		g_free(path);
	} else {
		setting = config_root_setting(config->config);
	}

	if( setting == NULL ) {
		return FALSE;
	}

	if( config_setting_remove(setting,group) == CONFIG_FALSE) {
		return FALSE;
	}

	config->config_update = TRUE;
	return TRUE;
}

/*********************/
/*********************/
/*********************/
/* Returns :

TRUE on success, FALSE if an error occurred
*/
int remove_group(const gchar * table, const gchar * file, const gchar * group, ...)
{
	int ret;
	va_list ap;

	g_static_mutex_lock(&file_mutex);
	va_start(ap, group);
	ret = __remove_group(table, file, group, ap);
	va_end(ap);
	g_static_mutex_unlock(&file_mutex);

	return ret;
}
/**********************
get_unused_list_entry
find an unused tag in a list, add it to the list and return it.
returned string must be freed
**********************/
gchar * get_unused_list_entry(const gchar * table, const gchar * file, ...)
{
	gchar **  list;
	gchar tag[7];
	gint index = 0;
	gint i = 0;
	va_list ap;

	g_static_mutex_lock(&file_mutex);
	g_sprintf(tag,"A%05x",index);

	va_start(ap,file);
	if(__read_list(table,file,&list,ap)) {
		while(list[i] != NULL ) {
			if( g_strcmp0(list[i],tag) == 0 ) {
				index++;
				g_sprintf(tag,"A%05x",index);
				i=0;
			}
			i++;
		}
	}
	va_end(ap);

	va_start(ap,file);
	if( !__add_to_list(table,file,tag,ap)) {
		va_end(ap);
		return NULL;
	}
	va_end(ap);
	g_static_mutex_unlock(&file_mutex);

	return g_strdup(tag);
}

/**********************
get_unused_group
find an unused group, create it and return its name
returned string must be freed
**********************/
gchar * __get_unused_group(const gchar * table, const gchar * file,  va_list ap)
{
	gchar tag[7];
	gint index = 0;
	gchar * path;
	config_setting_t * setting;
	const file_data_t * config;

	config = get_config(table,file);
	if(config==NULL) {
		return NULL;
	}

	g_sprintf(tag,"A%05x",index);

	path = get_path(ap);

	if(path != NULL) {
		setting = config_lookup(config->config,path);
		g_free(path);
	} else {
		setting = config_root_setting(config->config);
	}

	while( config_setting_add(setting,tag,CONFIG_TYPE_GROUP) == NULL ) {
		index++;
		g_sprintf(tag,"A%05x",index);
	}

	return g_strdup(tag);
}

gchar * get_unused_group(const gchar * table, const gchar * file, ...)
{
	gchar * ret;
	va_list ap;

	g_static_mutex_lock(&file_mutex);
	va_start(ap, file);
	ret = __get_unused_group(table, file, ap);
	va_end(ap);
	g_static_mutex_unlock(&file_mutex);

	return ret;
}

/**********************
__get_unused_group_on_path
find an unused group, DO NOT CREATE IT and return its name
returned string must be freed
**********************/
gchar * __get_unused_group_on_path(const gchar * table, const gchar * file, gchar * path)
{
	gchar tag[7];
	gint index = 0;
	config_setting_t * setting;
	const file_data_t * config;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	g_sprintf(tag,"A%05x",index);

	if(path != NULL) {
		setting = config_lookup(config->config,path);
		g_free(path);
	} else {
		setting = config_root_setting(config->config);
	}

	while( config_setting_add(setting,tag,CONFIG_TYPE_GROUP) == NULL ) {
		index++;
		g_sprintf(tag,"A%05x",index);
	}

	config_setting_remove(setting,tag);

	return g_strdup(tag);
}

/**********************
get_group_list
Return a list of the name of the elements in the specified group
returned list must be freed  (g_free)
 return FALSE on error
**********************/
int get_group_list(const gchar * table, const gchar * file, gchar *** res, ...)
{
	gchar * path;
	const file_data_t * config;
	va_list ap;
	config_setting_t * setting;
	config_setting_t * elem_setting;
	gint index = 0;

	*res = NULL;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	g_static_mutex_lock(&file_mutex);

	va_start(ap,res);
	path = get_path(ap);
	va_end(ap);

	if(path != NULL && path[0] != 0) {
		setting = config_lookup(config->config,path);
		g_free(path);
	} else {
		setting = config_root_setting(config->config);
	}

	/* The path does not exist in the conf file */
	if(setting == NULL ) {
		g_static_mutex_unlock(&file_mutex);
		return FALSE;
	}

	while((elem_setting=config_setting_get_elem(setting,index))!= NULL ) {
		index++;
		*res = g_realloc(*res,(index+1)*sizeof(gchar *));
		(*res)[index-1] = (gchar *)config_setting_name(elem_setting);
		(*res)[index] = NULL;
	}

	g_static_mutex_unlock(&file_mutex);

	if( *res == NULL) {
		return FALSE;
	}

	return TRUE;
}
/*********************/
/*********************/
/* return 0 on failure */
/*********************/

static gboolean __remove_from_list(const gchar * table, const gchar * file, const gchar * to_be_removed, va_list ap)
{
	file_data_t * config = NULL;
	config_setting_t * setting = NULL;
	gchar * path;
	int i = 0;
	const char * elem;

	config = get_config(table,file);
	if(config==NULL) {
		return 0;
	}

	path = get_path(ap);
	if(path == NULL) {
		return FALSE;
	}

	setting = config_lookup (config->config, path);
	if( setting == NULL ) {
		g_free(path);
		return 0;
	}
	g_free(path);

	while( (elem=config_setting_get_string_elem (setting,i )) != NULL ) {
		if( g_strcmp0(elem,to_be_removed) == 0 ) {
			if(config_setting_remove_elem (setting,i)==CONFIG_FALSE) {
				return 0;
			}
			config->config_update = TRUE;
			return 1;
		}
		i++;
	}

	return 0;
}
gboolean remove_from_list(const gchar * table, const gchar * file, const gchar * to_be_removed, ...)
{
	gboolean ret;
	va_list ap;

	g_static_mutex_lock(&file_mutex);
	va_start(ap, to_be_removed);
	ret = __remove_from_list(table,file,to_be_removed,ap);
	va_end(ap);
	g_static_mutex_unlock(&file_mutex);
	return ret;
}

/* Not used yet
static gboolean _remove_from_list(const gchar * table, const gchar * file, const gchar * to_be_removed, ...) {
	gboolean ret;
	va_list ap;

        va_start(ap, to_be_removed);
	ret = __remove_from_list(table,file,to_be_removed,ap);
	va_end(ap);
	return ret;
}
*/
gboolean copy_config(config_setting_t * source,config_setting_t * destination);
gboolean copy_aggregate(config_setting_t * source, config_setting_t * dest, int type)
{
	const char * setting_name;
	config_setting_t * new_dest;
	gint index=0;
	gchar tag[7];

	setting_name = config_setting_name(source);
	if(setting_name == NULL) {
		return 0;
	}

	new_dest=config_setting_add(dest,setting_name,type);
	if( new_dest == NULL ) {
		/* Try to find an available name */
		g_sprintf(tag,"A%05x",index);

		while((new_dest=config_setting_add(dest,setting_name,type))== NULL) {
			index++;
			g_sprintf(tag,"A%05x",index);
		}
	}

	if(!copy_config(source,new_dest)) {
		config_setting_remove(dest,setting_name);
		return 0;
	}
	return 1;
}
gboolean copy_config(config_setting_t * source, config_setting_t * dest)
{
	config_setting_t * new_source;
	config_setting_t * new_dest;
	gint index = -1;
	int int_value;
	long long long_value;
	double double_value;
	const char * string;

	while((new_source=config_setting_get_elem(source,index+1))!= NULL ) {
		index++;
		if(config_setting_is_group(new_source)) {
			if(!copy_aggregate(new_source,dest,CONFIG_TYPE_GROUP)) {
				return 0;
			}
		}

		else if(config_setting_is_array(new_source)) {
			if(!copy_aggregate(new_source,dest,CONFIG_TYPE_ARRAY)) {
				return 0;
			}
		}

		else if(config_setting_is_list(new_source)) {
			if(!copy_aggregate(new_source,dest,CONFIG_TYPE_LIST)) {
				return 0;
			}
		}

		else {
			switch(config_setting_type(new_source)) {
			case CONFIG_TYPE_INT:
				int_value = config_setting_get_int(new_source);
				new_dest = config_setting_add (dest, config_setting_name(new_source),CONFIG_TYPE_INT);
				config_setting_set_int(new_dest,int_value);
				continue;
			case CONFIG_TYPE_INT64:
				long_value = config_setting_get_int64(new_source);
				new_dest = config_setting_add (dest, config_setting_name(new_source),CONFIG_TYPE_INT64);
				config_setting_set_int64(new_dest,long_value);
				continue;
			case CONFIG_TYPE_FLOAT:
				double_value = config_setting_get_float(new_source);
				new_dest = config_setting_add (dest, config_setting_name(new_source),CONFIG_TYPE_FLOAT);
				config_setting_set_float(new_dest,double_value);
				continue;
			case CONFIG_TYPE_BOOL:
				int_value = config_setting_get_bool(new_source);
				new_dest = config_setting_add (dest, config_setting_name(new_source),CONFIG_TYPE_BOOL);
				config_setting_set_bool(new_dest,int_value);
				continue;
			case CONFIG_TYPE_STRING:
				string = config_setting_get_string(new_source);
				new_dest = config_setting_add (dest, config_setting_name(new_source),CONFIG_TYPE_STRING);
				config_setting_set_string(new_dest,string);
				continue;
			default:
				return 0;
			}
		}
	}

	return 1;
}

/*********************/
/* Returns :

TRUE on success, FALSE if an error occurred
*/
static int __list_create(const gchar * table, const gchar * file, va_list ap)
{
	file_data_t * config;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	create_tree(config->config,NULL,NULL,NULL,CONFIG_TYPE_LIST,ap);

	config->config_update = TRUE;
	return TRUE;
}
int list_create(const gchar * table, const gchar * file, ...)
{
	int ret;
	va_list ap;

	g_static_mutex_lock(&file_mutex);
	va_start(ap, file);
	ret = __list_create(table, file, ap);
	va_end(ap);
	g_static_mutex_unlock(&file_mutex);

	return ret;
}

/*********************/
/* Returns :

TRUE on success, FALSE if an error occurred
*/
static int __group_create(const gchar * table, const gchar * file, va_list ap)
{
	file_data_t * config;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	create_tree(config->config,NULL,NULL,NULL,CONFIG_TYPE_GROUP,ap);

	config->config_update = TRUE;
	return TRUE;
}
int group_create(const gchar * table, const gchar * file, ...)
{
	int ret;
	va_list ap;

	g_static_mutex_lock(&file_mutex);
	va_start(ap, file);
	ret = __group_create(table, file, ap);
	va_end(ap);
	g_static_mutex_unlock(&file_mutex);

	return ret;
}

static gchar * __copy_group(const gchar * src_table, const gchar * src_file, const gchar * dst_table, const gchar * dst_file, const gchar * group_name, va_list ap)
{
	file_data_t * src_config = NULL;
	file_data_t * dst_config = NULL;
	config_setting_t * src_setting = NULL;
	config_setting_t * dst_setting = NULL;
	gchar * path;
	gchar * full_path;
	gchar * new_group_name = NULL;
	const gchar * group_name_used = NULL;

	path = get_path(ap);
	full_path = add_entry_to_path(path, (gchar *)group_name);
	if(full_path == NULL) {
		return NULL;
	}

	src_config = get_config(src_table,src_file);
	if(src_config==NULL) {
		g_free(path);
		g_free(full_path);
		return NULL;
	}

	src_setting = config_lookup (src_config->config, full_path);
	if( src_setting == NULL ) {
		g_free(path);
		g_free(full_path);
		return NULL;
	}

	dst_config = get_config(dst_table,dst_file);
	if(dst_config==NULL) {
		g_free(path);
		g_free(full_path);
		return NULL;
	}

	dst_setting = config_lookup(dst_config->config, full_path);
	g_free(full_path);
	/* if the setting does not exist, create it */
	if( dst_setting == NULL ) {
		group_name_used = group_name;
	}
	/* else find a new name for it */
	else {
		new_group_name =  __get_unused_group_on_path(dst_table, dst_file, path);
		group_name_used = new_group_name;
	}

	if( path != NULL ) {
		dst_setting = config_lookup(dst_config->config, path);
	} else {
		dst_setting = config_root_setting(dst_config->config);
	}
	dst_setting = config_setting_add(dst_setting,group_name_used,CONFIG_TYPE_GROUP);

	g_free(path);

	if(!copy_config(src_setting,dst_setting)) {
		return NULL;
	}

	if(new_group_name != NULL ) {
		return new_group_name;
	}

	return g_strdup(group_name);
}
/* return a copy of the name used for the destination
MUST BE FREED ! */
gchar * copy_group(const gchar * src_table, const gchar * src_file, const gchar * dst_table, const gchar * dst_file, const gchar * group_name, ...)
{
	gchar * ret;
	va_list ap;

	g_static_mutex_lock(&file_mutex);
	va_start(ap, group_name);
	ret = __copy_group(src_table,src_file,dst_table,dst_file,group_name,ap);
	va_end(ap);
	g_static_mutex_unlock(&file_mutex);
	return ret;
}

/* Update an entry from a network frame */
/* return -1 if fails */
gint entry_update(gchar * data)
{
	gchar ** elements;
	config_setting_t * setting;
	file_data_t * config;
	gint value;

	elements = g_strsplit(data,NETWORK_DELIMITER,0);

	if( g_strcmp0(elements[0],ENTRY_TYPE_INT) == 0 ) {
		config = get_config(elements[1],elements[2]);
		if(config==NULL) {
			return -1;
		}

		setting = config_lookup (config->config, elements[3]);
		if(setting==NULL) {
			return -1;
		}

		value = g_ascii_strtoll(elements[4],NULL,10);
		if(config_setting_set_int (setting, value) == CONFIG_FALSE) {
			werr(LOGUSER,"Errror setting %s/%s/%s to %d",elements[1],elements[2],elements[3],value);
		} else {
			config->config_update = TRUE;
		}
	}

	else if( g_strcmp0(elements[0],ENTRY_TYPE_STRING) == 0 ) {
		config = get_config(elements[1],elements[2]);
		if(config==NULL) {
			return -1;
		}

		setting = config_lookup (config->config, elements[3]);
		if(setting==NULL) {
			return -1;
		}

		if(config_setting_set_string (setting,elements[4]) == CONFIG_FALSE) {
			werr(LOGUSER,"Errror setting %s/%s/%s to %s",elements[1],elements[2],elements[3],elements[4]);
		} else {
			config->config_update = TRUE;
		}
	}
	g_strfreev(elements);

	return 0;
}


/* Remove a character's file and remove the corresponding entry in fileDB cache*/
/* return -1 if fails */
gint entry_destroy(const gchar * id)
{
	gchar * filename;
	int res;

	filename = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", CHARACTER_TABLE, "/", id, NULL);


	res = g_unlink(filename);
	if(res != 0 ) {
		werr(LOGUSER,"Error deleting file \"%s\"",filename);
	}

	g_hash_table_remove(fileDB,filename);

	return res;
}
