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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

list_t * file_list = NULL;

typedef struct file_tag {
        Uint32 timestamp;
        SDL_mutex * mutex;
} file_t;

/****************************************
  filename is "table/dir/file"
 *****************************************/
void file_lock(const char * filename)
{
	file_t * file_data;

	SDL_LockMutex(file_list_mutex);
	file_data = list_find(file_list,filename);
	if( file_data == NULL ) {
		file_data = malloc(sizeof(file_t));
		file_data->timestamp = 0;
		file_data->mutex = SDL_CreateMutex();

		file_list = list_update(file_list,filename,file_data);
	}
	SDL_UnlockMutex(file_list_mutex);

	SDL_LockMutex(file_data->mutex);
}

/****************************************
  filename is "table/dir/file"
 *****************************************/
void file_unlock(const char * filename)
{
	const file_t * file_data;

	SDL_LockMutex(file_list_mutex);
	file_data = list_find(file_list,filename);
	SDL_UnlockMutex(file_list_mutex);

	if( file_data == NULL ) {
		return;
	}

	SDL_UnlockMutex(file_data->mutex);
}

/****************************************
  filename is "table/dir/file"
 *****************************************/
void file_update(context_t * context, char * filename)
{
	Uint32 current_time = SDL_GetTicks();
	file_t * file_data;

	if(client_server == SERVER) {
		return;
	}

	SDL_LockMutex(file_list_mutex);
	file_data = list_find(file_list,filename);
	SDL_UnlockMutex(file_list_mutex);

	/* Avoid flooding the server */
	if( file_data->timestamp != 0 && file_data->timestamp + FILE_REQUEST_TIMEOUT > current_time ) {
		werr(LOGDEBUG,"Previous request of file  %s has been %d ms ago",filename,current_time - file_data->timestamp );
		return;
	}

	network_send_req_file(context, filename);

	file_data->timestamp = current_time;
}

/****************************
  Parameter 1: Name of the table to create the new file
  return the name of an available empty file
  if success the file is created on disk
  the return string must be freed by caller
 ****************************/
char * file_new(char * table)
{
	DIR * dir;
	struct dirent * ent;
	char dirname[512] = "";
	char filename[512] = "";
	char tag[10];
	int index = 0;
	int fd;

	sprintf(tag,"A%05x",index);

	strcat(dirname,getenv("HOME"));
	strcat(dirname,"/");
	strcat(dirname,base_directory);
	strcat(dirname,"/");
	strcat(dirname,table);

	dir = opendir(dirname);

	while(( ent = readdir(dir)) != NULL ) {
		if( strcmp(ent->d_name,".") == 0 ) {
			continue;
		}
		if( strcmp(ent->d_name,"..") == 0 ) {
			continue;
		}
		if( strcmp(ent->d_name,tag) == 0 ) {
			index++;
			sprintf(tag,"A%05x",index);
			rewinddir(dir);
			continue;
		}
	}

	closedir(dir);

	strcat(filename,dirname);
	strcat(filename,"/");
	strcat(filename,tag);

	fd = creat(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	close(fd);

	return strdup(tag);
}

/****************************
  filename is "table/dir/file"
  Fill contents and size with data from filename file.
  contents MUST BE FREED by caller
  return TRUE id success
 ****************************/
int file_get_contents(const char *filename,char **contents,int *length)
{
	char fullname[512] = "";
	struct stat sts;
	int fd;
	ssize_t size;
	char * buf;

	strcat(fullname,getenv("HOME"));
	strcat(fullname,"/");
	strcat(fullname,base_directory);
	strcat(fullname,"/");
	strcat(fullname,filename);

	file_lock(filename);

	if( stat(fullname, &sts) == -1) {
		werr(LOGDEV,"Error stat on file %s\n",fullname);
		return FALSE;
	}

	fd = open(fullname,O_RDONLY);
	if(fd == -1) {
		werr(LOGDEV,"Error open on file %s\n",fullname);
		return FALSE;
	}

	buf = malloc(sts.st_size);
	if( buf == NULL) {
		return FALSE;
	}

	size = read(fd,buf,sts.st_size);
	if( size == -1) {
		free(buf);
		werr(LOGDEV,"Error read on file %s\n",fullname);
		return FALSE;
	}

	close(fd);

	*contents = buf;
	*length = size;

	file_unlock(filename);

	return TRUE;
}
/****************************
  file_set_contents
  filename is "table/dir/file"
  return TRUE id success
 ****************************/
int file_set_contents(const char *filename,const char *contents,int length)
{
	char fullname[512] = "";
	int fd;
	ssize_t size;

	strcat(fullname,getenv("HOME"));
	strcat(fullname,"/");
	strcat(fullname,base_directory);
	strcat(fullname,"/");
	strcat(fullname,filename);

	file_lock(filename);

	fd = creat(fullname,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
	if(fd == -1) {
		werr(LOGDEV,"Error open on file %s\n",fullname);
		return FALSE;
	}

	size = write(fd,contents,length);
	if( size == -1) {
		werr(LOGDEV,"Error write on file %s\n",fullname);
		return FALSE;
	}

	close(fd);

	file_unlock(filename);

	return TRUE;
}

/******************************************************
******************************************************/
void file_copy(char * src_name, char * dst_name)
{
        FILE *src;
        FILE *dst;
        int i;

        src = fopen(src_name, "rb");
        dst = fopen(dst_name, "wb");

        for (i = getc(src); i != EOF; i = getc(src))
        {
                putc(i, dst);
        }

        fclose(dst);
        fclose(src);
}

/***************************************************
 return 0 if directory was successfully created
****************************************************/
static int mkdir_all(const char * pathname)
{
        char * token;
        char * source;
        int ret = -1;
        char directory[512] = "";
	char *saveptr;

        if(pathname == NULL) {
                return -1;
        }

        source = strdup(pathname);

        token =  strtok_r(source,"/",&saveptr);

        while( token != NULL ) {
                strcat(directory,"/");
                strcat(directory,token);
                ret = mkdir(directory,0775);
                token =  strtok_r(NULL,"/",&saveptr);
        }

        free(source);

        return ret;
}

/***************************************************
 create the path of fullname.
 fullname is a path + a file name. Only the path is
 created here, not the file itself.
 return 0 if directory was successfully created
****************************************************/
int file_create_directory(char * fullname)
{
        char * directory = strdup(fullname);
        int i;
        int ret;

        /* Remove file name, just kee directory name */
        for( i = strlen(directory); i > 0; i--) {
                if(directory[i] == '/') {
                        directory[i]=0;
                        break;
                }
        }

        ret = mkdir_all(directory);

        free(directory);

        return ret;
}

