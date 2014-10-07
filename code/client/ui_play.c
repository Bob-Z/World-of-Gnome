/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2014 carabobz@gmail.com

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

#include "../common/common.h"
#include "../sdl_item/anim.h"
#include "../sdl_item/item.h"
#include "../sdl_item/sdl.h"
#include "network_client.h"
#include "imageDB.h"
#include "screen.h"
#include "scr_play.h"
#include "textview.h"

#define UI_MAIN		0
#define UI_INVENTORY	1
#define UI_SPEAK	2

#define FONT "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-C.ttf"
#define FONT_SIZE 30
#define TEXT_FONT "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-C.ttf"
#define TEXT_FONT_SIZE 15
#define TEXT_TIMEOUT 5000 /* Text display timeout */
#define ITEM_FONT "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-C.ttf"
#define ITEM_FONT_SIZE 15
#define SPEAK_FONT "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-C.ttf"
#define SPEAK_FONT_SIZE 32

#define BACKGROUND_COLOR (0x000000C0)

static int current_ui = UI_MAIN;
static char * last_action_script = NULL;
/* main ui */
static char ** attribute_string = NULL;
static int action_bar_height;
static int attribute_height;
static char text_buffer[2048];
/* inventory ui */
static char ** inventory_list = NULL;
/* speak ui */
static char * speaker_id = NULL;
static char * speaker_name = NULL;
static char * speaker_text = NULL;
static speak_entry_t * speech = NULL;
static int speech_num = 0;

/**********************************
**********************************/
static void draw_background(context_t * ctx, item_t * item_list)
{
	static anim_t * bg_anim = NULL;
	int sw;
	int sh;
	item_t * item;

	if( bg_anim ) {
		si_anim_free(bg_anim);
	}
	SDL_GetRendererOutputSize(ctx->render,&sw,&sh);
	bg_anim = anim_create_color(ctx->render, sw, sh, BACKGROUND_COLOR);
	item = item_list_add(&item_list);
	item_set_anim(item,0,0,bg_anim);
	item_set_overlay(item,1);
}

/**********************************
**********************************/
void ui_play_set(int ui_type)
{
	current_ui = ui_type;
	screen_compose();
}

/**********************************
**********************************/
int ui_play_get()
{
	return current_ui;
}

/**********************************
**********************************/
char * ui_play_get_last_action_script()
{
	return last_action_script;
}

/****************************
****************************/
static void cb_main_quit(void * arg)
{
	context_t * current_ctx;
	context_t * next_ctx;

        if( ui_play_get() == UI_MAIN ) {
		context_set_in_game(context_get_player(),false);
		network_send_context(context_get_player());
		current_ctx = context_get_first();
		while( current_ctx != NULL ) {
			/* Save next before freeing the current context */
			next_ctx = current_ctx->next;
			if( current_ctx != context_get_player() ) {
				context_free(current_ctx);
			}
			current_ctx = next_ctx;
		}

		scr_play_init(true);
                screen_set_screen(SCREEN_SELECT);
        }
}

/****************************
****************************/
static void key_up(void * arg)
{
        context_t * ctx = context_get_player();

        network_send_action(ctx,"move_up.lua",NULL);
}

/**************************************
**************************************/
static void key_down(void * arg)
{
        context_t * ctx = context_get_player();

        network_send_action(ctx,"move_down.lua",NULL);
}

/**************************************
**************************************/
static void key_left(void * arg)
{
        context_t * ctx = context_get_player();

        network_send_action(ctx,"move_left.lua",NULL);
}

/**************************************
**************************************/
static void key_right(void * arg)
{
        context_t * ctx = context_get_player();

        network_send_action(ctx,"move_right.lua",NULL);
}

