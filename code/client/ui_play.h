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

#include "item.h"

#define UI_MAIN         0
#define UI_INVENTORY    1
#define UI_SPEAK        2

void ui_play_set(int ui_type);
int ui_play_get();
char * ui_play_get_last_action();
void ui_play_cb_action(void * arg);
void ui_play_compose(Context * ctx, item_t * item_list);
void ui_play_init();
void ui_play_popup_add(const std::vector<std::string> & data);
