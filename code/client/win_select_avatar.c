#include <gtk/gtk.h>
#include "config.h"
#include "../common/common.h"
#include "win_game.h"
#include <glib.h>
#include <glib/gstdio.h>
#include "imageDB.h"
#include "file.h"

static GtkWidget *              select_avatar_window;
static GtkWidget *              avatar_list;

/*************************
 Update marquee picture for each list entry
*************************/
void update_select_avatar_window(context_t * context)
{
	GtkTreeModel * model;
	GtkTreeIter     iter;
	gchar * type_name;
	const gchar * marquee_name;
	GtkListStore * avatar_list_store;

	/* Do nothing if not visible */
	gdk_threads_enter();
	if( gtk_widget_get_visible(select_avatar_window) == FALSE ) {
		gdk_threads_leave();
		return;
	}

	model=gtk_tree_view_get_model(GTK_TREE_VIEW(avatar_list));
	avatar_list_store=GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(avatar_list)));
	if( gtk_tree_model_get_iter_first (model,&iter) == FALSE ) {
		gdk_threads_leave();
		return;
	}
	do {
		gtk_tree_model_get (model,&iter,1,&type_name, -1);

		/* Compute the marquee file name */
		if(!read_string(AVATAR_TABLE,type_name,&marquee_name,AVATAR_KEY_MARQUEE,NULL)) {
			g_free(type_name);
			continue;
		}

		g_free(type_name);

		/* Get the marquee */
		GtkWidget * image = imageDB_get_widget(context,marquee_name);

		/* FIXME : It seems that animation are not supported in list... */
		gtk_list_store_set(avatar_list_store,&iter,3,gdk_pixbuf_animation_get_static_image(gtk_image_get_animation(GTK_IMAGE(image))),-1);
	}
	while( gtk_tree_model_iter_next(model,&iter) != FALSE );
	gdk_threads_leave();

}

/*************************
 show window
*************************/
void show_select_avatar_window(context_t * context)
{
	gdk_threads_enter();
        gtk_widget_show_all(select_avatar_window);
	gdk_threads_leave();
	update_select_avatar_window(context);
}

/*************************
 hide window
Called from on_* function so no need to lock GDK
*************************/
void hide_select_avatar_window(void)
{
        gtk_widget_hide_all(select_avatar_window);
}

/*************************
 add an avatar to the list
the data is a list a 3 strings, the first string is the id of the character (its file name) the second one is the type of the avatar, the third is the name of the character.
the list ends with an empty string
*************************/
void add_user_avatar(context_t * context, gchar * data)
{
	GtkListStore * avatar_list_store;
	GtkTreeIter iter;
	gchar * current_string = data;
	gchar * id;
	gchar * name;
	gchar * type;

	/* Start critical GDK/GTK section */
	gdk_threads_enter ();

	gdk_window_freeze_updates(avatar_list->window); /* to avoid flickering */

	avatar_list_store=GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(avatar_list)));
	gtk_list_store_clear(avatar_list_store);

	while(current_string[0] != 0) {
		id = current_string;
		current_string += g_utf8_strlen(id,-1)+1;
		type = current_string;
		current_string += g_utf8_strlen(type,-1)+1;
		name = current_string;
		current_string += g_utf8_strlen(name,-1)+1;

		gtk_list_store_append (avatar_list_store, &iter);
		/* Try to display marquee */
		gtk_list_store_set (avatar_list_store, &iter,
				0, name,
				1, type,
				2, id,
				-1);
	}

	gdk_window_thaw_updates(avatar_list->window); /* to avoid flickering */

	gdk_threads_leave ();

}

/*************************
 create button clicked
*************************/
void on_create_clicked (GtkButton *button, gpointer   user_data)
{
}

/*************************
 play button clicked
*************************/
void on_play_clicked (GtkButton *button, gpointer   user_data)
{
	GtkTreeSelection *sel;
	GtkTreeModel*   model;
        GtkTreeIter     iter;
	gchar * id;

	context_t * context = (context_t *)user_data;

	/* get selected avatar */
	sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(avatar_list));
	/* if no selection */
	if( ! gtk_tree_selection_count_selected_rows( sel ) ) {
		return;
	}
	gtk_tree_selection_get_selected(sel,&model,&iter);
	gtk_tree_model_get (model,&iter,2,&id, -1);
	context_set_id(context, id);
	g_free(id);
	
	gtk_tree_selection_get_selected(sel,&model,&iter);
	gtk_tree_model_get (model,&iter,0,&id, -1);
	context_set_avatar_name(context, id);
	g_free(id);
	
	file_clean(context);

	hide_select_avatar_window();
	show_game_window(context);
}

/****************
return FALSE on error
****************/

gboolean win_select_avatar_init(context_t * context)
{
        GtkBuilder * builder = NULL;
        builder = gtk_builder_new ();

	gdk_threads_enter ();
        gchar tmp[SMALL_BUF];
//	g_snprintf(tmp,SMALL_BUF,"./select_avatar.glade",DATADIR);
	g_snprintf(tmp,SMALL_BUF,"./select_avatar.glade");
	if( ! gtk_builder_add_from_file (builder, tmp,NULL)) {
		g_snprintf(tmp,SMALL_BUF,"%s/wog/client/select_avatar.glade",DATADIR);
		if( ! gtk_builder_add_from_file (builder, tmp,NULL)) {
			g_print("Error while loading UI file select_avatar.glade\n");
			return FALSE;
		}
	}

        select_avatar_window = GTK_WIDGET(gtk_builder_get_object(builder,"select_avatar"));
        avatar_list   = GTK_WIDGET(gtk_builder_get_object(builder,"avatar_list"));

        gtk_builder_connect_signals (builder, context);
        g_object_unref (G_OBJECT (builder));

	gtk_widget_hide_all(select_avatar_window);
	gtk_widget_realize(select_avatar_window);
	gdk_threads_leave ();

	return TRUE;
}