/**********************************
Compose attribute
**********************************/
static void compose_attribute(context_t * ctx, item_t * item_list)
{
        item_t * item;
        char ** name_list;
        int index = 0;
        int value;
        int y = 0;
        int num_attr = 0;
        char buf[1024];
        int w,h;
        static TTF_Font * font = NULL;

        if ( font == NULL ) {
                font = TTF_OpenFont(FONT, FONT_SIZE);
        }
        if ( font == NULL ) {
                return;
        }

        if(attribute_string) {
                index = 0;
                while(attribute_string[index]) {
                        free(attribute_string[index]);
                        attribute_string[index]=NULL;
                        index++;
                }
                free(attribute_string);
                attribute_string=NULL;
        }

        if(!entry_get_group_list(CHARACTER_TABLE,ctx->id,&name_list,ATTRIBUTE_GROUP,NULL) ) {
                return;
        }

        index=0;
        while( name_list[index] != NULL) {
                if(!entry_read_int(CHARACTER_TABLE,ctx->id,&value,ATTRIBUTE_GROUP,name_list[index],ATTRIBUTE_CURRENT,NULL)) {
                        index++;
                        continue;
                }

                num_attr++;
                attribute_string = realloc(attribute_string, (num_attr+1)*sizeof(char*));
                sprintf(buf,"%s: %d",name_list[index],value);
                attribute_string[num_attr-1] = strdup(buf);
                attribute_string[num_attr]=NULL;

                item = item_list_add(&item_list);

                item_set_overlay(item,1);
                item_set_string(item,attribute_string[num_attr-1]);
                item_set_font(item,font);
                sdl_get_string_size(item->font,item->string,&w,&h);
                item_set_frame_shape(item,0,y,w,h);
                y+=h;
                if(attribute_height<y) {
                        attribute_height = y;
                }

                index++;
        }

        deep_free(name_list);
}

/**********************************
**********************************/
void ui_play_cb_action(void * arg)
{
        char * script = (char *)arg;

        network_send_action(context_get_player(),script,NULL);

        if( last_action_script ) {
                free(last_action_script);
                last_action_script = NULL;
        }

        if( arg ) {
                last_action_script = strdup(arg);
        } else {
                last_action_script = NULL;
        }
}

/**********************************
Compose action icon
**********************************/
static void compose_action(context_t * ctx,item_t * item_list)
{
        char ** action_list = NULL;
        char * text = NULL;
        char * icon = NULL;
        char * script = NULL;
        anim_t * anim;
        item_t * item;
        int sw = 0;
        int sh = 0;
        int x=0;
        int i;

        SDL_GetRendererOutputSize(ctx->render,&sw,&sh);

        action_bar_height = 0;

        /* Read action list for current user */
        if(!entry_read_list(CHARACTER_TABLE,ctx->id,&action_list,CHARACTER_KEY_ACTION,NULL)) {
                return;
        }

        i=0;
        while(action_list[i] != NULL ) {
                if(text) {
                        free(text);
                        text = NULL;
                }
                if(icon) {
                        free(icon);
                        icon = NULL;
                }
                if(script) {
                        free(script);
                        script = NULL;
                }

                if(!entry_read_string(ACTION_TABLE,action_list[i],&text,ACTION_KEY_TEXT,NULL)) {
                        i++;
                        continue;
                }
                if(!entry_read_string(ACTION_TABLE,action_list[i],&icon,ACTION_KEY_ICON,NULL)) {
                        i++;
                        continue;
                }
                if(!entry_read_string(ACTION_TABLE,action_list[i],&script,ACTION_KEY_SCRIPT,NULL)) {
                        i++;
                        continue;
                }

                /* load image */
                anim = imageDB_get_anim(ctx, icon);

                item = item_list_add(&item_list);

                item_set_overlay(item,1);
                item_set_anim(item,x,sh-anim->h,anim);
                x += anim->w;
                item_set_click_left(item,ui_play_cb_action,(void*)strdup(script),free);
                if( action_bar_height < anim->h ) {
                        action_bar_height = anim->h;
                }

                i++;
        }

        deep_free(action_list);

        if(text) {
                free(text);
                text = NULL;
        }
        if(icon) {
                free(icon);
                icon = NULL;
        }
        if(script) {
                free(script);
                script = NULL;
        }
}

/**********************************
**********************************/
static void cb_select_slot(void * arg)
{
        char * id = (char*)arg;
        context_t * ctx = context_get_player();

        ctx->selection.equipment = id;
        network_send_context(ctx);
}

/**************************************
**************************************/
static void show_inventory(void * arg)
{
        ui_play_set(UI_INVENTORY);
}

