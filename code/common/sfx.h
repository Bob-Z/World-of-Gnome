/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2017-2020 carabobz@gmail.com

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

#ifndef SFX_H
#define SFX_H

#include <SDL2/SDL_mixer.h>
#include <string>

static const int LOOP = -1;
static const int NO_LOOP = 0;
static const int ANY_CHANNEL = -1;

int sfx_play(Context* ctx, const std::string & fileName, int channel, int loops);
void sfx_stop(int channel);
void sfx_set_volume(int channel, int volumePerCent);

#endif
