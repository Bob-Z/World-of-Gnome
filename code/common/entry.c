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
#include "file.h"
#include <libconfig.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>

list_t * entry_list = NULL;

/*********************
*********************/
static void config_print_error(char * file, const config_t * config)
{
	werr(LOGUSER,"libconfig error %s@%d : %s\n",
		 file,
		 config_error_line(config),
		 config_error_text(config));
}

/*********************
*********************/
static void write_config(const config_t * config,const char * table, const char * file)
{
	char filename[512] = "";
	char fullname[512] = "";

	strcat(filename,table);
	strcat(filename,"/");
	strcat(filename,file);

	strcat(fullname,getenv("HOME"));
	strcat(fullname,"/");
	strcat(fullname,base_directory);
	strcat(fullname,"/");
	strcat(fullname,filename);

	file_lock(filename);
	config_write_file((config_t*)config,fullname);
	file_unlock(filename);
}
/*********************
*********************/
#if 0
static void free_config(config_t * config)
{
	config_destroy(config);
	free(config);
}
#endif
/*********************
*********************/
static const config_t * load_config(char * filename)
{
	int ret;
	config_t * config = NULL;
	struct stat sts;
	char fullname[512] = "";

	strcat(fullname,getenv("HOME"));
	strcat(fullname,"/");
	strcat(fullname,base_directory);
	strcat(fullname,"/");
	strcat(fullname,filename);

	config = malloc(sizeof(config_t));
	if(config == NULL) {
		return NULL;
	}
	bzero(config,sizeof(config_t));

	ret = config_read_file (config, fullname);
	if( ret == CONFIG_FALSE ) {
		// For client, only print errors when file is present
		// If file is not present, let get_config ask for a network update
		if(stat(fullname,&sts) != -1 || client_server == SERVER) {
			config_print_error(fullname,config);
		}
		return NULL;
	}

	return config;
}

/****************************************************
Remove an entry from the DB
******************************************************/
void entry_remove(char * filename)
{
	const config_t * old_config;

	wlog(LOGDEBUG,"Removing entry : %s",filename);
        /* Clean-up old anim if any */
        SDL_LockMutex(entry_mutex);
        old_config = list_find(entry_list,filename);
        if( old_config ) {
		/* TODO: Since read_string (and some more) use const pointer to config_t data
		we cannot delete a config_t structure freely because afterward, some functions
		may point to freed (and corrupted)  string data.
		This is a major memory leak which need a clever solution.
		*/
#if 0
                free_config(old_config);
#endif
        }

        entry_list = list_update(entry_list,filename,NULL);

        SDL_UnlockMutex(entry_mutex);
}

/*********************
returned config MUST BE config_destroy and FREED
*********************/
static const config_t * get_config(const char * table, const char * file)
{
	char filename[512] = "";
	const config_t * config;

	strcat(filename,table);
	strcat(filename,"/");
	strcat(filename,file);

	wlog(LOGDEBUG,"Entry get : %s",filename);

	SDL_LockMutex(entry_mutex);
        config = list_find(entry_list,filename);

	if(config) {
		wlog(LOGDEBUG,"Entry found : %s",filename);
		SDL_UnlockMutex(entry_mutex);
		return config;
	}

	file_lock(filename);

	if( (config=load_config(filename)) == NULL ) {
		wlog(LOGDEBUG,"Entry asked : %s",filename);
		file_update(context_get_list_first(),filename);
		file_unlock(filename);
		SDL_UnlockMutex(entry_mutex);
		return NULL;
	}
	file_unlock(filename);

	wlog(LOGDEBUG,"Entry loaded : %s",filename);
	entry_list = list_update(entry_list,filename,(config_t*)config);
	SDL_UnlockMutex(entry_mutex);

	return config;
}

/*********************
returned string MUST BE FREED
*********************/
static char * add_entry_to_path(char * path, char * entry)
{
	char new_path[512] = "";

	if( path == NULL ) {
		return strdup(entry);
	} else {
		strcat(new_path,path);
		strcat(new_path,".");
		strcat(new_path,entry);
	}

	return strdup(new_path);
}

