#include <glib.h>
#include <glib/gprintf.h>
#include <gio/gio.h>
#include "../common/common.h"
#include <dirent.h>
#include <string.h>
#include "action.h"

/*****************************/
/* Delete the selected item from the avatar's equipment slot */
/* return -1 if fails */
gint equipment_delete(const gchar *id, const gchar * slot)
{
        context_t * context = context_find(id);
        if( context == NULL ) {
                g_message("%s: Could not find context %s",__func__,id);
                return -1;
        }

	if( remove_group(CHARACTER_TABLE, context->id, EQUIPMENT_EQUIPPED, EQUIPMENT_GROUP, slot, NULL)) {
		/* update client */
		network_send_avatar_file(context);
		return 0;
	}

	return -1;
}

/*****************************/
/* Add the passed item to the avatar's equipment slot */
/* return -1 if fails */
gint equipment_add(const gchar *id, const gchar * slot, const gchar * item)
{
        context_t * context = context_find(id);
        if( context == NULL ) {
                g_message("%s: Could not find context %s",__func__,id);
                return -1;
        }

	if( write_string(CHARACTER_TABLE, context->id, item, EQUIPMENT_GROUP, slot, EQUIPMENT_EQUIPPED, NULL)) {
		/* update client */
		network_send_avatar_file(context);
		return 0;
	}

	return -1;
}

/*****************************/
/* Return the name of the item in avatar's specified equipment slot*/
/* return NULL if fails */
const char * equipment_get_item_id(const gchar *id, const gchar * slot)
{
	const gchar * item;
        context_t * context = context_find(id);
        if( context == NULL ) {
                g_message("%s: Could not find context %s",__func__,id);
                return NULL;
        }

	if(!read_string(CHARACTER_TABLE, context->id, &item, EQUIPMENT_GROUP, slot, EQUIPMENT_EQUIPPED, NULL)) {
		return NULL;
	}

	return item;
}
