/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2016-2020 carabobz@gmail.com

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

#include "Context.h"
#include "font.h"

#include "client_server.h"
#include "file.h"
#include "list.h"
#include <string>

static list_t * font_list = nullptr;

/****************************************
 *****************************************/
TTF_Font * font_get(Context* ctx, const std::string & filename, int size)
{
	TTF_Font * font = nullptr;

	file_lock(filename.c_str());

	font = (TTF_Font*) list_find(font_list, filename.c_str());

	if (font != nullptr)
	{
		file_unlock(filename.c_str());
		return font;
	}

	const std::string file_path = base_directory + "/" + filename;
	font = TTF_OpenFont(file_path.c_str(), size);

	if (font != nullptr)
	{
		list_update(&font_list, filename.c_str(), font);
		file_unlock(filename.c_str());
		return font;
	}

	file_update(ctx->getConnection(), filename.c_str());

	file_unlock(filename.c_str());

	return nullptr;
}
