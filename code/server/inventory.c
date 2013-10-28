#include <glib.h>
#include <glib/gprintf.h>
#include <gio/gio.h>
#include "../common/common.h"
#include <dirent.h>
#include <string.h>
#include "action.h"

/*****************************/
/* Delete the requested item from the avatar's inventory */
/* return -1 if fails */
gint inventory_delete(const gchar * id, const gchar * item)
{
	context_t * context = context_find(id);
	if( context == NULL ) {
		g_message("%s: Could not find context %s",__func__,id);
		return -1;
	}

	if( remove_from_list(CHARACTER_TABLE, context->id, item, AVATAR_KEY_INVENTORY, NULL)) {
		/* update client */
		network_send_avatar_file(context);
		return 0;
	}

	network_send_avatar_file(context);
	return 0;
}

/*****************************/
/* Add the requested item to the avatar's inventory */
/* return -1 if fails */
gint inventory_add(const gchar * id, const gchar * item)
{
        context_t * context = context_find(id);
        if( context == NULL ) {
                g_message("%s: Could not find context %s",__func__,id);
                return -1;
        }

	if(!add_to_list(CHARACTER_TABLE,context->id,item, AVATAR_KEY_INVENTORY, NULL)) {
		return -1;
	}

	network_send_avatar_file(context);
	return 0;
}

/*****************************/
/* Count the number of item whose name is passed in the avatar's inventory */
/* return -1 if fails, otherwise the number of item */
gint inventory_count(const gchar * id, const gchar * item_name)
{
	gint count = 0;
	gint index;
	gchar ** name_list;
	const gchar * name;

        context_t * context = context_find(id);
        if( context == NULL ) {
                g_message("%s: Could not find context %s",__func__,id);
                return -1;
        }

	if(!read_list(CHARACTER_TABLE,context->id,&name_list,AVATAR_KEY_INVENTORY,NULL) ) {
		return -1;
	}

	index=0;
	while( name_list[index] != NULL) {
		if(read_string(ITEM_TABLE,name_list[index],&name,ITEM_NAME,NULL)) {
			if( g_strcmp0(item_name,name) == 0 ) {
				count++;
			}
		}
		index++;
	}

	g_free(name_list);
	return count;
}

/*****************************/
/* return an item ID of an item in inventory with specified name */
/* the returned string must be freed */
gchar * inventory_get_by_name(const gchar * id, const gchar * item_name)
{
        gint index;
        gchar ** name_list;
        const gchar * name;
	gchar * res;

        context_t * context = context_find(id);
        if( context == NULL ) {
                g_message("%s: Could not find context %s",__func__,id);
                return NULL;
        }

	if(!read_list(CHARACTER_TABLE,context->id,&name_list,AVATAR_KEY_INVENTORY,NULL) ) {
                return NULL;
        }

        index=0;
        while( name_list[index] != NULL) {
		if(read_string(ITEM_TABLE,name_list[index],&name,ITEM_NAME,NULL)) {
                        if( g_strcmp0(item_name,name) == 0 ) {
				res = g_strdup(name_list[index]);
				g_free(name_list);
				return res;
                        }
                }
		index++;
        }

	g_free(name_list);
        return NULL;
}
