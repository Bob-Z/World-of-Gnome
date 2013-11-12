/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013 carabobz@gmail.com

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

#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include "../common/common.h"
#include "character.h"
#include "equipment.h"
#include "inventory.h"
#include "attribute.h"
#include "item.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define LUAVM_CONTEXT "wog_context"

extern context_t * context_list_start;

/* LUA script functions */
static int l_player_get_id( lua_State* L)
{
	context_t * context;

	lua_getglobal(L,LUAVM_CONTEXT);
	context = lua_touserdata(L, -1);
	lua_pop(L,1);
	lua_pushstring(L, context->id);
	return 1;  /* number of results */
}

static int l_character_create_from_template( lua_State* L)
{
	const gchar * character;
	gchar * res;

	character = luaL_checkstring(L, -1);
	res = character_create_from_template(character);
	lua_pushstring(L, res);
        if( res)
                g_free(res);
	return 1;  /* number of results */
}

static int l_character_get_selected_map_tile_x( lua_State* L)
{
	context_t * target;
	const gchar * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		g_message("%s: Cannot find context with ID %s",__func__,id);
		return 0;  /* number of results */
	}
	lua_pushnumber(L, target->selection.map_coord[0]);
	return 1;  /* number of results */
}

static int l_character_get_selected_map_tile_y( lua_State* L)
{
	context_t * target;
	const gchar * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		g_message("%s: Cannot find context with ID %s",__func__,id);
		return 0;  /* number of results */
	}
	lua_pushnumber(L, target->selection.map_coord[1]);
	return 1;  /* number of results */
}

static int l_character_get_selected_map( lua_State* L)
{
	context_t * target;
	const gchar * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		g_message("%s: Cannot find context with ID %s",__func__,id);
		return 0;  /* number of results */
	}
	lua_pushstring(L, target->selection.map);
	return 1;  /* number of results */
}

static int l_character_get_selected_inventory_id( lua_State* L)
{
	context_t * target;
	const gchar * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		g_message("%s: Cannot find context with ID %s",__func__,id);
		return 0;  /* number of results */
	}
	lua_pushstring(L, target->selection.inventory);
	return 1;  /* number of results */
}

static int l_character_get_selected_equipment_slot( lua_State* L)
{
	context_t * target;
	const gchar * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		g_message("%s: Cannot find context with ID %s",__func__,id);
		return 0;  /* number of results */
	}
	lua_pushstring(L, target->selection.equipment);
	return 1;  /* number of results */
}

static int l_character_get_selected_character_id( lua_State* L)
{
	context_t * target;
	const gchar * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		g_message("%s: Cannot find context with ID %s",__func__,id);
		return 0;  /* number of results */
	}
	lua_pushstring(L, target->selection.id);
	return 1;  /* number of results */
}

static int l_character_get_map( lua_State* L)
{
	context_t * target;
	const gchar * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		g_message("%s: Cannot find context with ID %s",__func__,id);
		return 0;  /* number of results */
	}
	lua_pushstring(L, target->map);
	return 1;  /* number of results */
}

static int l_character_get_map_x( lua_State* L)
{
	context_t * target;
	const gchar * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		g_message("%s: Cannot find context with ID %s",__func__,id);
		return 0;  /* number of results */
	}
	lua_pushnumber(L, target->map_x);
	return 1;  /* number of results */
}

static int l_character_get_map_y( lua_State* L)
{
	context_t * target;
	const gchar * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		g_message("%s: Cannot find context with ID %s",__func__,id);
		return 0;  /* number of results */
	}
	lua_pushnumber(L, target->map_y);
	return 1;  /* number of results */
}

static int l_character_get_x( lua_State* L)
{
	context_t * target;
	const gchar * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		g_message("%s: Cannot find context with ID %s",__func__,id);
		return 0;  /* number of results */
	}
	lua_pushnumber(L, target->pos_x);
	return 1;  /* number of results */
}

static int l_character_get_y( lua_State* L)
{
	context_t * target;
	const gchar * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		g_message("%s: Cannot find context with ID %s",__func__,id);
		return 0;  /* number of results */
	}
	lua_pushnumber(L, target->pos_y);
	return 1;  /* number of results */
}