/*********************
Create a libconfig path from a list of string
the returned path MUST BE FREED
*********************/
static char * get_path(va_list ap)
{
	char * path=NULL;
	char * entry = NULL;

	entry=va_arg(ap,char*);
	if( entry == NULL ) {
		return NULL;
	}

	while( entry != NULL ) {
		path = add_entry_to_path(path,entry);
		if(path == NULL) {
			return NULL;
		}
		entry=va_arg(ap,char*);
	}

	return path;
}

/*********************
return FALSE on error
*********************/
static int __read_int(const char * table, const char * file, int * res, va_list ap)
{
	const config_t * config = NULL;
	char * path=NULL;
	int result;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	path = get_path(ap);
	if(path == NULL) {
		return FALSE;
	}

	if(config_lookup_int (config, path, &result)==CONFIG_FALSE) {
		free(path);
		return FALSE;
	}
	free(path);

	*res = result;

	return TRUE;
}

/*********************
return FALSE on error
*********************/
int read_int(const char * table, const char * file, int * res, ...)
{
	int ret;
	va_list ap;

	va_start(ap, res);
	ret = __read_int(table, file, res, ap);
	va_end(ap);

	return ret;
}

/*********************
return FALSE on error
res MUST BE FREED by caller
*********************/
static int __read_string(const char * table, const char * file, const char ** res, va_list ap)
{
	const config_t * config = NULL;
	char * path;
	const char * result;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	path = get_path(ap);
	if(path == NULL) {
		return FALSE;
	}

	if(config_lookup_string (config, path, &result)==CONFIG_FALSE) {
//		g_warning("%s: Can't read %s/%s/%s",__func__,table,file,path);
		free(path);
		return FALSE;
	}
	free(path);
	*res = strdup(result);

	return TRUE;
}

/*********************
return FALSE on error
*********************/
int read_string(const char * table, const char * file, const char ** res, ...)
{
	int ret;
	va_list ap;

	va_start(ap, res);
	ret = __read_string(table, file, res, ap);
	va_end(ap);

	return ret;
}

/*********************
return FALSE on error
res MUST BE FREED by caller
*********************/
static int __read_list_index(const char * table, const char * file, const char ** res, int index, va_list ap)
{
	const config_t * config = NULL;
	config_setting_t * setting = NULL;
	char * path;
	const char * result;

	*res = NULL;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	path = get_path(ap);
	if(path == NULL) {
		return FALSE;
	}

	setting = config_lookup (config, path);
	if( setting == NULL ) {
//		g_warning("%s: Can't read %s/%s/%s",__func__,table,file,path);
		free(path);
		return FALSE;
	}
	free(path);

	result = config_setting_get_string_elem (setting,index);
	if(result == NULL ) {
		return FALSE;
	}

	*res = strdup(result);
	return TRUE;
}

/*********************
return FALSE on error
*********************/
int read_list_index(const char * table, const char * file, const char ** res,int index, ...)
{
	int ret;
	va_list ap;

	va_start(ap, index);
	ret = __read_list_index(table,file, res,index, ap);
	va_end(ap);

	return ret;
}

/*********************
return FALSE on error
Each elements of res MUST BE FREED by caller
*********************/
static int __read_list(const char * table, const char * file, char *** res, va_list ap)
{
	const config_t * config = NULL;
	config_setting_t * setting = NULL;
	char * path;
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

	setting = config_lookup (config, path);
	if( setting == NULL ) {
		free(path);
		return FALSE;
	}
	free(path);

	while( (elem=config_setting_get_string_elem (setting,i )) != NULL ) {
		i++;
		*res = realloc(*res,(i+1)*sizeof(char *));
		(*res)[i-1] = strdup(elem);
		(*res)[i] = NULL;

	}


	if(*res == NULL) {
		return FALSE;
	}

	return TRUE;
}

/*********************
return FALSE on error
*********************/
int read_list(const char * table, const char * file, char *** res, ...)
{
	int ret;
	va_list ap;

    va_start(ap, res);
	ret = __read_list(table, file, res, ap);
    va_end(ap);

	return ret;
}

