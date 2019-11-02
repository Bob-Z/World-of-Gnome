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

#ifndef SCREEN_H
#define SCREEN_H

#include "item.h"

class Camera;

enum class Screen
{
	SELECT, CREATE, PLAY, LAST
};

static constexpr int MUSIC_CHANNEL = 0;

void screen_display(Context * ctx);
void screen_compose();
void screen_set_screen(Screen p_Screen);
void screen_quit();
Screen screen_get_current_screen();
Camera * screen_get_camera();

#endif // SCREEN_H

