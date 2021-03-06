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

#ifndef NETWORK_SERVER
#define NETWORK_SERVER

#include "Context.h"
#include <string>
#include <vector>

#include "../client/EffectManager.h"

void network_broadcast_text(Context * context, const std::string & text);
void network_send_user_character(Connection & connection, const std::string & id, const std::string & typr, const std::string & name);
void network_send_character_file(Context * context);
void network_broadcast_entry_int(const char * table, const char * file, const char * path, int value, bool same_map_only);
void network_init(void);
void network_send_popup(const std::string & p_rCtxId, const std::vector<std::string> & p_rPopupData);
void network_broadcast_effect(EffectManager::EffectType p_Type, const std::string & p_TargetId, const std::vector<std::string> & p_Param);
void network_send_login_ok(Connection & connection);
void network_send_login_nok(Connection & connection);
void network_send_playable_character(Connection & connection, const std::vector<std::string> & id_list);
void network_send_context_to_context(Context * dest_ctx, Context * src_ctx);
void network_send_text(const std::string & id, const std::string & string);
void network_send_entry_int(Context * context, const char * table, const char * file, const char *path, int value);

#endif
