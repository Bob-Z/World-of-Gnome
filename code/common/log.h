/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2020 carabobz@gmail.com

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

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <string>

#ifdef __cplusplus
extern "C"
{
#endif

void log_set_level(char * log_level);
void log_add_file_filter(const char * file);
void log_add_func_filter(const char * func);
void log_print(int type, const char * file, const char * func, int line, FILE *stream, int level, const char * format, ...);
void log_print_std(int type, const char * file, const char * func, int line, FILE *stream, int level, const std::string & format);

#define LOGUSER			0
#define LOGDESIGNER		1
#define LOGDEVELOPER	2

#define TYPELOG		0
#define TYPEERR		1

#define wlog(level,str,args...) log_print(TYPELOG,__FILE__,__func__,__LINE__,stdout,level,str, ## args)
#define werr(level,str,args...) log_print(TYPEERR,__FILE__,__func__,__LINE__,stderr,level,str, ## args)

#define LOG(str,args...) log_print_std(TYPELOG,__FILE__,__func__,__LINE__,stdout,LOGDEVELOPER,str)
#define ERR(str,args...) log_print_std(TYPEERR,__FILE__,__func__,__LINE__,stderr,LOGDEVELOPER,str)
#define LOG_USER(str,args...) log_print_std(TYPELOG,__FILE__,__func__,__LINE__,stdout,LOGUSER,str)
#define ERR_USER(str,args...) log_print_std(TYPEERR,__FILE__,__func__,__LINE__,stderr,LOGUSER,str)
#define LOG_DESIGN(str,args...) log_print_std(TYPELOG,__FILE__,__func__,__LINE__,stdout,LOGDESIGNER,str)
#define ERR_DESIGN(str,args...) log_print_std(TYPEERR,__FILE__,__func__,__LINE__,stderr,LOGDESIGNER,str)

#ifdef __cplusplus
}
#endif

#endif // LOG_H
