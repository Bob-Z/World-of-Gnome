#include <gtk/gtk.h>
#include "../common/common.h"
#include "config.h"
#include "imageDB.h"
#include <glib/gprintf.h>

extern GtkWidget *             equipment_list;
GtkTreePath * selected_equipment_item_path = NULL;

static void get_selection(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	gtk_tree_path_free(selected_equipment_item_path);

	if( path ) {
		selected_equipment_item_path = gtk_tree_path_copy(path);
	}
	else {
		selected_equipment_item_path = NULL;
	}
}

/****************************
get_selected_equipment_item
need to free the returned string
****************************/
gchar * get_selected_equipment_item()
{
	GtkTreeModel * model;
	GtkTreeIter iter;
	gchar * slot_name;
	GtkTreeSelection *  selection;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(equipment_list));

	/* Get the selection path */
        gtk_tree_selection_selected_foreach(selection,get_selection,NULL);
	if( selected_equipment_item_path == NULL ) {
		return NULL;
	}

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(equipment_list));
	if(!gtk_tree_model_get_iter(model,&iter,selected_equipment_item_path)){
		return NULL;
	}
	gtk_tree_model_get (model,&iter,4,&slot_name,-1);

	return g_strdup(slot_name);
}
/************************
fill_equipment
************************/
void fill_equipment(context_t * context)
{
        gchar ** name_list;
        GtkWidget * icon;
        GdkPixbuf * icon_pixbuf;
	GtkListStore * equipment_list_store;
	GtkTreeIter iter;
	gchar * new_slot;
	gint index = 0;
        const gchar * icon_name;
        const gchar * equipped_name;
        const gchar * equipped_text;
        const gchar * equipped_icon_name;
        const gchar * item_name;
        gchar * name;
	GtkTreeSelection *  selection;


	/* Get the selection path */
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(equipment_list));
        gtk_tree_selection_selected_foreach(selection,get_selection,NULL);
	/* clear the view */
	equipment_list_store=GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(equipment_list)));
	gtk_list_store_clear(equipment_list_store);

        if(!get_group_list(CHARACTER_TABLE,context->id,&name_list,EQUIPMENT_GROUP,NULL) ) {
                return;
        }

        while( name_list[index] != NULL) {
                gtk_list_store_append (equipment_list_store, &iter);

                /* Get the slot name */
                if(!read_string(CHARACTER_TABLE,context->id,&item_name,EQUIPMENT_GROUP,name_list[index],EQUIPMENT_NAME,NULL)) {
                        name = g_strdup(name_list[index]);
                }
                else {
                        name = g_strdup(item_name);
                }

                /* Get the slot icon */
                if(!read_string(CHARACTER_TABLE,context->id,&icon_name,EQUIPMENT_GROUP,name_list[index],EQUIPMENT_ICON,NULL)) {
                        gtk_list_store_set (equipment_list_store, &iter,
                                0, name,
				4, name_list[index],
                                -1);
                }
                else {
                        icon = imageDB_get_widget(context,icon_name);
                        icon_pixbuf = gdk_pixbuf_animation_get_static_image(gtk_image_get_animation(GTK_IMAGE(icon)));
                        gtk_widget_destroy(icon);

                        gtk_list_store_set (equipment_list_store, &iter,
                                0, name,
                                1, icon_pixbuf,
				4, name_list[index],
                                -1);
                }

                /* Is there an equipped object ? */
                if(read_string(CHARACTER_TABLE,context->id,&equipped_name,EQUIPMENT_GROUP,name_list[index],EQUIPMENT_EQUIPPED,NULL)) {
                        /* Get the equipped object name */
                        if(!read_string(ITEM_TABLE,equipped_name,&equipped_text,ITEM_NAME,NULL)) {
                                g_warning("Can't read object %s name in equippment slot %s",equipped_name,name);
                        }
                        else {
                                /* Get it's icon */
                                if(!read_string(ITEM_TABLE,equipped_name,&equipped_icon_name,ITEM_ICON,NULL)) {
                                        g_warning("Can't read object %s icon in equippment slot %s",equipped_name,name);
                                }
                                else {
                                        icon = imageDB_get_widget(context,equipped_icon_name);
                                        icon_pixbuf = gdk_pixbuf_animation_get_static_image(gtk_image_get_animation(GTK_IMAGE(icon)));
                                        gtk_widget_destroy(icon);

                                        gtk_list_store_set (equipment_list_store, &iter,
                                                2, equipped_text,
                                                3, icon_pixbuf,
                                         -1);
                                }

                        }
                }

                g_free(name);
                index++;
        }

	g_free(name_list);


	/* Re-select the old selected item if it still exists */
	if( selected_equipment_item_path && gtk_tree_path_get_indices(selected_equipment_item_path)==NULL) {
		selected_equipment_item_path=NULL;
	}

	if( selected_equipment_item_path ) {
		gtk_tree_selection_select_path(selection,selected_equipment_item_path);
	}

	new_slot = get_selected_equipment_item();
	if( new_slot != NULL ) {
		/* no slot was selected previoulsly */
		if( context->selection.equipment == NULL) {
			context->selection.equipment = new_slot;
			network_send_context(context);
		}
		else {
			/* a different slot was selected previously */
			if(g_strcmp0(new_slot,context->selection.equipment)!=0) {
				g_free( context->selection.equipment );
				context->selection.equipment = new_slot;
				network_send_context(context);
			}
			/* the same slot was previously selected */
			else {
				g_free(new_slot);
			}
		}
	}
	else {
		/* No more slot selected */
		if( context->selection.equipment != NULL ) {
			g_free( context->selection.equipment );
			context->selection.equipment = NULL;
			network_send_context(context);
		}
	}
}
