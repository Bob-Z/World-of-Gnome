#include <gtk/gtk.h>
#include "../common/common.h"
#include "config.h"
#include "imageDB.h"
#include <glib/gprintf.h>

extern GtkWidget *             inventory_list;
GtkTreePath * selected_inventory_item_path = NULL;

static void get_selection(GtkIconView *icon_view,GtkTreePath *path,gpointer data)
{
	gtk_tree_path_free(selected_inventory_item_path);

	if( path ) {
		selected_inventory_item_path = gtk_tree_path_copy(path);
	}
	else {
		selected_inventory_item_path = NULL;
	}
}

/****************************
get_selected_inventory_item
need to free the returned string
****************************/
gchar * get_selected_inventory_item()
{
	GtkTreeModel * model;
	GtkTreeIter iter;
	gchar * item_name;

	/* Get the selection path */
        gtk_icon_view_selected_foreach(GTK_ICON_VIEW(inventory_list),get_selection,NULL);
	if( selected_inventory_item_path == NULL ) {
		return NULL;
	}

	model = gtk_icon_view_get_model(GTK_ICON_VIEW(inventory_list));
	if(!gtk_tree_model_get_iter(model,&iter,selected_inventory_item_path)){
		return NULL;
	}
	gtk_tree_model_get (model,&iter,3,&item_name,-1);

	return g_strdup(item_name);
}
/************************
fill_inventory
************************/
void fill_inventory(context_t * context)
{
        gchar ** item_list;
        const gchar * value;
        gint i=0;
        GtkWidget * icon;
        GdkPixbuf * icon_pixbuf;
        gchar * label;
        gchar * description = NULL;
	GtkListStore * inventory_list_store;
	GtkTreeIter iter;
	gchar * new_item;

	/* Get the selection path */
        gtk_icon_view_selected_foreach(GTK_ICON_VIEW(inventory_list),get_selection,NULL);
	/* clear the view */
	inventory_list_store=GTK_LIST_STORE(gtk_icon_view_get_model(GTK_ICON_VIEW(inventory_list)));
	gtk_list_store_clear(inventory_list_store);

	/* set-up the column */
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(inventory_list),0);
	gtk_icon_view_set_tooltip_column(GTK_ICON_VIEW(inventory_list),1);
	gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(inventory_list),2);

	/* read data from file */
	if(!read_list(CHARACTER_TABLE,context->id,&item_list, AVATAR_KEY_INVENTORY,NULL)) {
		return;
	}

        while( item_list[i] != NULL) {
		if(!read_string(ITEM_TABLE,item_list[i],&value,ITEM_NAME,NULL)) {
			label = g_strdup(item_list[i]);
                }
		else {
                	label = g_strdup(value);
		}

		if(!read_string(ITEM_TABLE,item_list[i],&value,ITEM_DESC,NULL)) {
			description = g_strdup("");;
                }
		else {
                        description = g_strdup(value);
		}

		if(!read_string(ITEM_TABLE,item_list[i],&value,ITEM_ICON,NULL)) {
			gtk_list_store_append (inventory_list_store, &iter);
			gtk_list_store_set (inventory_list_store, &iter,
					0, label,
					1, description,
					3, item_list[i],
					-1);

		}
		else {
			icon = imageDB_get_widget(context,value);
			icon_pixbuf = gdk_pixbuf_animation_get_static_image(gtk_image_get_animation(GTK_IMAGE(icon)));
			gtk_widget_destroy(icon);

			gtk_list_store_append (inventory_list_store, &iter);
			gtk_list_store_set (inventory_list_store, &iter,
					0, label,
					1, description,
					2, icon_pixbuf,
					3, item_list[i],
					-1);

		}

	        g_free(description);
                g_free(label);

                i++;
        }

	g_free(item_list);

	/* Re-select the old selected item if it still exists */
	if( selected_inventory_item_path && gtk_tree_path_get_indices(selected_inventory_item_path)==NULL) {
		selected_inventory_item_path=NULL;
	}

	if( selected_inventory_item_path ) {
		gtk_icon_view_select_path(GTK_ICON_VIEW(inventory_list),selected_inventory_item_path);
	}

	new_item = get_selected_inventory_item();
	if( new_item != NULL ) {
		/* no item was selected previoulsly */
		if( context->selection.inventory == NULL) {
			context->selection.inventory = new_item;
			network_send_context(context);
		}
		else {
			/* a different item was selected previously */
			if(g_strcmp0(new_item,context->selection.inventory)!=0) {
				g_free( context->selection.inventory );
				context->selection.inventory = new_item;
				network_send_context(context);
			}
			/* the same item was previously selected */
			else {
				g_free(new_item);
			}
		}
	}
	else {
		/* No more item selected */
		if( context->selection.inventory != NULL ) {
			g_free( context->selection.inventory );
			context->selection.inventory = NULL;
			network_send_context(context);
		}
	}
}
