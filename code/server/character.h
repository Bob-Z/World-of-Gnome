/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2017 carabobz@gmail.com

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

void character_playable_send_list(context_t * context);
void character_user_send(context_t * p_pCtx, const char * p_pCharacterId);
void character_user_send_list(context_t * context);
int character_out_of_game(const char * id);
int character_disconnect(const char * id);
int character_set_pos(context_t * ctx,const char * map, int x, int y);
char * character_create_from_template(context_t * ctx,const char * my_template,const char * map, int layer, int x, int y);
void character_update_aggro(context_t * context);
int character_set_npc(const char * ctx, int npc);
int character_get_npc(const char * id);
char * character_get_portrait(const char * id);
int character_set_portrait(const char * id,const char * portrait);
int character_set_ai_script(const char * id, const char * script_name);
int character_wake_up(const char * id);
int character_set_sprite(const char * id, int index, const char * filename);
int character_set_sprite_dir(const char * id, const char * dir, int index, const char * filename);
int character_set_sprite_move(const char * id, const char * dir, int index, const char * filename);
void character_broadcast(const char * character);