/**********************************
Compose equipment icon
**********************************/
static void compose_equipment(context_t * ctx, item_t * item_list)
{
        char ** name_list = NULL;
        anim_t * anim;
        anim_t * anim2;
        anim_t * anim3;
        item_t * item;
        int sw = 0;
        int sh = 0;
        int y=0;
        int x=0;
        int h1;
        int index;
        char * template = NULL;
#if 0
        char * name;
#endif
        char * icon_name = NULL;
        char * equipped_name = NULL;
#if 0
        char * equipped_text = NULL;
#endif
        char * equipped_icon_name = NULL;
        char * inventory_icon_name = NULL;
        static anim_t * inventory_icon = NULL;
        int max_h;
        int max_w;

        SDL_GetRendererOutputSize(ctx->render,&sw,&sh);

        entry_get_group_list(CHARACTER_TABLE,ctx->id,&name_list,EQUIPMENT_GROUP,NULL);

        max_w = 0;
        max_h = 0;
        index=0;
        while( name_list && name_list[index] != NULL) {
#if 0
                /* Get the slot name */
                if(!entry_read_string(CHARACTER_TABLE,ctx->id,&item_name,EQUIPMENT_GROUP,name_list[index],EQUIPMENT_NAME,NULL)) {
                        name = strdup(name_list[index]);
                } else {
                        name = item_name;
                }
                free(item_name);
#endif
                h1 = 0;
                /* Get the slot icon */
                if(!entry_read_string(CHARACTER_TABLE,ctx->id,&icon_name,EQUIPMENT_GROUP,name_list[index],EQUIPMENT_ICON,NULL)) {
                        continue;
                } else {
                        /* load image */
                        anim = imageDB_get_anim(ctx, icon_name);
                        free(icon_name);

                        item = item_list_add(&item_list);

                        x = sw-anim->w;
                        h1 = anim->h;
                        item_set_overlay(item,1);
                        item_set_anim(item,x,y,anim);

                        item_set_click_left(item,cb_select_slot,strdup(name_list[index]),NULL);

                        if(anim->w > max_w) {
                                max_w = anim->w;
                        }
                        if(anim->h > max_h) {
                                max_h = anim->h;
                        }
                }

                /* Is there an equipped object ? */
                if(entry_read_string(CHARACTER_TABLE,ctx->id,&equipped_name,EQUIPMENT_GROUP,name_list[index],EQUIPMENT_EQUIPPED,NULL) && equipped_name[0]!=0 ) {
#if 0
                        /* Get the equipped object name */
                        if(!entry_read_string(ITEM_TABLE,equipped_name,&equipped_text,ITEM_NAME,NULL)) {
                                werr(LOGDEV,"Can't read object %s name in equipment slot %s",equipped_name,name_list[index]);
                        }
                        free(equipped_text);
#endif
                        /* Get its icon */
                        if(!entry_read_string(ITEM_TABLE,equipped_name,&equipped_icon_name,ITEM_ICON,NULL)) {
                                werr(LOGDEV,"Can't read object %s icon in equipment slot %s",equipped_name,name_list[index]);
                        } else {
                                item = item_list_add(&item_list);

                                anim2 = imageDB_get_anim(ctx, equipped_icon_name);
                                free(equipped_icon_name);

                                item_set_overlay(item,1);
                                item_set_anim(item,x-anim->w,y,anim2);
                                item_set_click_left(item,cb_select_slot,strdup(name_list[index]),NULL);
                                if(h1 < anim->h) {
                                        h1 = anim->h;
                                }
                        }
                        free(equipped_name);
                }

                /* Draw selection cursor */
                if( ctx->selection.equipment != NULL) {
                        if( !strcmp(ctx->selection.equipment,name_list[index]) ) {
                                anim3 = imageDB_get_anim(ctx,CURSOR_EQUIP_FILE);

                                item = item_list_add(&item_list);

                                /* Center on icon */
                                item_set_overlay(item,1);
                                item_set_anim(item,x - (anim3->w-anim->w)/2, y - (anim3->h-anim->w)/2, anim3);
                        }
                }

                if(h1 > anim->h) {
                        y += h1;
                } else {
                        y += anim->h;
                }

                index++;
        }

        /* Draw selected item */
        if( ctx->selection.inventory != NULL) {
                template = item_is_resource(ctx->selection.inventory);

                if ( template == NULL ) {
                        if(!entry_read_string(ITEM_TABLE,ctx->selection.inventory,&inventory_icon_name,ITEM_ICON,NULL)) {
                                werr(LOGDEV,"Can't read item %s icon name",ctx->selection.inventory);
                        }
                } else {
                        if(!entry_read_string(ITEM_TEMPLATE_TABLE,template,&inventory_icon_name,ITEM_ICON,NULL)) {
                                werr(LOGDEV,"Can't read item %s icon name (template: %s)",ctx->selection.inventory,template);
                        }
                        free(template);
                }

                if( inventory_icon_name ) {
                        item = item_list_add(&item_list);

                        anim = imageDB_get_anim(ctx, inventory_icon_name);
                        free(inventory_icon_name);

                        item_set_overlay(item,1);
                        item_set_anim(item,sw-anim->w,y,anim);
                        item_set_click_left(item,show_inventory,NULL,NULL);
                }
        } else {
                if( max_w == 0) {
                        max_w = 32;
                }
                if( max_h == 0) {
                        max_h = 32;
                }

                if( inventory_icon == NULL ) {
                        inventory_icon = anim_create_color(ctx->render, max_w, max_h, 0x7f7f7f7f);
                }

                item = item_list_add(&item_list);

                item_set_overlay(item,1);
                item_set_anim(item,sw-inventory_icon->w,y,inventory_icon);
                item_set_click_left(item,show_inventory,NULL,NULL);
        }

        deep_free(name_list);
}