/*********************
return FALSE on error
*********************/
static config_setting_t * create_tree(const config_t * config, config_setting_t * prev_setting, char * prev_entry, char * path, int type, va_list ap)
{
	char * entry = NULL;
	char new_path[512] = "";
	config_setting_t * setting = NULL;

	if( prev_setting == NULL) {
		prev_setting = config_root_setting(config);
	}

	entry = va_arg(ap,char*);

	/* no more leaf */
	if(entry == NULL) {
		/* check if leaf exists */
		setting = config_lookup(config, path);
		if( setting == NULL) {
			/* create the leaf */
			setting=config_setting_add(prev_setting, prev_entry,type);
		}
		free(path);
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
				free(path);
				return NULL;
			}
		}
	}

	/* update path */
	if( path ) {
		strcat(new_path,path);
		strcat(new_path,".");
		strcat(new_path,entry);
		free(path);
		path = strdup(new_path);
	} else {
		path = strdup(entry);
	}

	return create_tree(config,setting,entry,path,type,ap);
}

/*********************
return FALSE on error
*********************/
static int __write_int(const char * table, const char * file, int data, va_list ap)
{
	config_setting_t * setting;
	const config_t * config;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	setting = create_tree(config,NULL,NULL,NULL,CONFIG_TYPE_INT,ap);

	/* update int */
	if(config_setting_set_int(setting, data)==CONFIG_FALSE) {
		return FALSE;
	}

	write_config(config,table,file);
	return TRUE;
}

/*********************
return FALSE on error
*********************/
int write_int(const char * table, const char * file,int data, ...)
{
	int ret;
	va_list ap;

	va_start(ap, data);
	ret = __write_int(table, file, data, ap);
	va_end(ap);

	return ret;
}

/*********************
return FALSE on error
*********************/
static int __write_string(const char * table, const char * file, const char * data, va_list ap)
{
	config_setting_t * setting;
	const config_t * config;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	setting = create_tree(config,NULL,NULL,NULL,CONFIG_TYPE_STRING,ap);

	/* update string */
	if(config_setting_set_string(setting, data)==CONFIG_FALSE) {
		return FALSE;
	}

	write_config(config,table,file);
	return TRUE;
}

/*********************
return FALSE on error
*********************/
int write_string(const char * table, const char * file, const char * data, ...)
{
	int ret;
	va_list ap;

	va_start(ap, data);
	ret = __write_string(table, file, data, ap);
	va_end(ap);

	return ret;
}

/*********************
return FALSE on error
*********************/
static int __write_list_index(const char * table, const char * file, const char * data, int index, va_list ap)
{
	config_setting_t * setting = NULL;
	const config_t * config;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	setting = create_tree(config,NULL,NULL,NULL,CONFIG_TYPE_ARRAY,ap);

	if(config_setting_set_string_elem(setting,index,data)==NULL) {
		return FALSE;
	}

	write_config(config,table,file);
	return TRUE;
}

/*********************
return FALSE on error
*********************/
int write_list_index(const char * table, const char * file, const char * data, int index, ...)
{
	int ret;
	va_list ap;

	va_start(ap, index);
	ret = __write_list_index(table, file, data,index,ap);
	va_end(ap);

	return ret;
}

/*********************
return FALSE on error
*********************/
static int __write_list(const char * table, const char * file, char ** data, va_list ap)
{
	config_setting_t * setting;
	const config_t * config;
	int i=0;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	setting = create_tree(config,NULL,NULL,NULL,CONFIG_TYPE_ARRAY,ap);

	while(data[i] != NULL) {
		if(config_setting_set_string_elem(setting,-1,data[i])==NULL) {
			return FALSE;
		}
		i++;
	}

	write_config(config,table,file);

	return TRUE;
}

/*********************
return FALSE on error
*********************/
int write_list(const char * table, const char * file, char ** data, ...)
{
	int ret;
	va_list ap;

	va_start(ap, data);
	ret = __write_list(table, file, data, ap);
	va_end(ap);

	return ret;
}

/*********************
return FALSE on error
*********************/
static int __add_to_list(const char * table, const char * file, const char * to_be_added, va_list ap)
{
	const config_t * config = NULL;
	config_setting_t * setting = NULL;
	char * path;

	config = get_config(table,file);
	if(config==NULL) {
		return 0;
	}

	path = get_path(ap);
	if(path == NULL) {
		return FALSE;
	}

	setting = config_lookup (config, path);
	if( setting == NULL ) {
		free(path);
		return 0;
	}
	free(path);

	if(config_setting_set_string_elem(setting,-1,to_be_added)==NULL) {
		return 0;
	}

	write_config(config,table,file);

	return 1;
}