static int l_character_get_name( lua_State* L)
{
	context_t * target;
	const gchar * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		g_message("%s: Cannot find context with ID %s",__func__,id);
		return 0;  /* number of results */
	}
	lua_pushstring(L, target->character_name);
	return 1;  /* number of results */
}

static int l_character_get_type( lua_State* L)
{
	context_t * target;
	const gchar * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		g_message("%s: Cannot find context with ID %s",__func__,id);
		return 0;  /* number of results */
	}
	lua_pushstring(L, target->type);
	return 1;  /* number of results */
}

static int l_character_disconnect( lua_State* L)
{
	const gchar * id;
	gint res;

	id = luaL_checkstring(L, -1);
	res = character_disconnect(id);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

static int l_character_delete( lua_State* L)
{
	const gchar * id;
	gint res;

	id = luaL_checkstring(L, -1);
	res = entry_destroy(id);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

static int l_character_set_pos( lua_State* L)
{
	const gchar * id;
	const gchar * map;
	gint x;
	gint y;
	gint res;
	context_t * ctx;

	id = luaL_checkstring(L, -4);
	map = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);

	ctx = context_find(id);
	res = character_set_pos(ctx,map,x,y);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

static int l_character_set_npc( lua_State* L)
{
	const gchar * id;
	gint npc;
	gint res;

	id = luaL_checkstring(L, -2);
	npc = luaL_checkint(L, -1);

	res = character_set_npc(id,npc);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

static int l_map_new( lua_State* L)
{
        gchar * map_name;
        gint x;
        gint y;
	gint tile_x;
	gint tile_y;
	const gchar * default_tile;

        x = luaL_checkint(L, -5);
        y = luaL_checkint(L, -4);
        tile_x = luaL_checkint(L, -3);
        tile_y = luaL_checkint(L, -2);
	default_tile = luaL_checkstring(L, -1);
        map_name = map_new(x,y,tile_x,tile_y,(gchar *)default_tile);
        lua_pushstring(L, map_name);
	g_free(map_name);
        return 1;  /* number of results */
}

static int l_map_set_tile( lua_State* L)
{
	const gchar * map;
	const gchar * tile;
	gint x;
	gint y;
	gint res;

	map = luaL_checkstring(L, -4);
	tile = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_set_tile(map,tile,x,y);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

static int l_print_text_id( lua_State* L)
{
	const gchar * id;
	const gchar * string;
	gchar * buf;

	id = luaL_checkstring(L, -2);
	string = luaL_checkstring(L, -1);
	/* add a trailing \n */
	buf = g_strconcat(string,"\n",NULL);
	network_send_text(id,buf);
	g_free(buf);
	return 0;  /* number of results */
}

static int l_print_text_map( lua_State* L)
{
	const gchar * map;
	const gchar * string;
	gchar * buf;

	map = luaL_checkstring(L, -2);
	string = luaL_checkstring(L, -1);
	/* add a trailing \n */
	buf = g_strconcat(string,"\n",NULL);
	context_broadcast_text(map,buf);
	g_free(buf);
	return 0;  /* number of results */
}

static int l_print_text_server( lua_State* L)
{
	const gchar * string;
	gchar * buf;

	string = luaL_checkstring(L, -1);
	/* add a trailing \n */
	buf = g_strconcat(string,"\n",NULL);
	context_broadcast_text(NULL,buf);
	g_free(buf);
	return 0;  /* number of results */
}

static int l_map_add_item( lua_State* L)
{
	const gchar * map;
	const gchar * item;
	gint x;
	gint y;
	gint res;

	map = luaL_checkstring(L, -4);
	item = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_add_item(map,item,x,y);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

static int l_map_delete_item( lua_State* L)
{
	const gchar * map;
	gint x;
	gint y;
	gchar * res;

	map = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_delete_item(map,x,y);
	lua_pushstring(L, res);
	if(res) {
		g_free(res);
	}
	return 1;  /* number of results */
}

static int l_map_add_event( lua_State* L)
{
	const gchar * map;
	const gchar * script;
	gint x;
	gint y;
	gchar *res;

	map = luaL_checkstring(L, -4);
	script = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_add_event(map,script,x,y);
	lua_pushstring(L, res);
	if(res) {
		g_free(res);
	}
	return 1;  /* number of results */
}

static int l_map_add_event_param( lua_State* L)
{
	const gchar * map;
	const gchar * event_id;
	const gchar * param;
	gint res;

	map = luaL_checkstring(L, -3);
	event_id = luaL_checkstring(L, -2);
	param = luaL_checkstring(L, -1);
	res = map_add_event_param(map,event_id,param);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

static int l_map_delete_event( lua_State* L)
{
	const gchar * map;
	const gchar * script;
	gint x;
	gint y;
	gint res;

	map = luaL_checkstring(L, -4);
	script = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_delete_event(map,script,x,y);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

static int l_inventory_delete( lua_State* L)
{
	const gchar * id;
	const gchar * item;
	gint res;

	id = luaL_checkstring(L, -2);
	item = luaL_checkstring(L, -1);
	res = inventory_delete(id,item);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

static int l_inventory_add( lua_State* L)
{
	const gchar * id;
	const gchar * item;
	gint res;

	id = luaL_checkstring(L, -2);
	item = luaL_checkstring(L, -1);
	res = inventory_add(id,item);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

static int l_inventory_count( lua_State* L)
{
	const gchar * id;
	const gchar * item;
	gint res;

	id = luaL_checkstring(L, -2);
	item = luaL_checkstring(L, -1);
	res = inventory_count(id,item);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

static int l_inventory_get_by_name( lua_State* L)
{
	const gchar * id;
	const gchar * item_name;
	gchar * res;

	id = luaL_checkstring(L, -2);
	item_name = luaL_checkstring(L, -1);
	res = inventory_get_by_name(id,item_name);
	lua_pushstring(L, res);
        if( res)
                g_free(res);
	return 1;  /* number of results */
}

static int l_item_create_empty( lua_State* L)
{
	gchar * res;

	res = item_create_empty();
	lua_pushstring(L, res);
        if( res)
                g_free(res);
	return 1;  /* number of results */
}

static int l_item_create_from_template( lua_State* L)
{
	const gchar * item;
	gchar * res;

	item = luaL_checkstring(L, -1);
	res = item_create_from_template(item);
	lua_pushstring(L, res);
        if( res)
                g_free(res);
	return 1;  /* number of results */
}

static int l_item_destroy( lua_State* L)
{
	const gchar * item;
	gint res;

	item = luaL_checkstring(L, -1);
	res = item_destroy(item);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

static int l_map_get_tile( lua_State* L)
{
	const gchar * map;
	gint x;
	gint y;
	gchar * res;

	map = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_get_tile(map,x,y);
	lua_pushstring(L, res);
	if( res) 
		g_free(res);
	return 1;  /* number of results */
}

static int l_map_get_tile_type( lua_State* L)
{
	const gchar * map;
	gint x;
	gint y;
	const gchar * res;

	map = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_get_tile_type(map,x,y);
	lua_pushstring(L, res);
	return 1;  /* number of results */
}

static int l_character_attribute_change( lua_State* L)
{
	const gchar * id;
	const gchar * attribute;
	gint value;
	gint res;
	context_t * context;

	id = luaL_checkstring(L, -3);
	attribute = luaL_checkstring(L, -2);
	value = luaL_checkint(L, -1);

	lua_getglobal(L,LUAVM_CONTEXT);
	context = lua_touserdata(L, -1);
	lua_pop(L,1);

	res = attribute_change(context,id,attribute,value);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

static int l_character_attribute_get( lua_State* L)
{
	const gchar * id;
	const gchar * attribute;
	gint res;

	id = luaL_checkstring(L, -2);
	attribute = luaL_checkstring(L, -1);
	res = attribute_get(id,attribute);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

static int l_character_attribute_set( lua_State* L)
{
	const gchar * id;
	const gchar * attribute;
	gint value;
	gint res;

	id = luaL_checkstring(L, -3);
	attribute = luaL_checkstring(L, -2);
	value = luaL_checkint(L, -1);
	res = attribute_set(id,attribute,value);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

static int l_equipment_slot_delete_item( lua_State* L)
{
	const gchar * id;
	const gchar * slot;
	gint res;

	id = luaL_checkstring(L, -2);
	slot = luaL_checkstring(L, -1);
	res = equipment_delete(id,slot);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

static int l_equipment_slot_add_item( lua_State* L)
{
	const gchar * id;
	const gchar * slot;
	const gchar * item;
	gint res;

        id = luaL_checkstring(L, -3);
        slot = luaL_checkstring(L, -2);
	item = luaL_checkstring(L, -1);
	res = equipment_add(id,slot,item);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

static int l_equipment_slot_get_item_id( lua_State* L)
{
        const gchar * id;
        const gchar * slot;
	const gchar * item;

        id = luaL_checkstring(L, -2);
        slot = luaL_checkstring(L, -1);
	item = equipment_get_item_id(id,slot);
	if( item == NULL ) {
		return 0;  /* number of results */
	}
	lua_pushstring(L, item);
	return 1;  /* number of results */
}

void register_lua_functions(context_t * context)
{
	lua_State* L = context->luaVM;

	/* player func */
	lua_pushcfunction(L, l_player_get_id);
	lua_setglobal(L, "player_get_id");
	/* character func */
	lua_pushcfunction(L, l_character_create_from_template);
	lua_setglobal(L, "character_create_from_template");
	lua_pushcfunction(L, l_character_get_selected_map);
	lua_setglobal(L, "character_get_selected_map");
	lua_pushcfunction(L, l_character_get_selected_map_tile_x);
	lua_setglobal(L, "character_get_selected_map_tile_x");
	lua_pushcfunction(L, l_character_get_selected_map_tile_y);
	lua_setglobal(L, "character_get_selected_map_tile_y");
	lua_pushcfunction(L, l_character_get_selected_inventory_id);
	lua_setglobal(L, "character_get_selected_inventory_id");
	lua_pushcfunction(L, l_character_get_selected_equipment_slot);
	lua_setglobal(L, "character_get_selected_equipment_slot");
	lua_pushcfunction(L, l_character_get_selected_character_id);
	lua_setglobal(L, "character_get_selected_character_id");
	lua_pushcfunction(L, l_character_get_map);
	lua_setglobal(L, "character_get_map");
	lua_pushcfunction(L, l_character_get_map_x);
	lua_setglobal(L, "character_get_map_x");
	lua_pushcfunction(L, l_character_get_map_y);
	lua_setglobal(L, "character_get_map_y");
	lua_pushcfunction(L, l_character_get_x);
	lua_setglobal(L, "character_get_x");
	lua_pushcfunction(L, l_character_get_y);
	lua_setglobal(L, "character_get_y");
	lua_pushcfunction(L, l_character_get_name);
	lua_setglobal(L, "character_get_name");
	lua_pushcfunction(L, l_character_get_type);
	lua_setglobal(L, "character_get_type");
	lua_pushcfunction(L, l_character_set_pos);
	lua_setglobal(L, "character_set_pos");
	lua_pushcfunction(L, l_character_set_npc);
	lua_setglobal(L, "character_set_npc");
	lua_pushcfunction(L, l_character_disconnect);
	lua_setglobal(L, "character_disconnect");
	lua_pushcfunction(L, l_character_delete);
	lua_setglobal(L, "character_delete");
	/* map func */
	lua_pushcfunction(L, l_map_new);
	lua_setglobal(L, "map_new");
	lua_pushcfunction(L, l_map_set_tile);
	lua_setglobal(L, "map_set_tile");
	lua_pushcfunction(L, l_map_add_item);
	lua_setglobal(L, "map_add_item");
	lua_pushcfunction(L, l_map_delete_item);
	lua_setglobal(L, "map_delete_item");
	lua_pushcfunction(L, l_map_get_tile);
	lua_setglobal(L, "map_get_tile");
	lua_pushcfunction(L, l_map_get_tile_type);
	lua_setglobal(L, "map_get_tile_type");
	lua_pushcfunction(L, l_map_add_event);
	lua_setglobal(L, "map_add_event");
	lua_pushcfunction(L, l_map_add_event_param);
	lua_setglobal(L, "map_add_event_param");
	lua_pushcfunction(L, l_map_delete_event);
	lua_setglobal(L, "map_delete_event");
	/* inventory func */
	lua_pushcfunction(L, l_inventory_delete);
	lua_setglobal(L, "inventory_delete");
	lua_pushcfunction(L, l_inventory_add);
	lua_setglobal(L, "inventory_add");
	lua_pushcfunction(L, l_inventory_count);
	lua_setglobal(L, "inventory_count");
	lua_pushcfunction(L, l_inventory_get_by_name);
	lua_setglobal(L, "inventory_get_by_name");
	/* item func */
	lua_pushcfunction(L, l_item_create_empty);
	lua_setglobal(L, "item_create_empty");
	lua_pushcfunction(L, l_item_create_from_template);
	lua_setglobal(L, "item_create_from_template");
	lua_pushcfunction(L, l_item_destroy);
	lua_setglobal(L, "item_destroy");
	/* character attribute func */
	lua_pushcfunction(L, l_character_attribute_change);
	lua_setglobal(L, "character_attribute_change");
	lua_pushcfunction(L, l_character_attribute_get);
	lua_setglobal(L, "character_attribute_get");
	lua_pushcfunction(L, l_character_attribute_set);
	lua_setglobal(L, "character_attribute_set");
	/* equipment func */
	lua_pushcfunction(L, l_equipment_slot_delete_item);
	lua_setglobal(L, "equipment_slot_delete_item");
	lua_pushcfunction(L, l_equipment_slot_add_item);
	lua_setglobal(L, "equipment_slot_get_add_item");
	lua_pushcfunction(L, l_equipment_slot_get_item_id);
	lua_setglobal(L, "equipment_slot_get_item_id");
	/* misc func */
	lua_pushcfunction(L, l_print_text_id);
	lua_setglobal(L, "print_text_id");
	lua_pushcfunction(L, l_print_text_map);
	lua_setglobal(L, "print_text_map");
	lua_pushcfunction(L, l_print_text_server);
	lua_setglobal(L, "print_text_server");

	/* push the context on lua VM stack */
	lua_pushlightuserdata(L,context);
	lua_setglobal (L, LUAVM_CONTEXT);
}

static void action_chat(context_t * context, const gchar * text)
{
	gchar * new_text;

	new_text = g_strconcat(context->character_name,": ",text,"\n",NULL);
	network_broadcast_text(context,new_text);
	g_free(new_text);
}

/**************************************
return -1 if the script do not return something
**************************************/

gint action_execute_script(context_t * context, const gchar * script, gchar ** parameters)
{
	gchar * filename;
	gchar * previous_parameter;
	gchar parameter_name[SMALL_BUF];
	gint i;
	gint return_value;

	/* Special case for chat */
	if( g_strcmp0(script,WOG_CHAT)==0) {
		action_chat(context,parameters[0]);
		return -1;
	}

	filename = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", SCRIPT_TABLE, "/", script, NULL);
	/* push parameters on lua VM stack (only strings paramters are supported) */
	if(parameters != NULL ) {
		previous_parameter = parameters[0];	
		for(i=0;i<MAX_PARAMETER;i++) {
			g_sprintf(parameter_name,"parameter%d",i);
			if(previous_parameter == NULL ) {
				lua_pushstring(context->luaVM,NULL);
				lua_setglobal (context->luaVM, parameter_name);
			}
			else {
				lua_pushstring(context->luaVM,parameters[i]);
				lua_setglobal (context->luaVM, parameter_name);
				previous_parameter = parameters[i];
			}
		}
	}

	if (luaL_loadfile(context->luaVM, filename) != 0 ) {
		/* If something went wrong, error message is at the top of */
		/* the stack */
		g_message("Couldn't load LUA script %s: %s\n", filename, lua_tostring(context->luaVM, -1));
		return -1;
	}

	/* Ask Lua to run the script */
	if (lua_pcall(context->luaVM, 0, LUA_MULTRET, 0) != 0) {
		g_message("Failed to run LUA script %s: %s\n", filename, lua_tostring(context->luaVM, -1));
		return -1;
	}

	g_free(filename);

	/* retrieve result */
	if (!lua_isnumber(context->luaVM, -1)) {
		return -1;
	}
	return_value = lua_tonumber(context->luaVM, -1);
	return return_value;
}
