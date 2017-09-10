/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2017 carabobz@gmail.com

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

#include "common.h"
#include <SDL2/SDL_mixer.h>
#include <string>

static const int LOOP = -1;
static const int NO_LOOP = 0;
static const int ANY_CHANNEL = -1;

int sfx_play(context_t* p_Ctx, const std::string & p_FileName, int p_Channel,
		int p_Loops);
void sfx_stop(int p_Channel);
void sfx_set_volume(int p_Channel, int p_VolumePerCent);

#endif
