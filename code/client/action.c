#include <glib.h>
#include <gtk/gtk.h>
#include "../common/common.h"
#include "action.h"
#include "imageDB.h"
#include "inventory.h"
#include <glib/gprintf.h>
#include <textview.h>

/************************
on_action_bar_button_clicked
************************/
void on_action_bar_button_clicked(GtkToolButton *toolbutton,gpointer user_data)
{
	const gchar * action = gtk_tool_button_get_label(toolbutton);
	const gchar * script;

	if(!read_string(ACTION_TABLE,action,&script,ACTION_KEY_SCRIPT,NULL)) {
		return;
	}

	network_send_action(context_get_list_first(),(gchar *)script,NULL);
}

/************************
clear_bar
************************/
static void clear_bar(GtkWidget *widget,gpointer data)
{
        gtk_widget_destroy(widget);
}

/************************
update_action_bar
************************/
void update_action_bar(context_t * context, GtkWidget * bar)
{
	gchar ** action_list = NULL;
	static gchar ** old_action_list = NULL;
	gboolean identical = TRUE;
	gboolean save_list = TRUE;
	gint index = 0;
	const gchar * text;
	const gchar * icon;
	GtkWidget * image_widget;
	GtkToolItem * item;

        /* Read action list for current user */
	if(!read_list(CHARACTER_TABLE,context->id,&action_list,AVATAR_KEY_ACTION,NULL)) {
		return;
	}

	/* Avoid action bar flickering by comparing with old list */
	if(old_action_list && !updated_media) {
		index = 0;
		while(action_list[index]!=NULL && old_action_list[index]!= NULL) {
			if(g_strcmp0(action_list[index],old_action_list[index]) != 0) {
				identical = FALSE;
				break;
			}
			index++;
		}

		if(action_list[index]==NULL &&
			old_action_list[index]== NULL &&
			identical ){
			g_free(action_list);
			return;
		}
	}

	/* update the bar */
	/* Clear action_bar */
        gtk_container_foreach(GTK_CONTAINER(bar),clear_bar,NULL);

	index = 0;
	while(action_list[index] != NULL ) {
		if(!read_string(ACTION_TABLE,action_list[index],&text,ACTION_KEY_TEXT,NULL)) {
			save_list = FALSE;
			index++;
			continue;
		}

		if(!read_string(ACTION_TABLE,action_list[index],&icon,ACTION_KEY_ICON,NULL)) {
			save_list = FALSE;
			index++;
			continue;
		}

		/* load image */
		image_widget = imageDB_get_widget(context, icon);

		item = gtk_tool_button_new(image_widget,action_list[index]);
		g_signal_connect(item,"clicked",G_CALLBACK(on_action_bar_button_clicked),NULL);
		gtk_tool_item_set_tooltip_text(item,text);
		gtk_toolbar_insert(GTK_TOOLBAR(bar),item,0);

		index++;
	}

	if( save_list ) {
		if( old_action_list) {
			g_free(old_action_list);
		}

		old_action_list = action_list;
	}

	return;
}

/* Just a stub, for the moment clients do not use LUA */
void register_lua_functions( lua_State* L)
{
}