/**************************************
**************************************/
static void keyboard_text(void * arg)
{
        char * text = (char*)arg;

        network_send_action(context_get_player(),WOG_CHAT,text,NULL);
        text_buffer[0]=0;
        screen_compose();
}

/**********************************
Compose text
**********************************/
static void compose_text(context_t * ctx, item_t * item_list)
{
        const history_entry_t * history;
        history_entry_t * hist;
        Uint32 time = SDL_GetTicks();
        int sw;
        int sh;
        int current_y;
        static TTF_Font * font = NULL;
        item_t * item;
        int w;
        int h;
        int x;
        int y;

        if ( font == NULL ) {
                font = TTF_OpenFont(TEXT_FONT, TEXT_FONT_SIZE);
        }
        if ( font == NULL ) {
                return;
        }

        SDL_GetRendererOutputSize(ctx->render,&sw,&sh);
        current_y = sh - action_bar_height;

        /* Draw edit box */
        item = item_list_add(&item_list);

        item_set_overlay(item,1);
        item_set_string(item,text_buffer);
        item_set_string_bg(item,BACKGROUND_COLOR);
        item_set_font(item,font);
        item_set_editable(item,1);
        item_set_edit_cb(item,keyboard_text);
        sdl_get_string_size(item->font,item->string,&w,&h);
        x = w;
        if ( w < 100 ) {
                x = 100;
        }
        y = h;
        if ( y < TEXT_FONT_SIZE ) {
                y = TEXT_FONT_SIZE;
        }
        item_set_frame_shape(item,0,current_y-y,x,y);
        current_y-=y;
        if(attribute_height > current_y) {
                return;
        }

        /* Draw text history */
        history = textview_get_history();

        if ( history == NULL ) {
                return;
        }

        hist = (history_entry_t*)history;

        while(hist) {
                if ( time > hist->time + TEXT_TIMEOUT ) {
                        return;
                }

                item = item_list_add(&item_list);

                item_set_overlay(item,1);
                item_set_string(item,hist->text);
		item_set_string_bg(item,BACKGROUND_COLOR);
                item_set_font(item,font);
                sdl_get_string_size(item->font,item->string,&w,&h);
                item_set_frame_shape(item,0,current_y-h,w,h);
                current_y-=h;
                if(attribute_height > current_y) {
                        return;
                }

                hist = hist->next;
        }
}

/**********************************
**********************************/
static void main_compose(context_t * ctx, item_t * item_list) {
        compose_attribute(ctx,item_list);
        compose_action(ctx,item_list);
        compose_equipment(ctx,item_list);
        compose_text(ctx,item_list);

	sdl_add_keycb(SDL_SCANCODE_I,show_inventory,NULL,NULL);
        sdl_add_keycb(SDL_SCANCODE_UP,key_up,NULL,NULL);
        sdl_add_keycb(SDL_SCANCODE_DOWN,key_down,NULL,NULL);
        sdl_add_keycb(SDL_SCANCODE_LEFT,key_left,NULL,NULL);
        sdl_add_keycb(SDL_SCANCODE_RIGHT,key_right,NULL,NULL);
        sdl_add_keycb(SDL_SCANCODE_ESCAPE,cb_main_quit,NULL,NULL);
}

