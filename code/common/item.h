/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2019 carabobz@gmail.com

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

#ifndef ITEM_H
#define ITEM_H

#include "Context.h"
#include <string>
#include <utility>

std::pair<bool, std::string> item_create_empty();
std::pair<bool, std::string> item_create_from_template(const std::string & my_template);
int item_destroy(const std::string & item_id);
std::pair<bool, std::string> resource_new(const std::string & my_template, int quantity);
char * item_is_resource(const std::string & item_id);
int resource_get_quantity(const std::string & item_id);
int resource_set_quantity(Context * context, const std::string & item_id, int quantity);
char * item_get_name(const std::string & item_id);

#endif
