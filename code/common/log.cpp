/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2016 carabobz@gmail.com

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

#ifndef LOG_C
#define LOG_C

#include "log.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <SDL2/SDL.h>

#ifdef __cplusplus
extern "C"
{
#endif

static int log_level = 0;

const char *log_level_string[3] =
{ "user", "dev", "debug" };

char ** file_filter = nullptr;
int file_filter_num = 0;

char ** func_filter = nullptr;
int func_filter_num = 0;

/**************************************************
 **************************************************/
void log_add_file_filter(const char * file)
{
	file_filter_num++;
	file_filter = (char **) realloc(file_filter,
			sizeof(char*) * file_filter_num);
	file_filter[file_filter_num - 1] = strdup(file);
}

/**************************************************
 **************************************************/
void log_add_func_filter(const char * func)
{
	func_filter_num++;
	func_filter = (char **) realloc(func_filter,
			sizeof(char*) * func_filter_num);
	func_filter[func_filter_num - 1] = strdup(func);
}

/**************************************************
 return 1 if filter is found
 **************************************************/
static int is_allowed_file(const char * file)
{
	int i;

	if (file_filter == nullptr)
	{
		return 1;
	}

	for (i = 0; i < file_filter_num; i++)
	{
		if (!strcmp(file_filter[i], file))
		{
			return 1;
		}
	}

	return 0;
}

/**************************************************
 return 1 if filter is found
 **************************************************/
static int is_allowed_func(const char * func)
{
	int i;

	if (func_filter == nullptr)
	{
		return 1;
	}

	for (i = 0; i < func_filter_num; i++)
	{
		if (!strcmp(func_filter[i], func))
		{
			return 1;
		}
	}

	return 0;
}

/**************************************************
 **************************************************/
void log_print(int type, const char * file, const char * func, int line,
		FILE *stream, int level, const char * format, ...)
{
	va_list ap;
	char buf[10000];

	if (level > log_level)
	{
		return;
	}

	if (!is_allowed_file(file))
	{
		return;
	}

	if (!is_allowed_func(func))
	{
		return;
	}

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	if (type == TYPEERR)
	{
		fprintf(stream, "\e[31m");
	}

	if (log_level == LOGDEBUG)
	{
		fprintf(stream, "%09d|%ld|%s(%d):%s|%s", SDL_GetTicks(), SDL_ThreadID(),
				file, line, func, buf);
	}
	else if (log_level == LOGDEV)
	{
		fprintf(stream, "%09d|%s", SDL_GetTicks(), buf);
	}
	else if (log_level == LOGUSER)
	{
		fprintf(stream, "%s", buf);
	}

	if (type == TYPEERR)
	{
		fprintf(stream, "\e[0m");
	}

	fprintf(stream, "\n");
}

/**************************************************
 **************************************************/
void log_set_level(char * log)
{
	if (log == nullptr)
	{
		return;
	}

	if (!strcmp(log, "0") || !strcasecmp(log, "user"))
	{
		log_level = LOGUSER;
		return;
	}
	if (!strcmp(log, "1") || !strcasecmp(log, "dev"))
	{
		log_level = LOGDEV;
		return;
	}
	if (!strcmp(log, "2") || !strcasecmp(log, "debug"))
	{
		log_level = LOGDEBUG;
		return;
	}
}

#ifdef __cplusplus
}
#endif

#endif // LOG_C