/****************************
****************************/
static void cb_inventory_quit(void * arg)
{
        ui_play_set(UI_MAIN);
}

/****************************
****************************/
void cb_inventory_select(void * arg)
{
        context_t * player = context_get_player();
        char * item_id = (char *)arg;

        if( player->selection.inventory ) {
                free( player->selection.inventory );
        }
        player->selection.inventory = strdup(item_id);
        network_send_context(player);
}

/**********************************
Compose inventory
**********************************/
static void compose_inventory(context_t * ctx,item_t * item_list)
{
        char * value = NULL;
        char * label;
        char * description;
        anim_t * anim;
        item_t * item;
        int x=0;
        int i = 0;
        static TTF_Font * font = NULL;
        char * template;
        int quantity;
        char buf[1024];

        if ( font == NULL ) {
                font = TTF_OpenFont(ITEM_FONT, ITEM_FONT_SIZE);
        }

        deep_free(inventory_list);

        /* read data from file */
        if(!entry_read_list(CHARACTER_TABLE,ctx->id,&inventory_list, CHARACTER_KEY_INVENTORY,NULL)) {
                return;
        }

	draw_background(ctx,item_list);

        while( inventory_list[i] != NULL) {
                template = item_is_resource(inventory_list[i]);

                if( template == NULL ) {
                        /* Icon is mandatory for now */
                        if(!entry_read_string(ITEM_TABLE,inventory_list[i],&value,ITEM_ICON,NULL)) {
                                i++;
                                continue;
                        }
                        /* load image */
                        anim = imageDB_get_anim(ctx, value);
                        free(value);

                        if(!entry_read_string(ITEM_TABLE,inventory_list[i],&value,ITEM_NAME,NULL)) {
                                label = strdup(inventory_list[i]);
                        } else {
                                label = value;
                        }

                        if(!entry_read_string(ITEM_TABLE,inventory_list[i],&value,ITEM_DESC,NULL)) {
                                description = strdup("");;
                        } else {
                                description = value;
                        }
                } else {
                        /* Icon is mandatory for now */
                        if(!entry_read_string(ITEM_TEMPLATE_TABLE,template,&value,ITEM_ICON,NULL)) {
                                i++;
                                free(template);
                                continue;
                        }
                        /* load image */
                        anim = imageDB_get_anim(ctx, value);
                        free(value);

                        if(!entry_read_string(ITEM_TEMPLATE_TABLE,template,&value,ITEM_NAME,NULL)) {
                                label = strdup(inventory_list[i]);
                        } else {
                                label = value;
                        }

                        if(!entry_read_string(ITEM_TEMPLATE_TABLE,template,&value,ITEM_DESC,NULL)) {
                                description = strdup("");;
                        } else {
                                description = value;
                        }
                        free(template);
                }

                quantity = resource_get_quantity(inventory_list[i]);
                sprintf(buf,"%d",quantity);

                item = item_list_add(&item_list);

                item_set_anim(item,x,0,anim);
                item_set_string(item,buf);
                item_set_font(item,font);
                item_set_overlay(item,1);

                x += anim->w;
                item_set_click_left(item,cb_inventory_select,(void*)strdup(inventory_list[i]),free);

                free(description);
                free(label);
                i++;
        }
}

