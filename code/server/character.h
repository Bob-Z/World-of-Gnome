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

#include "Context.h"
#include <string>
#include <utility>

void character_playable_send_list(Connection & connection);
void character_user_send(Connection & connection, const std::string & id);
void character_user_send_list(Connection & connection);
int character_out_of_game(const char * id);
int character_disconnect(const char * id);
int character_set_pos(Context * ctx, const std::string & map, int x, int y);
std::pair<bool, std::string> character_create_from_template(Context * ctx, const char * my_template, const char * map, int layer, int x, int y);
void character_update_aggro(Context * context);
int character_set_npc(const char * ctx, int npc);
int character_get_npc(const std::string & id);
char * character_get_portrait(const char * id);
int character_set_portrait(const char * id, const char * portrait);
int character_set_ai_script(const char * id, const char * script_name);
int character_wake_up(const char * id);
int character_set_sprite(const char * id, int index, const char * filename);
int character_set_sprite_dir(const std::string & id, const std::string & dir, int index, const std::string & filename);
int character_set_sprite_move(const char * id, const char * dir, int index, const char * filename);
void character_broadcast(const char * character);

