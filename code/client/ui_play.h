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

#ifndef UI_PLAY_H
#define UI_PLAY_H

#include <string>
#include <vector>

class Context;
class SdlItem;

enum class UiType
{
	MAIN, INVENTORY, POPUP
};

void ui_play_set(const UiType type);
UiType ui_play_get();
const std::string & ui_play_get_last_action();
void ui_play_cb_action(const std::string & action);
void ui_play_compose(Context * ctx, std::vector<SdlItem*> & itemArray);
void ui_play_init();
void ui_play_popup_add(const std::vector<std::string> & data);

#endif