/**********************************
Compose select cursor
**********************************/
static void compose_inventory_select(context_t * ctx,item_t * item_list)
{
        item_t * item;
        int x;
        int i;
	char * icon_name;
	anim_t * anim;
	anim_t * icon_anim;
	char * template;

        if(ctx->selection.inventory == NULL) {
                return;
        }
        if(ctx->selection.inventory[0] == 0) {
                return;
        }

        if(item_list == NULL) {
                return;
        }

        anim = imageDB_get_anim(ctx,CURSOR_SPRITE_FILE);

        deep_free(inventory_list);

        /* read data from file */
        if(!entry_read_list(CHARACTER_TABLE,ctx->id,&inventory_list, CHARACTER_KEY_INVENTORY,NULL)) {
                return;
        }

        i = 0;
        x = 0;
        while( inventory_list[i] && strcmp(inventory_list[i],ctx->selection.inventory) ) {
                template = item_is_resource(inventory_list[i]);

                if ( template == NULL ) {
                        if(!entry_read_string(ITEM_TABLE,inventory_list[i],&icon_name,ITEM_ICON,NULL)) {
                                werr(LOGDEV,"Can't read item %s icon name",inventory_list[i]);
                        }
                } else {
                        if(!entry_read_string(ITEM_TEMPLATE_TABLE,template,&icon_name,ITEM_ICON,NULL)) {
                                werr(LOGDEV,"Can't read item %s icon name (template: %s)",inventory_list[i],template);
                        }
                        free(template);
                }

		icon_anim = imageDB_get_anim(ctx,icon_name);
		free(icon_name);

                x += icon_anim->w;
                i++;
        }

        if(inventory_list[i]) {
                item = item_list_add(&item_list);
                item_set_anim(item,x,0,anim);
                item_set_overlay(item,1);
        }
}

/**********************************
**********************************/
static void inventory_compose(context_t * ctx, item_t * item_list) {
	compose_inventory(ctx,item_list);
        compose_inventory_select(ctx,item_list);

        sdl_add_keycb(SDL_SCANCODE_I,cb_inventory_quit,NULL,NULL);
        sdl_add_keycb(SDL_SCANCODE_ESCAPE,cb_inventory_quit,NULL,NULL);
        sdl_add_keycb(SDL_SCANCODE_SPACE,cb_inventory_quit,NULL,NULL);
}

/****************************
****************************/
static void speak_cleanup()
{
        int i;

        if( speaker_id ) {
                free(speaker_id);
                speaker_id = NULL;
        }
        if( speaker_name ) {
                free(speaker_name);
                speaker_name = NULL;
        }
        if( speaker_text ) {
                free(speaker_text);
                speaker_text = NULL;
        }

        if( speech ) {
                for(i=0;i<speech_num;i++) {
                        free(speech[i].icon);
                        free(speech[i].text);
                        free(speech[i].keyword);
                }
                free(speech);
                speech = NULL;
        }

        speech_num = 0;
}

/****************************
****************************/
void cb_speak(void * arg)
{
        context_t * player = context_get_player();
        char * keyword = (char *)arg;
        char * speak_script = NULL;

        if(!entry_read_string(CHARACTER_TABLE, player->id,&speak_script,CHARACTER_KEY_SPEAK,NULL)) {
                return;
        }

        network_send_action(player, speak_script,speaker_id,keyword,NULL);

	free(speak_script);
}

/**********************************
Compose screen
**********************************/
static void compose_speak(context_t * ctx,item_t * item_list)
{
        item_t * item;
        int i = 0;
        int x = 0;
        int y = 0;
        static TTF_Font * font = NULL;
        int w;
        int h;
        int max_h;
        anim_t * anim;

	draw_background(ctx,item_list);

        if ( font != NULL ) {
                TTF_CloseFont(font);
                font = NULL;
        }

        if( speech && speech[0].icon ) {
                anim = imageDB_get_anim(ctx,speech[i].icon);
                font = TTF_OpenFont(FONT, anim->h);
        }
        else {
                font = TTF_OpenFont(SPEAK_FONT, SPEAK_FONT_SIZE );
        }

        if( speaker_name ) {
                item = item_list_add(&item_list);
                item_set_string(item,speaker_name);
                item_set_font(item,font);
                sdl_get_string_size(item->font,item->string,&w,&h);
                item_set_frame_shape(item,0,y,w,h);
		item_set_overlay(item,1);
                y += h;
        }

        item = item_list_add(&item_list);
        item_set_string(item,speaker_text);
        item_set_font(item,font);
        sdl_get_string_size(item->font,item->string,&w,&h);
        item_set_frame_shape(item,0,y,w,h);
	item_set_overlay(item,1);
        y += h;

        for ( i = 0; i < speech_num; i++) {
                x = 0;
                max_h = 0;

                if( speech[i].icon ) {
                        item = item_list_add(&item_list);
                        anim = imageDB_get_anim(ctx,speech[i].icon);
                        item_set_anim(item,0,y,anim);
			item_set_overlay(item,1);
                        if ( speech[i].keyword ) {
                                item_set_click_left(item,cb_speak,(void*)speech[i].keyword,NULL);
                        }
                        x = anim->w;
                        max_h = anim->h;
                }
                if( speech[i].text ) {
                        item = item_list_add(&item_list);
                        item_set_string(item,speech[i].text);
                        item_set_font(item,font);
                        sdl_get_string_size(item->font,item->string,&w,&h);
                        item_set_frame_shape(item,x,y,w,h);
			item_set_overlay(item,1);
                        if( speech[i].keyword ) {
                                item_set_click_left(item,cb_speak,(void*)speech[i].keyword,NULL);
                        }
                        if( max_h < h ) {
                                max_h = h;
                        }
                }

                y += max_h;
        }
}