/*********************
return FALSE on error
*********************/
int add_to_list(const char * table, const char * file, const char * to_be_added, ...)
{
	int ret;
	va_list ap;

	va_start(ap, to_be_added);
	ret = __add_to_list(table,file,to_be_added,ap);
	va_end(ap);

	return ret;
}

/*********************
return FALSE on error
*********************/
static int __remove_group(const char * table, const char * file, const char * group, va_list ap)
{

	const config_t * config;
	config_setting_t * setting = NULL;
	char * path;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	path = get_path(ap);
	if(path != NULL) {
		setting = config_lookup(config,path);
		free(path);
	} else {
		setting = config_root_setting(config);
	}

	if( setting == NULL ) {
		return FALSE;
	}

	if( config_setting_remove(setting,group) == CONFIG_FALSE) {
		return FALSE;
	}

	write_config(config,table,file);

	return TRUE;
}

/*********************
return FALSE on error
*********************/
int remove_group(const char * table, const char * file, const char * group, ...)
{
	int ret;
	va_list ap;

	va_start(ap, group);
	ret = __remove_group(table, file, group, ap);
	va_end(ap);

	return ret;
}
/**********************
get_unused_list_entry
find an unused tag in a list, add it to the list and return it.
returned string must be freed
**********************/
char * get_unused_list_entry(const char * table, const char * file, ...)
{
	char **  list;
	char tag[7];
	int index = 0;
	int i = 0;
	va_list ap;

	sprintf(tag,"A%05x",index);

	va_start(ap,file);
	if(__read_list(table,file,&list,ap)) {
		while(list[i] != NULL ) {
			if( strcmp(list[i],tag) == 0 ) {
				index++;
				sprintf(tag,"A%05x",index);
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

	return strdup(tag);
}

/**********************
__get_unused_group
find an unused group, create it and return its name
returned string must be freed
**********************/
char * __get_unused_group(const char * table, const char * file,  va_list ap)
{
	char tag[7];
	int index = 0;
	char * path;
	config_setting_t * setting;
	const config_t * config;

	config = get_config(table,file);
	if(config==NULL) {
		return NULL;
	}

	sprintf(tag,"A%05x",index);

	path = get_path(ap);

	if(path != NULL) {
		setting = config_lookup(config,path);
		free(path);
	} else {
		setting = config_root_setting(config);
	}

	while( config_setting_add(setting,tag,CONFIG_TYPE_GROUP) == NULL ) {
		index++;
		sprintf(tag,"A%05x",index);
	}

	return strdup(tag);
}

/**********************
get_unused_group
find an unused group, create it and return its name
returned string must be freed
**********************/
char * get_unused_group(const char * table, const char * file, ...)
{
	char * ret;
	va_list ap;

	va_start(ap, file);
	ret = __get_unused_group(table, file, ap);
	va_end(ap);

	return ret;
}

/**********************
__get_unused_group_on_path
find an unused group, DO NOT CREATE IT and return its name
returned string must be freed
**********************/
char * __get_unused_group_on_path(const char * table, const char * file, char * path)
{
	char tag[7];
	int index = 0;
	config_setting_t * setting;
	const config_t * config;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	sprintf(tag,"A%05x",index);

	if(path != NULL) {
		setting = config_lookup(config,path);
		free(path);
	} else {
		setting = config_root_setting(config);
	}

	while( config_setting_add(setting,tag,CONFIG_TYPE_GROUP) == NULL ) {
		index++;
		sprintf(tag,"A%05x",index);
	}

	config_setting_remove(setting,tag);

	return strdup(tag);
}

/**********************
get_group_list
Return a list of the name of the elements in the specified group
returned list must be freed  (g_free)
 return FALSE on error
**********************/
int get_group_list(const char * table, const char * file, char *** res, ...)
{
	char * path;
	const config_t * config;
	va_list ap;
	config_setting_t * setting;
	config_setting_t * elem_setting;
	int index = 0;

	*res = NULL;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	va_start(ap,res);
	path = get_path(ap);
	va_end(ap);

	if(path != NULL && path[0] != 0) {
		setting = config_lookup(config,path);
		free(path);
	} else {
		setting = config_root_setting(config);
	}

	/* The path does not exist in the conf file */
	if(setting == NULL ) {
		return FALSE;
	}

	while((elem_setting=config_setting_get_elem(setting,index))!= NULL ) {
		index++;
		*res = realloc(*res,(index+1)*sizeof(char *));
		(*res)[index-1] = (char *)config_setting_name(elem_setting);
		(*res)[index] = NULL;
	}

	if( *res == NULL) {
		return FALSE;
	}

	return TRUE;
}

/*********************
return 0 on failure
*********************/
static int __remove_from_list(const char * table, const char * file, const char * to_be_removed, va_list ap)
{
	const config_t * config = NULL;
	config_setting_t * setting = NULL;
	char * path;
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

	setting = config_lookup (config, path);
	if( setting == NULL ) {
		free(path);
		return 0;
	}
	free(path);

	while( (elem=config_setting_get_string_elem (setting,i )) != NULL ) {
		if( strcmp(elem,to_be_removed) == 0 ) {
			if(config_setting_remove_elem (setting,i)==CONFIG_FALSE) {
				return 0;
			}

			write_config(config,table,file);
			return 1;
		}
		i++;
	}

	return 0;
}

/*********************
return 0 on failure
*********************/
int remove_from_list(const char * table, const char * file, const char * to_be_removed, ...)
{
	int ret;
	va_list ap;

	va_start(ap, to_be_removed);
	ret = __remove_from_list(table,file,to_be_removed,ap);
	va_end(ap);
	return ret;
}

/*********************
return 0 on failure
*********************/
int copy_config(config_setting_t * source,config_setting_t * destination);
int copy_aggregate(config_setting_t * source, config_setting_t * dest, int type)
{
	const char * setting_name;
	config_setting_t * new_dest;
	int index=0;
	char tag[7];

	setting_name = config_setting_name(source);
	if(setting_name == NULL) {
		return 0;
	}

	new_dest=config_setting_add(dest,setting_name,type);
	if( new_dest == NULL ) {
		/* Try to find an available name */
		sprintf(tag,"A%05x",index);

		while((new_dest=config_setting_add(dest,setting_name,type))== NULL) {
			index++;
			sprintf(tag,"A%05x",index);
		}
	}

	if(!copy_config(source,new_dest)) {
		config_setting_remove(dest,setting_name);
		return 0;
	}
	return 1;
}

/*********************
return 0 on failure
*********************/
int copy_config(config_setting_t * source, config_setting_t * dest)
{
	config_setting_t * new_source;
	config_setting_t * new_dest;
	int index = -1;
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

/*********************
return 0 on failure
*********************/
static int __list_create(const char * table, const char * file, va_list ap)
{
	const config_t * config;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	create_tree(config,NULL,NULL,NULL,CONFIG_TYPE_LIST,ap);

	write_config(config,table,file);
	return TRUE;
}

/*********************
return 0 on failure
*********************/
int list_create(const char * table, const char * file, ...)
{
	int ret;
	va_list ap;

	va_start(ap, file);
	ret = __list_create(table, file, ap);
	va_end(ap);

	return ret;
}

/*********************
return 0 on failure
*********************/
static int __group_create(const char * table, const char * file, va_list ap)
{
	const config_t * config;

	config = get_config(table,file);
	if(config==NULL) {
		return FALSE;
	}

	create_tree(config,NULL,NULL,NULL,CONFIG_TYPE_GROUP,ap);

	write_config(config,table,file);
	return TRUE;
}

/*********************
return 0 on failure
*********************/
int group_create(const char * table, const char * file, ...)
{
	int ret;
	va_list ap;

	va_start(ap, file);
	ret = __group_create(table, file, ap);
	va_end(ap);

	return ret;
}

/*********************
return 0 on failure
*********************/
static char * __copy_group(const char * src_table, const char * src_file, const char * dst_table, const char * dst_file, const char * group_name, va_list ap)
{
	const config_t * src_config = NULL;
	const config_t * dst_config = NULL;
	config_setting_t * src_setting = NULL;
	config_setting_t * dst_setting = NULL;
	char * path;
	char * full_path;
	char * new_group_name = NULL;
	const char * group_name_used = NULL;

	path = get_path(ap);
	full_path = add_entry_to_path(path, (char *)group_name);
	if(full_path == NULL) {
		return NULL;
	}

	src_config = get_config(src_table,src_file);
	if(src_config==NULL) {
		free(path);
		free(full_path);
		return NULL;
	}

	src_setting = config_lookup (src_config, full_path);
	if( src_setting == NULL ) {
		free(path);
		free(full_path);
		return NULL;
	}

	dst_config = get_config(dst_table,dst_file);
	if(dst_config==NULL) {
		free(path);
		free(full_path);
		return NULL;
	}

	dst_setting = config_lookup(dst_config, full_path);
	free(full_path);
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
		dst_setting = config_lookup(dst_config, path);
	} else {
		dst_setting = config_root_setting(dst_config);
	}
	dst_setting = config_setting_add(dst_setting,group_name_used,CONFIG_TYPE_GROUP);

	free(path);

	if(!copy_config(src_setting,dst_setting)) {
		return NULL;
	}

	if(new_group_name != NULL ) {
		return new_group_name;
	}

	return strdup(group_name);

}
/**************************************
return a copy of the name used for the destination
MUST BE FREED !
***************************************/
char * copy_group(const char * src_table, const char * src_file, const char * dst_table, const char * dst_file, const char * group_name, ...)
{
	char * ret;
	va_list ap;

	va_start(ap, group_name);
	ret = __copy_group(src_table,src_file,dst_table,dst_file,group_name,ap);
	va_end(ap);
	return ret;
}

/*********************************************
Update an entry from a network frame
return -1 if fails
*********************************************/
int entry_update(char * data)
{
	char * source;
	char * elements[5] = {NULL,NULL,NULL,NULL,NULL};
	config_setting_t * setting;
	const config_t * config= NULL;
	int value;
	char * token;
	int index = 0;
	int ret = -1;

	source = strdup(data);
	token = strtok(data,NETWORK_DELIMITER);
	while( token != NULL ) {
		elements[index] = strdup(token);
		index++;
		if(index>=5) {
			werr(LOGDEV,"Split sting error of : %s",source);
			goto entry_update_cleanup;
		}
	}

	config = get_config(elements[1],elements[2]);
	if(config==NULL) {
		goto entry_update_cleanup;
	}

	setting = config_lookup (config, elements[3]);
	if(setting==NULL) {
		goto entry_update_cleanup;
	}

	if( strcmp(elements[0],ENTRY_TYPE_INT) == 0 ) {
		value = atoll(elements[4]);
		if(config_setting_set_int (setting, value) == CONFIG_FALSE) {
			werr(LOGUSER,"Errror setting %s/%s/%s to %d",elements[1],elements[2],elements[3],value);
		} else {
			write_config(config,elements[1],elements[2]);
		}
	}
	else if( strcmp(elements[0],ENTRY_TYPE_STRING) == 0 ) {
		if(config_setting_set_string (setting,elements[4]) == CONFIG_FALSE) {
			werr(LOGUSER,"Errror setting %s/%s/%s to %s",elements[1],elements[2],elements[3],elements[4]);
		} else {
			write_config(config,elements[1],elements[2]);
		}
	}

	ret = 0;

entry_update_cleanup:
	free(source);
	for(index=0;index<5;index++) {
		if(elements[index]) {
			free(elements[index]);
		}
	}

	return ret;
}

/***********************************************
dELETE a character's file
return -1 if fails
***********************************************/
int entry_destroy(const char * id)
{
	char fullname[512];
	int res;

	strcat(fullname,getenv("HOME"));
	strcat(fullname,"/");
	strcat(fullname,base_directory);
	strcat(fullname,"/");
	strcat(fullname,CHARACTER_TABLE);
	strcat(fullname,"/");
	strcat(fullname,id);

	res = unlink(fullname);
	if(res != 0 ) {
		werr(LOGUSER,"Error deleting file \"%s\"",fullname);
	}

	return res;
}