/**********************************
Parse network frame and generate a speak screen
TODO: put this in network files ?
**********************************/
void ui_play_speak_parse(int total_size, char * frame)
{
        char * s_icon = NULL;
        char * s_text = NULL;
        char * s_keyword = NULL;
        int size = 0;
        context_t * speaker_ctx;

        speak_cleanup();

        speaker_id = strdup(_strsep(&frame,NETWORK_DELIMITER));
        speaker_ctx = context_find(speaker_id);
        if( speaker_ctx->character_name ) {
                speaker_name = strdup(speaker_ctx->character_name);
        }

        size += strlen(speaker_id);
        size += strlen(NETWORK_DELIMITER);
        speaker_text = strdup(_strsep(&frame,NETWORK_DELIMITER));
        size += strlen(speaker_text);
        size += strlen(NETWORK_DELIMITER);
        while( size < total_size ) {
                s_icon = _strsep(&frame,NETWORK_DELIMITER);
                if(s_icon == NULL) {
                        werr(LOGDEBUG,"Parsing error",NULL);
                        return;
                }
                size += strlen(s_icon);
                size += strlen(NETWORK_DELIMITER);

                s_text = _strsep(&frame,NETWORK_DELIMITER);
                if(s_text == NULL) {
                        werr(LOGDEBUG,"Parsing error",NULL);
                        return;
                }
                size += strlen(s_text);
                size += strlen(NETWORK_DELIMITER);

                s_keyword = _strsep(&frame,NETWORK_DELIMITER);
                if(s_keyword == NULL) {
                        werr(LOGDEBUG,"Parsing error",NULL);
                        return;
                }
                size += strlen(s_keyword);
                size += strlen(NETWORK_DELIMITER);

                speech_num++;
                speech = realloc(speech,speech_num*sizeof(speak_entry_t));
                if( *s_icon != 0 ) {
                        speech[speech_num-1].icon = strdup(s_icon);
                }
                else {
                        speech[speech_num-1].icon = NULL;
                }
                if(*s_text != 0) {
                        speech[speech_num-1].text = strdup(s_text);
                }
                else {
                        speech[speech_num-1].text = NULL;
                }
                if(*s_keyword != 0) {
                        speech[speech_num-1].keyword = strdup(s_keyword);
                }
                else {
                        speech[speech_num-1].keyword = NULL;
                }
        }

        ui_play_set(UI_SPEAK);
}


/****************************
****************************/
static void cb_speak_quit(void * arg)
{
	speak_cleanup();
        ui_play_set(UI_MAIN);
}

/**********************************
**********************************/
static void speak_compose(context_t * ctx, item_t * item_list) {
	compose_speak(ctx,item_list);

	sdl_add_keycb(SDL_SCANCODE_ESCAPE,cb_speak_quit,NULL,NULL);
        sdl_add_keycb(SDL_SCANCODE_SPACE,cb_speak_quit,NULL,NULL);
}
/**********************************
**********************************/
void ui_play_compose(context_t * ctx, item_t * item_list) {
	switch ( current_ui ) {
		case UI_MAIN:
			main_compose(ctx,item_list);
			break;
		case UI_INVENTORY:
			inventory_compose(ctx,item_list);
			break;
		case UI_SPEAK:
			speak_compose(ctx,item_list);
			break;
		default:
			werr(LOGUSER,"Wrong UI type");
			break;
	}
}
/**********************************
**********************************/
void ui_play_init()
{
	/* Empty text buffer */
	text_buffer[0]=0;
}
